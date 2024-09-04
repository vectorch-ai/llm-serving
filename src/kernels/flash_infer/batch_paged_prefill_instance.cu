#include "attention_kernel.h" // IWYU pragma: keep

namespace flashinfer {

constexpr PageStorage page_storage = PageStorage::kIndices;

template cudaError_t mha_varlen_dispatch<page_storage, WarpLayout::k4x1x2, 128, LogitsPostHook::kSoftCap, PosEncodingMode::kNone, true, MaskMode::kNone, nv_bfloat16, nv_bfloat16, nv_bfloat16, int32_t>(
    nv_bfloat16* q, int32_t* request_indices, int32_t* q_tile_indices, int32_t* kv_tile_indices,
    int32_t* q_indptr, int32_t* q_offset,
    paged_kv_t<page_storage, nv_bfloat16, int32_t> paged_kv, uint8_t* custom_mask,
    int32_t* qk_indptr, int32_t* o_indptr, nv_bfloat16* o, nv_bfloat16* tmp_v, float* tmp_s, float* lse,
    int32_t* merge_indptr, bool* block_valid_mask, int32_t* kv_chunk_size_ptr, uint32_t max_num_rows,
    uint32_t num_qo_heads, uint32_t padded_batch_size, int32_t window_left,
    float logits_soft_cap, float sm_scale, cudaStream_t stream);
    
template cudaError_t mha_varlen_dispatch<page_storage, WarpLayout::k4x1x1, 128, LogitsPostHook::kSoftCap, PosEncodingMode::kNone, true, MaskMode::kNone, nv_bfloat16, nv_bfloat16, nv_bfloat16, int32_t>(
    nv_bfloat16* q, int32_t* request_indices, int32_t* q_tile_indices, int32_t* kv_tile_indices,
    int32_t* q_indptr, int32_t* q_offset,
    paged_kv_t<page_storage, nv_bfloat16, int32_t> paged_kv, uint8_t* custom_mask,
    int32_t* qk_indptr, int32_t* o_indptr, nv_bfloat16* o, nv_bfloat16* tmp_v, float* tmp_s, float* lse,
    int32_t* merge_indptr, bool* block_valid_mask, int32_t* kv_chunk_size_ptr, uint32_t max_num_rows,
    uint32_t num_qo_heads, uint32_t padded_batch_size, int32_t window_left,
    float logits_soft_cap, float sm_scale, cudaStream_t stream);
    
template cudaError_t mha_varlen_dispatch<page_storage, WarpLayout::k1x4x1, 128, LogitsPostHook::kSoftCap, PosEncodingMode::kNone, true, MaskMode::kNone, nv_bfloat16, nv_bfloat16, nv_bfloat16, int32_t>(
    nv_bfloat16* q, int32_t* request_indices, int32_t* q_tile_indices, int32_t* kv_tile_indices,
    int32_t* q_indptr, int32_t* q_offset,
    paged_kv_t<page_storage, nv_bfloat16, int32_t> paged_kv, uint8_t* custom_mask,
    int32_t* qk_indptr, int32_t* o_indptr, nv_bfloat16* o, nv_bfloat16* tmp_v, float* tmp_s, float* lse,
    int32_t* merge_indptr, bool* block_valid_mask, int32_t* kv_chunk_size_ptr, uint32_t max_num_rows,
    uint32_t num_qo_heads, uint32_t padded_batch_size, int32_t window_left,
    float logits_soft_cap, float sm_scale, cudaStream_t stream);
    

}