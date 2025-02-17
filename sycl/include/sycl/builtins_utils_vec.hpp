//==--- builtins_utils_vec.hpp - SYCL built-in function utilities for vec --==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include <sycl/detail/type_traits/vec_marray_traits.hpp>

#include <sycl/builtins_utils_scalar.hpp>

#include <sycl/detail/type_traits.hpp>
#include <sycl/detail/vector_convert.hpp>
#include <sycl/marray.hpp> // for marray
#include <sycl/vector.hpp> // for vec

namespace sycl {
inline namespace _V1 {
namespace detail {

// Utilty trait for checking that the number of elements in T is in Ns.
template <typename T, size_t... Ns>
struct is_valid_size
    : std::bool_constant<check_size_in_v<num_elements<T>::value, Ns...>> {};

template <typename T, int... Ns>
constexpr bool is_valid_size_v = is_valid_size<T, Ns...>::value;

// Utility for converting a swizzle to a vector or preserve the type if it isn't
// a swizzle.
template <typename VecT, typename OperationLeftT, typename OperationRightT,
          template <typename> class OperationCurrentT, int... Indexes>
struct simplify_if_swizzle<SwizzleOp<VecT, OperationLeftT, OperationRightT,
                                     OperationCurrentT, Indexes...>> {
  using type = vec<typename VecT::element_type, sizeof...(Indexes)>;
};

template <typename T1, typename T2>
struct is_same_op<
    T1, T2,
    std::enable_if_t<is_vec_or_swizzle_v<T1> && is_vec_or_swizzle_v<T2>>>
    : std::is_same<simplify_if_swizzle_t<T1>, simplify_if_swizzle_t<T2>> {};

// Utility trait for changing the element type of a type T. If T is a scalar,
// the new type replaces T completely.
template <typename NewElemT, typename T> struct change_elements {
  using type = NewElemT;
};
template <typename NewElemT, typename T, size_t N>
struct change_elements<NewElemT, marray<T, N>> {
  using type = marray<typename change_elements<NewElemT, T>::type, N>;
};
template <typename NewElemT, typename T, int N>
struct change_elements<NewElemT, vec<T, N>> {
  using type = vec<typename change_elements<NewElemT, T>::type, N>;
};
template <typename NewElemT, typename VecT, typename OperationLeftT,
          typename OperationRightT, template <typename> class OperationCurrentT,
          int... Indexes>
struct change_elements<NewElemT,
                       SwizzleOp<VecT, OperationLeftT, OperationRightT,
                                 OperationCurrentT, Indexes...>> {
  // Converts to vec for simplicity.
  using type =
      vec<typename change_elements<NewElemT, typename VecT::element_type>::type,
          sizeof...(Indexes)>;
};

template <typename NewElemT, typename T>
using change_elements_t = typename change_elements<NewElemT, T>::type;

// Utility functions for converting to/from vec/marray.
template <class T, size_t N> vec<T, 2> to_vec2(marray<T, N> X, size_t Start) {
  return {X[Start], X[Start + 1]};
}
template <class T, size_t N> vec<T, N> to_vec(marray<T, N> X) {
  vec<T, N> Vec;
  for (size_t I = 0; I < N; I++)
    Vec[I] = X[I];
  return Vec;
}
template <class T, int N> marray<T, N> to_marray(vec<T, N> X) {
  marray<T, N> Marray;
  for (size_t I = 0; I < N; I++)
    Marray[I] = X[I];
  return Marray;
}

} // namespace detail
} // namespace _V1
} // namespace sycl
