/*
 * Modified by Neural Magic
 * Copyright (C) Marlin.2024 Elias Frantar
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Adapted from https://github.com/IST-DASLab/marlin
 */

#include <c10/cuda/CUDAGuard.h>
#include <cuda_bf16.h>
#include <cuda_fp16.h>
#include <glog/logging.h>
#include <torch/torch.h>

#include "gemm_kernel_launch.cuh"
#include "marlin.h"

namespace marlin {
namespace {

using thread_config_t = struct {
  int thread_k;
  int thread_n;
  int num_threads;
};

using exec_config_t = struct {
  int max_m_blocks;
  thread_config_t tb_cfg;
};

thread_config_t small_batch_thread_configs[] = {
    // Ordered by priority

    // thread_k, thread_n, num_threads
    {128, 128, 256},
    {64, 128, 128},
    {128, 64, 128},
};

thread_config_t large_batch_thread_configs[] = {
    // Ordered by priority

    // thread_k, thread_n, num_threads
    {64, 256, 256},
    {64, 128, 128},
    {128, 64, 128},

};

int get_scales_cache_size(thread_config_t const& th_config,
                          int prob_m,
                          int prob_n,
                          int prob_k,
                          int num_bits,
                          int group_size,
                          bool has_act_order,
                          bool is_k_full) {
  bool cache_scales_chunk = has_act_order && !is_k_full;

  int tb_n = th_config.thread_n;
  int tb_k = th_config.thread_k;

  // Get max scale groups per thread-block
  int tb_groups;
  if (group_size == -1) {
    tb_groups = 1;
  } else if (group_size == 0) {
    tb_groups = div_ceil(tb_k, 32);  // Worst case is 32 group size
  } else {
    tb_groups = div_ceil(tb_k, group_size);
  }

  if (cache_scales_chunk) {
    int load_groups =
        tb_groups * pipe_stages * 2;     // Chunk size is 2x pipeline over dim K
    load_groups = max(load_groups, 32);  // We load at least 32 scale groups
    return load_groups * tb_n * 2;

  } else {
    int tb_scales = tb_groups * tb_n * 2;

    return tb_scales * pipe_stages;
  }
}

bool is_valid_cache_size(thread_config_t const& th_config,
                         int max_m_blocks,
                         int prob_m,
                         int prob_n,
                         int prob_k,
                         int num_bits,
                         int scales_cache_size,
                         int max_shared_mem) {
  int pack_factor = 32 / num_bits;

  // Get B size
  int tb_k = th_config.thread_k;
  int tb_n = th_config.thread_n;

  int b_size = (tb_k * tb_n / pack_factor) * 4;

  // Get A size
  int m_blocks = div_ceil(prob_m, 16);
  int tb_max_m = 16;

  while (true) {
    if (m_blocks >= max_m_blocks) {
      tb_max_m *= max_m_blocks;
      break;
    }

    max_m_blocks--;
    if (max_m_blocks == 0) {
      TORCH_CHECK(false, "Unexpected m_blocks = ", m_blocks);
    }
  }

  int a_size = (tb_max_m * tb_k) * 2;

  float pipe_size = (a_size + b_size) * pipe_stages;

  TORCH_CHECK(max_shared_mem / 2 > scales_cache_size);  // Sanity

  return pipe_size < 0.95f * (max_shared_mem - scales_cache_size);
}

bool is_valid_config(thread_config_t const& th_config,
                     int max_m_blocks,
                     int prob_m,
                     int prob_n,
                     int prob_k,
                     int num_bits,
                     int group_size,
                     bool has_act_order,
                     bool is_k_full,
                     int max_shared_mem) {
  // Sanity
  if (th_config.thread_k == -1 || th_config.thread_n == -1 ||
      th_config.num_threads == -1) {
    return false;
  }

  // Verify K/N are divisible by thread K/N
  if (prob_k % th_config.thread_k != 0 || prob_n % th_config.thread_n != 0) {
    return false;
  }

  // Verify min for thread K/N
  if (th_config.thread_n < min_thread_n || th_config.thread_k < min_thread_k) {
    return false;
  }

  // num_threads must be at least 128 (= 4 warps)
  if (th_config.num_threads < 128) {
    return false;
  }

  //  Determine cache for scales
  int scales_cache_size = get_scales_cache_size(th_config,
                                                prob_m,
                                                prob_n,
                                                prob_k,
                                                num_bits,
                                                group_size,
                                                has_act_order,
                                                is_k_full);

  // Check that pipeline fits into cache
  if (!is_valid_cache_size(th_config,
                           max_m_blocks,
                           prob_m,
                           prob_n,
                           prob_k,
                           num_bits,
                           scales_cache_size,
                           max_shared_mem)) {
    return false;
  }

  return true;
}

int determine_reduce_max_m(int prob_m, int max_par) {
  constexpr int tile_m_size = 16;

  if (prob_m <= tile_m_size) {
    return tile_m_size;

  } else if (prob_m <= tile_m_size * 2) {
    return tile_m_size * 2;

  } else if (prob_m <= tile_m_size * 3) {
    return tile_m_size * 3;

  } else if (prob_m <= tile_m_size * 4) {
    return tile_m_size * 4;

  } else {
    int cur_par = min(div_ceil(prob_m, tile_m_size * 4), max_par);
    return tile_m_size * 4 * cur_par;
  }
}

exec_config_t determine_thread_config(int prob_m,
                                      int prob_n,
                                      int prob_k,
                                      int num_bits,
                                      int group_size,
                                      bool has_act_order,
                                      bool is_k_full,
                                      int max_shared_mem) {
  int max_m_blocks = 4;
  while (max_m_blocks > 0) {
    if (prob_m <= 16) {
      for (auto th_config : small_batch_thread_configs) {
        if (is_valid_config(th_config,
                            max_m_blocks,
                            prob_m,
                            prob_n,
                            prob_k,
                            num_bits,
                            group_size,
                            has_act_order,
                            is_k_full,
                            max_shared_mem)) {
          return exec_config_t{max_m_blocks, th_config};
        }
      }
    } else {
      for (auto th_config : large_batch_thread_configs) {
        if (is_valid_config(th_config,
                            max_m_blocks,
                            prob_m,
                            prob_n,
                            prob_k,
                            num_bits,
                            group_size,
                            has_act_order,
                            is_k_full,
                            max_shared_mem)) {
          return exec_config_t{max_m_blocks, th_config};
        }
      }
    }

    max_m_blocks--;  // Process less M blocks per invocation to reduce cache
                     // usage
  }

  return exec_config_t{0, {-1, -1, -1}};
}

template <typename scalar_t>
void marlin_mm(const void* A,
               const void* B,
               void* C,
               void* C_tmp,
               void* s,
               void* zp,
               void* g_idx,
               void* perm,
               void* a_tmp,
               int prob_m,
               int prob_n,
               int prob_k,
               void* workspace,
               int num_bits,
               bool has_act_order,
               bool is_k_full,
               bool has_zp,
               int num_groups,
               int group_size,
               int dev,
               cudaStream_t stream,
               int thread_k,
               int thread_n,
               int sms,
               int max_par,
               bool use_fp32_reduce) {
  CHECK(num_bits == 4 || num_bits == 8)
      << "Unsupported num_bits = " << num_bits;
  CHECK(prob_m > 0 && prob_n > 0 && prob_k > 0)
      << "Invalid MNK = [" << prob_m << ", " << prob_n << ", " << prob_k << "]";

  // TODO: remove alias when we start supporting other 8bit types
  int tot_m = prob_m;
  int tot_m_blocks = div_ceil(tot_m, 16);
  int pad = 16 * tot_m_blocks - tot_m;

  if (sms == -1) {
    cudaDeviceGetAttribute(&sms, cudaDevAttrMultiProcessorCount, dev);
  }

  int max_shared_mem = 0;
  cudaDeviceGetAttribute(
      &max_shared_mem, cudaDevAttrMaxSharedMemoryPerBlockOptin, dev);
  CHECK(max_shared_mem > 0);

  // Set thread config
  exec_config_t exec_cfg;
  if (thread_k != -1 && thread_n != -1) {
    // User-defined config
    exec_cfg =
        exec_config_t{4, thread_config_t{thread_k, thread_n, default_threads}};
  } else {
    // Auto config
    exec_cfg = determine_thread_config(prob_m,
                                       prob_n,
                                       prob_k,
                                       num_bits,
                                       group_size,
                                       has_act_order,
                                       is_k_full,
                                       max_shared_mem);
  }

  const bool valid_config =
      exec_cfg.max_m_blocks > 0 && is_valid_config(exec_cfg.tb_cfg,
                                                   exec_cfg.max_m_blocks,
                                                   prob_m,
                                                   prob_n,
                                                   prob_k,
                                                   num_bits,
                                                   group_size,
                                                   has_act_order,
                                                   is_k_full,
                                                   max_shared_mem);
  CHECK(valid_config) << "Invalid thread config: max_m_blocks = "
                      << exec_cfg.max_m_blocks
                      << ", thread_k = " << exec_cfg.tb_cfg.thread_k
                      << ", thread_n = " << exec_cfg.tb_cfg.thread_n
                      << ", num_threads = " << exec_cfg.tb_cfg.num_threads
                      << " for MKN = [" << prob_m << ", " << prob_k << ", "
                      << prob_n << "] and num_bits = " << num_bits
                      << ", group_size = " << group_size
                      << ", has_act_order = " << has_act_order
                      << ", is_k_full = " << is_k_full
                      << ", max_shared_mem = " << max_shared_mem;

  int num_threads = exec_cfg.tb_cfg.num_threads;
  thread_k = exec_cfg.tb_cfg.thread_k;
  thread_n = exec_cfg.tb_cfg.thread_n;

  int thread_k_blocks = thread_k / 16;
  int thread_n_blocks = thread_n / 16;

  int blocks = sms;

  CHECK(prob_n % thread_n == 0)
      << "prob_n = " << prob_n
      << " is not divisible by thread_n = " << thread_n;
  CHECK(prob_k % thread_k == 0)
      << "prob_k = " << prob_k
      << " is not divisible by thread_k = " << thread_k;

  int group_blocks = 0;
  if (has_act_order) {
    if (is_k_full) {
      CHECK(group_size != -1);
      group_blocks = group_size / 16;
      CHECK(prob_k % group_blocks == 0)
          << "prob_k = " << prob_k
          << " is not divisible by group_blocks = " << group_blocks;
    } else {
      CHECK(group_size == 0);
      group_blocks = 0;
    }

  } else {
    if (group_size == -1) {
      group_blocks = -1;
    } else {
      group_blocks = group_size / 16;
      CHECK(prob_k % group_blocks == 0)
          << "prob_k = " << prob_k
          << " is not divisible by group_blocks = " << group_blocks;
    }
  }

  const int4* A_ptr = (const int4*)A;
  const int4* B_ptr = (const int4*)B;
  int4* C_ptr = (int4*)C;
  int4* C_tmp_ptr = (int4*)C_tmp;
  const int4* s_ptr = (const int4*)s;
  const int4* zp_ptr = (const int4*)zp;
  const int* g_idx_ptr = (const int*)g_idx;
  const int* perm_ptr = (const int*)perm;
  int4* a_tmp_ptr = (int4*)a_tmp;

  int* locks = (int*)workspace;

  if (has_act_order) {
    // Permute A columns
    int block_rows = div_ceil(prob_m, blocks);
    permute_cols_kernel<<<blocks, default_threads, 0, stream>>>(
        A_ptr, perm_ptr, a_tmp_ptr, prob_m, prob_k, block_rows);
    A_ptr = a_tmp_ptr;
  }

  // If we have a full K, then we can run the non-act-order version of Marlin
  // (since the weight rows are reordered by increasing group ids, and by having
  // a full K, we have full original groups)
  if (is_k_full) {
    has_act_order = false;
  }

  // Main loop
  for (int i = 0; i < tot_m_blocks; i += exec_cfg.max_m_blocks) {
    int thread_m_blocks = tot_m_blocks - i;
    prob_m = tot_m - 16 * i;
    int par = 1;
    if (thread_m_blocks > exec_cfg.max_m_blocks) {
      // Note that parallel > 1 currently only works for inputs without any
      // padding
      par = (16 * thread_m_blocks - pad) / (16 * exec_cfg.max_m_blocks);
      if (par > max_par) par = max_par;
      prob_m = (16 * exec_cfg.max_m_blocks) * par;
      i += exec_cfg.max_m_blocks * (par - 1);
      thread_m_blocks = exec_cfg.max_m_blocks;
    }

    if (false) {
    }
    GPTQ_CALL_IF(4, 16, 4, 256)
    GPTQ_CALL_IF(4, 8, 8, 256)
    GPTQ_CALL_IF(4, 8, 4, 128)
    GPTQ_CALL_IF(4, 4, 8, 128)
    GPTQ_CALL_IF(8, 16, 4, 256)
    GPTQ_CALL_IF(8, 8, 8, 256)
    GPTQ_CALL_IF(8, 8, 4, 128)
    GPTQ_CALL_IF(8, 4, 8, 128)

    AWQ_CALL_IF(4, 16, 4, 256)
    AWQ_CALL_IF(4, 8, 8, 256)
    AWQ_CALL_IF(4, 8, 4, 128)
    AWQ_CALL_IF(4, 4, 8, 128)
    AWQ_CALL_IF(8, 16, 4, 256)
    AWQ_CALL_IF(8, 8, 8, 256)
    AWQ_CALL_IF(8, 8, 4, 128)
    AWQ_CALL_IF(8, 4, 8, 128)
    else {
      LOG(FATAL) << "Unsupported shapes: MNK = [" << prob_m << ", " << prob_n
                 << ", " << prob_k << "], has_act_order = " << has_act_order
                 << ", num_groups = " << num_groups
                 << ", group_size = " << group_size
                 << ", thread_m_blocks = " << thread_m_blocks
                 << ", thread_n_blocks = " << thread_n_blocks
                 << ", thread_k_blocks = " << thread_k_blocks
                 << ", num_bits = " << num_bits;
    }

    A_ptr += 16 * thread_m_blocks * (prob_k / 8) * par;
    C_ptr += 16 * thread_m_blocks * (prob_n / 8) * par;
  }
}

}  // namespace

void gptq_gemm(
    const torch::Tensor& A,       // (m, k)
    const torch::Tensor& B,       // (k, n) => (k/16, n * 16 / pack_factor)
    torch::Tensor& C,             // (m, n)
    const torch::Tensor& scales,  // (n_groups, n)
    const torch::Tensor& zeros,   // (n_groups, n / pack_factor)
    const torch::Tensor& g_idx,
    const torch::Tensor& perm,
    torch::Tensor& workspace,
    int num_bits,
    bool is_k_full,
    bool has_zp,
    bool use_fp32_reduce) {
  CHECK(num_bits == 4 || num_bits == 8)
      << "num_bits must be 4 or 8. Got = " << num_bits;

  int pack_factor = 32 / num_bits;

  int prob_m = A.size(0);
  int prob_n = C.size(1);
  int prob_k = A.size(1);

  // Alloc buffers
  const at::cuda::OptionalCUDAGuard device_guard(device_of(A));
  auto options = torch::TensorOptions().dtype(A.dtype()).device(A.device());

  // why do we need to allocate a_tmp?
  torch::Tensor a_tmp = torch::empty({prob_m, prob_k}, options);

  // Alloc C tmp buffer that is going to be used for the global reduce
  int reduce_max_m = marlin::determine_reduce_max_m(prob_m, marlin::max_par);
  int reduce_n = prob_n;
  auto options_fp32 =
      torch::TensorOptions().dtype(at::kFloat).device(A.device());
  if (!use_fp32_reduce) {
    reduce_max_m = 0;
    reduce_n = 0;
  }
  torch::Tensor c_tmp = torch::empty({reduce_max_m, reduce_n}, options_fp32);

  // thread_k: `k` size of a thread_tile in `weights` (can usually be left as
  // auto -1)
  int thread_k = -1;
  // thread_n: `n` size of a thread_tile in `weights` (can usually be left as
  // auto -1)
  int thread_n = -1;
  // sms: number of SMs to use for the kernel (can usually be left as auto -1)
  int sms = -1;

  // Verify g_idx and perm
  CHECK((g_idx.size(0) == 0 && perm.size(0) == 0) ||
        (g_idx.size(0) == prob_k && perm.size(0) == prob_k))
      << "Unexpected g_idx.size(0) = " << g_idx.size(0)
      << " and perm.size(0) = " << perm.size(0)
      << ", where size_k = " << prob_k;

  // Detect groupsize and act_order
  int num_groups = -1;
  int group_size = -1;
  bool has_act_order = g_idx.size(0) != 0;

  int rank = scales.sizes().size();
  CHECK(rank == 2) << "b_scales rank = " << rank << " is not 2";
  CHECK(scales.size(1) == prob_n)
      << "b_scales dim 1 = " << scales.size(1) << " is not size_n = " << prob_n;
  num_groups = scales.size(0);

  if (has_act_order) {
    if (is_k_full) {
      CHECK(num_groups > 1) << "For act_order, num_groups must be > 1";
      CHECK(prob_k % num_groups == 0)
          << "size_k = " << prob_k
          << ", is not divisible by num_groups = " << num_groups;
      group_size = prob_k / num_groups;
    } else {
      group_size = 0;
    }

  } else {
    if (num_groups > 1) {
      CHECK(prob_k % num_groups == 0)
          << "size_k = " << prob_k
          << ", is not divisible by b_scales.size(0) = " << scales.size(0);
      group_size = prob_k / num_groups;
    } else {
      group_size = -1;
    }
  }

  // Verify workspace size
  CHECK(prob_n % marlin::min_thread_n == 0)
      << "size_n = " << prob_n
      << ", is not divisible by min_thread_n = " << marlin::min_thread_n;
  int min_workspace_size = (prob_n / marlin::min_thread_n) * marlin::max_par;
  CHECK(workspace.numel() >= min_workspace_size)
      << "workspace.numel = " << workspace.numel()
      << " is below min_workspace_size = " << min_workspace_size;

  int dev = A.get_device();
  if (A.scalar_type() == at::ScalarType::Half) {
    marlin_mm<half>(A.data_ptr(),
                    B.data_ptr(),
                    C.data_ptr(),
                    c_tmp.data_ptr(),
                    scales.data_ptr(),
                    zeros.data_ptr(),
                    g_idx.data_ptr(),
                    perm.data_ptr(),
                    a_tmp.data_ptr(),
                    prob_m,
                    prob_n,
                    prob_k,
                    workspace.data_ptr(),
                    num_bits,
                    has_act_order,
                    is_k_full,
                    has_zp,
                    num_groups,
                    group_size,
                    dev,
                    at::cuda::getCurrentCUDAStream(dev),
                    thread_k,
                    thread_n,
                    sms,
                    marlin::max_par,
                    use_fp32_reduce);
  } else if (A.scalar_type() == at::ScalarType::BFloat16) {
    marlin_mm<nv_bfloat16>(A.data_ptr(),
                           B.data_ptr(),
                           C.data_ptr(),
                           c_tmp.data_ptr(),
                           scales.data_ptr(),
                           zeros.data_ptr(),
                           g_idx.data_ptr(),
                           perm.data_ptr(),
                           a_tmp.data_ptr(),
                           prob_m,
                           prob_n,
                           prob_k,
                           workspace.data_ptr(),
                           num_bits,
                           has_act_order,
                           is_k_full,
                           has_zp,
                           num_groups,
                           group_size,
                           dev,
                           at::cuda::getCurrentCUDAStream(dev),
                           thread_k,
                           thread_n,
                           sms,
                           marlin::max_par,
                           use_fp32_reduce);
  } else {
    LOG(FATAL) << "Unsupported scalar type = " << A.scalar_type();
  }
}

}  // namespace marlin