#pragma once

#include <cute/atom/mma_atom.hpp>
#include <cute/swizzle_layout.hpp>
#include <cute/tensor.hpp>

#include "cute/config.hpp"
#include "cute/layout.hpp"
#include "cute/layout_composed.hpp"

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

template <size_t I, class IntTupleA, class IntTupleB>
CUTE_HOST_DEVICE constexpr auto elem_less(IntTupleA const& a,
                                          IntTupleB const& b) {
  return elem_less(get<I>(a), get<I>(b));
}

template <bool EVEN_K,
          bool EVEN_MN,
          bool ZERO_FILL_MN,
          bool ZERO_FILL_K,
          class TiledCopy,
          class TensorS,
          class TensorD,
          class TensorC,
          class Coord>
CUTE_HOST_DEVICE void safe_copy(
    const TiledCopy& tiled_copy,
    const TensorS& src,       // (CPY, CPY_M/N, CPY_K)
    TensorD& dst,             // (CPY, CPY_M/N, CPY_K)
    const TensorC& identity,  // (CPY, CPY_M/N, CPY_K) -> (blk_m/n, blk_k)
    const Coord& max_coord    // max_coord(blk_m/n, blk_k)
) {
  if constexpr (!EVEN_MN && !EVEN_K) {
    // handle both m/n and k oob
    CUTE_UNROLL
    for (int mi = 0; mi < size<1>(src); ++mi) {
      if (elem_less<0>(identity(_0{}, mi, _0{}), max_coord)) {
        CUTE_UNROLL
        for (int ki = 0; ki < size<2>(src); ++ki) {
          if (elem_less<1>(identity(_0{}, _0{}, ki), max_coord)) {
            copy(tiled_copy, src(_, mi, ki), dst(_, mi, ki));
          } else {
            if constexpr (ZERO_FILL_K) {
              clear(dst(_, mi, ki));
            }
          }
        }
      } else {
        if constexpr (ZERO_FILL_MN) {
          clear(dst(_, mi, _));
        }
      }
    }
  } else if constexpr (!EVEN_MN && EVEN_K) {
    // only handle m/n oob
    CUTE_UNROLL
    for (int mi = 0; mi < size<1>(src); ++mi) {
      if (elem_less<0>(identity(_0{}, mi, _0{}), max_coord)) {
        copy(tiled_copy, src(_, mi, _), dst(_, mi, _));
      } else {
        if constexpr (ZERO_FILL_MN) {
          clear(dst(_, mi, _));
        }
      }
    }
  } else if constexpr (EVEN_MN && !EVEN_K) {
    // only handle k oob
    CUTE_UNROLL
    for (int ki = 0; ki < size<2>(src); ++ki) {
      if (elem_less<1>(identity(_0{}, _0{}, ki), max_coord)) {
        copy(tiled_copy, src(_, _, ki), dst(_, _, ki));
      } else {
        if constexpr (ZERO_FILL_K) {
          clear(dst(_, _, ki));
        }
      }
    }
  } else {
    // no oob, just copy
    copy(tiled_copy, src, dst);
  }
}

template <int N, int I, class Shape, class Stride>
CUTE_HOST_DEVICE constexpr auto upcast(Shape const& shape,
                                       Stride const& stride) {
  if constexpr (is_tuple<Shape>::value) {
    return transform_layout(shape, stride, [](auto const& s, auto const& d) {
      return upcast<N, I>(s, d);
    });
  } else if constexpr (is_scaled_basis<Stride>::value) {
    if constexpr (Stride::mode() == I) {
      return make_layout(shape_div(shape, Int<N>{}),
                         shape_div(stride, Int<N>{}));
    } else {
      return make_layout(shape, stride);
    }
  } else {
    return upcast<N>(shape, stride);
  }

  CUTE_GCC_UNREACHABLE;
}

template <int N,
          class OuterShape,
          class OuterStride,
          class Offset,
          class Shape,
          class Stride>
CUTE_HOST_DEVICE constexpr auto upcast(
    ComposedLayout<Layout<OuterShape, OuterStride>,
                   Offset,
                   Layout<Shape, Stride>> const& layout) {
  // Find index of the stride-1 mode - that is the only one that requires
  // updating inner shape and offset
  auto idx = find_if(layout.layout_a().stride(),
                     [](auto x) { return is_constant<1, decltype(x)>{}; });
  constexpr int I = decltype(idx)::value;

  // Upcast the outer layout (works as expected)
  auto outer = upcast<N>(layout.layout_a());

  // Upcast the accumulated offset along stride-1 mode
  auto offset = as_arithmetic_tuple(
      replace<I>(layout.offset(), upcast<N>(get<I>(layout.offset()))));

  // Upcast the inner layout's shape along stride-1 mode
  auto inner =
      upcast<N, I>(layout.layout_b().shape(), layout.layout_b().stride());

  return composition(outer, offset, inner);
}

}  // namespace cute