#include <gtest/gtest.h>
#include <torch/torch.h>

#include <cstdint>

#include "attention_launch_sm80.cuh"
#include "attention_params.h"
#include "attention_ref.h"
#include "cute/layout.hpp"
#include "static_dispatch.h"

namespace llm {
namespace {
torch::Tensor attention_sm80(
    torch::Tensor query,  // [batch_size, q_len, n_heads, head_dim]
    torch::Tensor key,    // [batch_size, kv_len, n_kv_heads, head_dim]
    torch::Tensor value,  // [batch_size, kv_len, n_kv_heads, head_dim]
    torch::optional<torch::Tensor> alibi_slopes,  //[n_heads]
    float logits_soft_cap,
    int32_t sliding_window,
    int32_t max_q_len) {
  const auto batch_size = query.size(0);
  const auto q_len = query.size(-3);
  const auto kv_len = key.size(-3);
  const auto n_heads = query.size(-2);
  const auto n_kv_heads = key.size(-2);
  const auto head_dim = query.size(-1);

  auto out = torch::empty_like(query);

  const float sm_scale = 1.0 / sqrt(head_dim);

  // construct attention params
  AttentionParams params;
  params.q_ptr = query.const_data_ptr();
  params.q_stride =
      make_stride(query.stride(0), query.stride(1), query.stride(2));
  params.k_ptr = key.const_data_ptr();
  params.k_stride = make_stride(key.stride(0), key.stride(1), key.stride(2));
  params.v_ptr = value.const_data_ptr();
  params.v_stride =
      make_stride(value.stride(0), value.stride(1), value.stride(2));
  params.o_ptr = out.mutable_data_ptr();
  params.o_stride = make_stride(out.stride(0), out.stride(1), out.stride(2));
  params.alibi_slopes_ptr = alibi_slopes.has_value()
                                ? alibi_slopes.value().const_data_ptr<float>()
                                : nullptr;

  params.batch_size = batch_size;
  params.max_q_len = max_q_len;
  params.n_heads = n_heads;
  params.n_kv_heads = n_kv_heads;
  params.q_len = q_len;
  params.kv_len = kv_len;
  params.head_dim = head_dim;
  params.sm_scale = sm_scale;
  params.logits_soft_cap = logits_soft_cap;
  params.sliding_window = sliding_window;

  DISPATCH_TORCH_DTYPE(query.dtype(), DTYPE, [&] {
    DISPATCH_HEAD_DIM(head_dim, HEAD_DIM, [&] {
      run_attention_kernel_sm80<DTYPE, HEAD_DIM>(params);
    });
  });
  return out;
}

}  // namespace

class AttentionKernelTest
    : public ::testing::TestWithParam<std::tuple<int64_t /*batch_size*/,
                                                 int64_t /*q_len*/,
                                                 int64_t /*kv_len*/,
                                                 int64_t /*n_heads*/,
                                                 int64_t /*n_kv_heads*/,
                                                 int64_t /*head_dim*/,
                                                 float /*logits_soft_cap*/,
                                                 bool /*alibi*/,
                                                 int32_t /*sliding_window*/>> {
 public:
  void SetUp() override {
    // Set random seed for test stability
    torch::manual_seed(0);
  }
};

TEST_P(AttentionKernelTest, MHA) {
  const auto [batch_size,
              q_len,
              kv_len,
              n_heads,
              n_kv_heads,
              head_dim,
              logits_soft_cap,
              alibi,
              sliding_window] = GetParam();

  const auto options = torch::dtype(torch::kHalf).device(torch::kCUDA);

  // construct non-contiguous query, key and value
  const auto data = torch::randn(
      {batch_size, q_len, n_heads + 2 * n_kv_heads, head_dim}, options);
  const auto qkv =
      data.split(/*split_size=*/{n_heads, n_kv_heads, n_kv_heads}, /*dim=*/2);
  const auto& query = qkv[0];
  const auto& key = qkv[1];
  const auto& value = qkv[2];

  torch::optional<torch::Tensor> alibi_slopes;
  if (alibi) {
    alibi_slopes = torch::rand(
        {n_heads}, torch::dtype(torch::kFloat32).device(torch::kCUDA));
  }

  auto ref_out = attention_batch_ref(
      query, key, value, alibi_slopes, logits_soft_cap, sliding_window);
  auto out = attention_sm80(
      query, key, value, alibi_slopes, logits_soft_cap, sliding_window, q_len);

  EXPECT_TRUE(torch::allclose(out, ref_out, /*rtol=*/1e-3, /*atol=*/1e-3));
}

INSTANTIATE_TEST_SUITE_P(
    MHA,
    AttentionKernelTest,
    ::testing::Combine(
        ::testing::Values(1, 2, 4),                          // batch_size
        ::testing::Values(1, 62, 125),                       // q_len
        ::testing::Values(127, 287, 1000),                   // kv_len
        ::testing::Values(6),                                // n_heads
        ::testing::Values(6 /*mha*/, 3 /*gqa*/, 1 /*mqa*/),  // n_kv_heads
        ::testing::Values(32, 64, 96, 128, 256),             // head_dim
        ::testing::Values(0.0, 50.0),                        // logits_soft_cap
        ::testing::Values(false, true),                      // alibi slope
        ::testing::Values(-1, 0, 10)                         // sliding window
        ));

}  // namespace llm