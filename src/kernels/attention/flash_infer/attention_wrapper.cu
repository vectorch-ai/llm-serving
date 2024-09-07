/*
 * Copyright (c) 2023 by FlashInfer team.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <c10/cuda/CUDAStream.h>
#include <torch/torch.h>

#include <flashinfer/attention/logits_post_hook.cuh>
#include <flashinfer/attention/mask.cuh>
#include <flashinfer/attention/warp_layout.cuh>

#include "attention_wrapper.h"
#include "kv_cache.h"
#include "pytorch_extension_utils.h"

namespace flashinfer {

template <WarpLayout WARP_LAYOUT,
          uint32_t HEAD_DIM,
          LogitsPostHook LOGITS_POST_HOOK,
          PosEncodingMode pos_encoding_mode,
          bool ALLOW_FP16_QK_REDUCTION,
          MaskMode MASK_MODE,
          typename DTypeQ,
          typename DTypeKV,
          typename DTypeOut,
          typename IdType>
cudaError_t mha_varlen_dispatch(DTypeQ* q,
                                IdType* request_indices,
                                IdType* q_tile_indices,
                                IdType* kv_tile_indices,
                                IdType* q_indptr,
                                IdType* q_offset,
                                paged_kv_t<DTypeKV, IdType> paged_kv,
                                uint8_t* custom_mask,
                                IdType* qk_indptr,
                                IdType* o_indptr,
                                DTypeOut* o,
                                DTypeOut* tmp_v,
                                float* tmp_s,
                                float* lse,
                                IdType* merge_indptr,
                                bool* block_valid_mask,
                                IdType* kv_chunk_size_ptr,
                                uint32_t total_num_rows,
                                uint32_t num_qo_heads,
                                uint32_t padded_batch_size,
                                int32_t window_left,
                                float logits_soft_cap,
                                float sm_scale,
                                cudaStream_t stream);

template <uint32_t HEAD_DIM,
          LogitsPostHook LOGITS_POST_HOOK,
          PosEncodingMode POS_ENCODING_MODE,
          bool ALLOW_FP16_QK_REDUCTION,
          MaskMode MASK_MODE,
          typename DTypeQ,
          typename DTypeKV,
          typename DTypeOut,
          typename IdType>
cudaError_t BatchPrefillWithPagedKVCacheWrapperDispatched(
    BatchPrefillHandler* handler,
    DTypeQ* q,
    IdType* q_indptr,
    IdType* q_offset,
    paged_kv_t<DTypeKV, IdType> paged_kv,
    uint8_t* custom_mask,
    IdType* qk_indptr,
    DTypeOut* o,
    float* lse,
    uint32_t num_qo_heads,
    int32_t window_left,
    float logits_soft_cap,
    float sm_scale,
    cudaStream_t stream) {
  DTypeOut* tmp_v = nullptr;
  float* tmp_s = nullptr;
  IdType *request_indices = nullptr, *qo_tile_indices = nullptr,
         *kv_tile_indices = nullptr, *o_indptr = nullptr,
         *merge_indptr = nullptr, *kv_chunk_size_ptr = nullptr;
  bool* block_valid_mask = nullptr;
  WarpLayout warp_layout;
  uint32_t padded_batch_size = 0U;
  uint32_t total_num_rows = 0U;
  tmp_v = handler->GetTempV<DTypeOut>();
  tmp_s = handler->GetTempS();
  request_indices = handler->GetRequestIndices<IdType>();
  qo_tile_indices = handler->GetQOTileIndices<IdType>();
  kv_tile_indices = handler->GetKVTileIndices<IdType>();
  block_valid_mask = handler->GetBlockValidMask();
  o_indptr = handler->GetOIndptr<IdType>();
  merge_indptr = handler->GetMergeIndptr<IdType>();
  kv_chunk_size_ptr = handler->GetKVChunkSizePtr<IdType>();
  warp_layout = handler->GetWarpLayout();
  padded_batch_size = handler->GetPaddedBatchSize();
  total_num_rows = handler->GetTotalNumRows();

  DISPATCH_WARP_LAYOUT(warp_layout, WARP_LAYOUT, {
    return mha_varlen_dispatch<WARP_LAYOUT,
                               HEAD_DIM,
                               LOGITS_POST_HOOK,
                               POS_ENCODING_MODE,
                               ALLOW_FP16_QK_REDUCTION,
                               MASK_MODE,
                               DTypeQ,
                               DTypeKV,
                               DTypeOut,
                               IdType>(q,
                                       request_indices,
                                       qo_tile_indices,
                                       kv_tile_indices,
                                       q_indptr,
                                       q_offset,
                                       paged_kv,
                                       custom_mask,
                                       qk_indptr,
                                       o_indptr,
                                       o,
                                       tmp_v,
                                       tmp_s,
                                       lse,
                                       merge_indptr,
                                       block_valid_mask,
                                       kv_chunk_size_ptr,
                                       total_num_rows,
                                       num_qo_heads,
                                       padded_batch_size,
                                       window_left,
                                       logits_soft_cap,
                                       sm_scale,
                                       stream);
  });
  return cudaSuccess;
}

void BatchPrefillWithPagedKVCachePyTorchWrapper::Plan(
    torch::Tensor float_workspace_buffer,
    torch::Tensor int_workspace_buffer,
    torch::Tensor qo_indptr,
    torch::Tensor paged_kv_indptr,
    unsigned int batch_size,
    unsigned int num_qo_heads,
    unsigned int num_kv_heads,
    unsigned int head_dim,
    unsigned int page_size,
    torch::Tensor empty_q_data) {
  CHECK_INPUT(float_workspace_buffer);
  CHECK_INPUT(int_workspace_buffer);
  // NOTE(Zihao): not necessary to be a CUDA tensor
  CHECK_CONTIGUOUS(qo_indptr);
  CHECK_CONTIGUOUS(paged_kv_indptr);
  CHECK_GQA_HEAD_DIVISIBLE(num_qo_heads, num_kv_heads);
  CHECK_DIM(1, qo_indptr);
  CHECK_DIM(1, paged_kv_indptr);
  CHECK_DIM(1, float_workspace_buffer);
  CHECK_DIM(1, int_workspace_buffer);
  CHECK_EQ(qo_indptr.size(0), batch_size + 1);
  CHECK_EQ(paged_kv_indptr.size(0), batch_size + 1);
  // qo_indptr = qo_indptr.to(torch::dtype(torch::kInt32).device(torch::kCPU));
  // paged_kv_indptr =
  //     paged_kv_indptr.to(torch::dtype(torch::kInt32).device(torch::kCPU));
  // auto device = float_workspace_buffer.device();
  // size_t float_workspace_size_in_bytes =
  //     float_workspace_buffer.size(0) * float_workspace_buffer.element_size();
  // size_t int_workspace_size_in_bytes =
  //     int_workspace_buffer.size(0) * int_workspace_buffer.element_size();
  // cudaStream_t torch_current_stream =
  //     c10::cuda::getCurrentCUDAStream(device.index());
  // handler_->SetCUDAStream(torch_current_stream);

  // DISPATCH_PYTORCH_DTYPE_TO_CTYPE_FP16(empty_q_data.scalar_type(), q_type,
  // [&] {
  //   cudaError_t status = handler_->Plan<q_type, int32_t>(
  //       static_cast<void*>(float_workspace_buffer.data_ptr()),
  //       float_workspace_size_in_bytes,
  //       static_cast<void*>(int_workspace_buffer.data_ptr()),
  //       int_workspace_size_in_bytes,
  //       static_cast<int32_t*>(qo_indptr.data_ptr()),
  //       static_cast<int32_t*>(paged_kv_indptr.data_ptr()),
  //       batch_size,
  //       num_qo_heads,
  //       num_kv_heads,
  //       head_dim,
  //       page_size);
  //   TORCH_CHECK(status == cudaSuccess,
  //               "BatchPrefillWithPagedKVCache failed with error ",
  //               cudaGetErrorString(status));
  //   return true;
  // });
}

void BatchPrefillWithPagedKVCachePyTorchWrapper::UpdatePageLockedBufferSize(
    unsigned int int_workspace_size_in_bytes) {
  handler_->UpdatePageLockedBufferSize(int_workspace_size_in_bytes);
}

std::vector<torch::Tensor> BatchPrefillWithPagedKVCachePyTorchWrapper::Run(
    torch::Tensor q,
    torch::Tensor qo_indptr,
    std::optional<torch::Tensor> paged_k_cache,
    std::optional<torch::Tensor> paged_v_cache,
    torch::Tensor paged_kv_indptr,
    torch::Tensor paged_kv_indices,
    torch::Tensor paged_kv_last_page_len,
    bool causal,
    unsigned int pos_encoding_mode,
    bool allow_fp16_qk_reduction,
    int window_left,
    float logits_soft_cap,
    float sm_scale,
    bool return_lse) {
  CHECK_INPUT(q);
  CHECK_INPUT(qo_indptr);
  CHECK_INPUT(paged_k_cache.value());
  CHECK_INPUT(paged_v_cache.value());
  CHECK_INPUT(paged_kv_indptr);
  CHECK_INPUT(paged_kv_indices);
  CHECK_INPUT(paged_kv_last_page_len);
  auto device = q.device();
  CHECK_EQ(device, qo_indptr.device());
  CHECK_EQ(device, paged_k_cache->device());
  CHECK_EQ(device, paged_v_cache->device());
  CHECK_EQ(device, paged_kv_indptr.device());
  CHECK_EQ(device, paged_kv_indices.device());
  CHECK_EQ(device, paged_kv_last_page_len.device());
  CHECK_DIM(3, q);          // (nnz_qo, H_qo, D)
  CHECK_DIM(1, qo_indptr);  // (B + 1,)

  // [max_num_pages, num_kv_heads, page_size, head_dim] for HND
  // [max_num_pages, page_size, num_kv_heads, head_dim] for HND
  CHECK_DIM(4, paged_k_cache.value());
  CHECK_DIM(4, paged_v_cache.value());

  CHECK_DIM(1, paged_kv_indptr);         // (B + 1,)
  CHECK_DIM(1, paged_kv_indices);        // (nnz_kv,)
  CHECK_DIM(1, paged_kv_last_page_len);  // (B,)
  int64_t batch_size = qo_indptr.size(0) - 1;
  int64_t nnz_qo = q.size(0);
  int64_t num_qo_heads = q.size(1);
  int64_t head_dim = q.size(2);
  int64_t num_kv_heads, page_size;

  CHECK_EQ(paged_k_cache->size(3), head_dim);
  CHECK_EQ(paged_v_cache->size(3), head_dim);
  page_size = paged_k_cache->size(1);
  num_kv_heads = paged_k_cache->size(2);

  CHECK_GQA_HEAD_DIVISIBLE(num_qo_heads, num_kv_heads);
  CHECK_GE(qo_indptr.size(0), batch_size + 1);
  CHECK_GE(paged_kv_indptr.size(0), batch_size + 1);
  CHECK_GE(paged_kv_last_page_len.size(0), batch_size);
  qo_indptr = qo_indptr.to(torch::kInt32);
  paged_kv_indptr = paged_kv_indptr.to(torch::kInt32);
  paged_kv_indices = paged_kv_indices.to(torch::kInt32);
  paged_kv_last_page_len = paged_kv_last_page_len.to(torch::kInt32);

  cudaStream_t torch_current_stream =
      c10::cuda::getCurrentCUDAStream(device.index());
  torch::Tensor o = torch::empty_like(q, q.options());
  torch::Tensor lse = torch::empty({0});
  if (return_lse) {
    lse = torch::empty({nnz_qo, num_qo_heads},
                       q.options().dtype(torch::kFloat32));
  }
  // MaskMode mask_mode = causal ? MaskMode::kCausal : MaskMode::kNone;
  // TORCH_CHECK(logits_soft_cap >= 0.f, "logits_soft_cap must be
  // non-negative"); const LogitsPostHook logits_post_hook =
  //     logits_soft_cap > 0.f ? LogitsPostHook::kSoftCap :
  //     LogitsPostHook::kNone;

  // auto q_scalar_type = q.scalar_type();
  // auto kv_scalar_type = paged_k_cache->scalar_type();

  // if (q_scalar_type == kv_scalar_type) {
  //   DISPATCH_PYTORCH_DTYPE_TO_CTYPE_FP16(q_scalar_type, c_type, [&] {
  //     return DISPATCH_logits_post_hook(logits_post_hook, LOGITS_POST_HOOK,
  //     [&] {
  //       paged_kv_t<c_type, int32_t> paged_kv(
  //           num_kv_heads,
  //           page_size,
  //           head_dim,
  //           batch_size,
  //           static_cast<c_type*>(paged_k_cache.has_value()
  //                                    ? paged_k_cache->data_ptr()
  //                                    : nullptr),
  //           static_cast<c_type*>(paged_v_cache.has_value()
  //                                    ? paged_v_cache->data_ptr()
  //                                    : nullptr),
  //           static_cast<int32_t*>(paged_kv_indices.data_ptr()),
  //           static_cast<int32_t*>(paged_kv_indptr.data_ptr()),
  //           static_cast<int32_t*>(paged_kv_last_page_len.data_ptr()));
  //       return DISPATCH_head_dim(head_dim, HEAD_DIM, [&] {
  //         return DISPATCH_mask_mode(mask_mode, MASK_MODE, [&] {
  //           return DISPATCH_allow_fp16_qk_reduction(
  //               allow_fp16_qk_reduction, ALLOW_FP16_QK_REDUCTION, [&] {
  //                 return DISPATCH_pos_encoding_mode(
  //                     PosEncodingMode(pos_encoding_mode),
  //                     POS_ENCODING_MODE,
  //                     [&] {
  //                       cudaError_t status =
  //                           BatchPrefillWithPagedKVCacheWrapperDispatched<
  //                               HEAD_DIM,
  //                               LOGITS_POST_HOOK,
  //                               POS_ENCODING_MODE,
  //                               ALLOW_FP16_QK_REDUCTION,
  //                               MASK_MODE,
  //                               c_type,
  //                               c_type,
  //                               c_type,
  //                               int32_t>(
  //                               handler_.get(),
  //                               static_cast<c_type*>(q.data_ptr()),
  //                               static_cast<int32_t*>(qo_indptr.data_ptr()),
  //                               /*q_offset=*/nullptr,
  //                               paged_kv,
  //                               /*custom_mask=*/nullptr,
  //                               /*qk_indptr=*/nullptr,
  //                               static_cast<c_type*>(o.data_ptr()),
  //                               /*lse=*/
  //                                   return_lse
  //                                   ? static_cast<float*>(lse.data_ptr())
  //                                   : nullptr,
  //                               num_qo_heads,
  //                               window_left,
  //                               logits_soft_cap,
  //                               sm_scale,
  //                               /*stream=*/torch_current_stream);
  //                       TORCH_CHECK(status == cudaSuccess,
  //                                   "BatchPrefillWithPagedKVCache failed with
  //                                   " "error code ",
  //                                   cudaGetErrorString(status));
  //                       return true;
  //                     });
  //               });
  //         });
  //       });
  //     });
  //   });
  // } else {
  //   DISPATCH_PYTORCH_DTYPE_TO_CTYPE_FP16(q_scalar_type, q_type, [&] {
  //     return DISPATCH_PYTORCH_DTYPE_TO_CTYPE_FP8(kv_scalar_type, kv_type, [&]
  //     {
  //       return DISPATCH_logits_post_hook(
  //           logits_post_hook, LOGITS_POST_HOOK, [&] {
  //             paged_kv_t<kv_type, int32_t> paged_kv(
  //                 num_kv_heads,
  //                 page_size,
  //                 head_dim,
  //                 batch_size,
  //                 static_cast<kv_type*>(paged_k_cache.has_value()
  //                                           ? paged_k_cache->data_ptr()
  //                                           : nullptr),
  //                 static_cast<kv_type*>(paged_v_cache.has_value()
  //                                           ? paged_v_cache->data_ptr()
  //                                           : nullptr),
  //                 static_cast<int32_t*>(paged_kv_indices.data_ptr()),
  //                 static_cast<int32_t*>(paged_kv_indptr.data_ptr()),
  //                 static_cast<int32_t*>(paged_kv_last_page_len.data_ptr()));
  //             return DISPATCH_head_dim(head_dim, HEAD_DIM, [&] {
  //               return DISPATCH_mask_mode(mask_mode, MASK_MODE, [&] {
  //                 return DISPATCH_allow_fp16_qk_reduction(
  //                     allow_fp16_qk_reduction, ALLOW_FP16_QK_REDUCTION, [&] {
  //                       return DISPATCH_pos_encoding_mode(
  //                           PosEncodingMode(pos_encoding_mode),
  //                           POS_ENCODING_MODE,
  //                           [&] {
  //                             cudaError_t status =
  //                                 BatchPrefillWithPagedKVCacheWrapperDispatched<
  //                                     HEAD_DIM,
  //                                     LOGITS_POST_HOOK,
  //                                     POS_ENCODING_MODE,
  //                                     ALLOW_FP16_QK_REDUCTION,
  //                                     MASK_MODE,
  //                                     q_type,
  //                                     kv_type,
  //                                     q_type,
  //                                     int32_t>(
  //                                     handler_.get(),
  //                                     static_cast<q_type*>(q.data_ptr()),
  //                                     static_cast<int32_t*>(
  //                                         qo_indptr.data_ptr()),
  //                                     /*q_offset=*/nullptr,
  //                                     paged_kv,
  //                                     /*custom_mask=*/nullptr,
  //                                     /*qk_indptr=*/nullptr,
  //                                     static_cast<q_type*>(o.data_ptr()),
  //                                     /*lse=*/
  //                                         return_lse
  //                                         ?
  //                                         static_cast<float*>(lse.data_ptr())
  //                                         : nullptr,
  //                                     num_qo_heads,
  //                                     window_left,
  //                                     logits_soft_cap,
  //                                     sm_scale,
  //                                     /*stream=*/torch_current_stream);
  //                             TORCH_CHECK(status == cudaSuccess,
  //                                         "BatchPrefillWithPagedKVCache
  //                                         failed " "with error code ",
  //                                         cudaGetErrorString(status));
  //                             return true;
  //                           });
  //                     });
  //               });
  //             });
  //           });
  //     });
  //   });
  // }

  if (return_lse) {
    return {o, lse};
  } else {
    return {o};
  }
}

}  // namespace flashinfer