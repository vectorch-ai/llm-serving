#pragma once

#include <torch/torch.h>

#include <flashinfer/attention/warp_layout.cuh>

#include "handler.h"

namespace flashinfer {

class BatchPrefillWithPagedKVCachePyTorchWrapper {
 public:
  BatchPrefillWithPagedKVCachePyTorchWrapper(bool enable_cuda_graph)
      : handler_(std::make_shared<flashinfer::BatchPrefillHandler>(
            enable_cuda_graph)) {}

  void Plan(torch::Tensor float_workspace_buffer,
            torch::Tensor int_workspace_buffer,
            torch::Tensor qo_indptr,
            torch::Tensor page_kv_indptr,
            unsigned int batch_size,
            unsigned int num_qo_heads,
            unsigned int num_kv_heads,
            unsigned int head_dim,
            unsigned page_size,
            torch::Tensor empty_q_data);

  bool IsCUDAGraphEnabled() const { return handler_->IsCUDAGraphEnabled(); }

  void UpdatePageLockedBufferSize(uint32_t int_workspace_size_in_bytes);

  std::vector<torch::Tensor> Run(torch::Tensor q,
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
                                 bool return_lse);

 private:
  std::shared_ptr<flashinfer::BatchPrefillHandler> handler_;
};

}  // namespace flashinfer