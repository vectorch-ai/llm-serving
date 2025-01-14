#pragma once

#include <cute/atom/mma_atom.hpp>
#include <cute/swizzle_layout.hpp>
#include <cute/tensor.hpp>

#include "cute/config.hpp"

namespace cute {

template <class... Args, class ATensor>
CUTE_HOST_DEVICE constexpr auto partition_fragment_A(
    const ThrMMA<Args...>& thr_mma,
    const ATensor& atensor) {
  auto atensor_noswizzle =
      make_tensor(atensor.data(), get_nonswizzle_portion(atensor.layout()));
  return thr_mma.partition_fragment_A(atensor_noswizzle);
}

template <class... Args, class BTensor>
CUTE_HOST_DEVICE constexpr auto partition_fragment_B(
    const ThrMMA<Args...>& thr_mma,
    const BTensor& btensor) {
  auto btensor_noswizzle =
      make_tensor(btensor.data(), get_nonswizzle_portion(btensor.layout()));
  return thr_mma.partition_fragment_B(btensor_noswizzle);
}

template <bool ZERO_FILL = false,
          class TiledCopy,
          class TensorS,
          class TensorD,
          class TensorC>
CUTE_HOST_DEVICE void safe_copy(
    const TiledCopy& tiled_copy,
    const TensorS& src,       // (CPY, CPY_M/N, CPY_K)
    TensorD& dst,             // (CPY, CPY_M/N, CPY_K)
    const TensorC& identity,  // (CPY, CPY_M/N, CPY_K) -> (blk_m/n, blk_k)
    const int max_coord       // max_coord_m/n
) {
  CUTE_UNROLL
  for (int mi = 0; mi < size<1>(src); ++mi) {
    if (get<0>(identity(0, mi, 0)) < max_coord) {
      cute::copy(tiled_copy, src(_, mi, _), dst(_, mi, _));
    } else {
      if constexpr (ZERO_FILL) {
        cute::clear(dst(_, mi, _));
      }
    }
  }
}

struct Identical {
  template <typename Index>
  CUTE_HOST_DEVICE constexpr auto operator()(Index i) const {
    return i;
  }
};

// dynamic stride that mapping stride based on index
template <typename Stride, typename Func = Identical>
struct DynamicStride {
  Func func_;
  Stride stride_;

  CUTE_HOST_DEVICE constexpr DynamicStride(const Func& func,
                                           const Stride& stride)
      : func_(func), stride_(stride) {}

  // overloads operator* to map seq_idx to slot_idx
  // slot_idx = block_table[seq_idx/block_size]*block_size + seq_idx%block_size
  template <typename Index>
  CUTE_HOST_DEVICE friend constexpr auto operator*(Index i,
                                                   const DynamicStride& s) {
    return s.func_(i) * s.stride_;
  }

  template <typename Index>
  CUTE_HOST_DEVICE friend constexpr auto operator*(const DynamicStride& s,
                                                   Index i) {
    return s.func_(i) * s.stride_;
  }
};

}  // namespace cute