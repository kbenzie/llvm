// RUN: %{build} -o %t1.out
// REQUIRES: cuda || hip
// RUN: %{run} %t1.out

//==---------------- memadvise_flags.cpp -----------------------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <sycl/detail/core.hpp>
#include <sycl/usm.hpp>
#include <vector>

using namespace sycl;

int main() {
  const size_t size = 100;
  queue q;
  auto dev = q.get_device();
  auto ctx = q.get_context();
  if (!dev.get_info<info::device::usm_shared_allocations>()) {
    std::cout << "Shared USM is not supported. Skipping test." << std::endl;
    return 0;
  }

  void *ptr = malloc_shared(size, dev, ctx);
  if (ptr == nullptr) {
    std::cout << "Allocation failed!" << std::endl;
    return -1;
  }

  bool isCuda = dev.get_backend() == sycl::backend::ext_oneapi_cuda;
  bool isHip = dev.get_backend() == sycl::backend::ext_oneapi_hip;

  std::vector<int> valid_advices;
  if (isCuda || isHip) {
    valid_advices.emplace_back(UR_USM_ADVICE_FLAG_SET_READ_MOSTLY);
    valid_advices.emplace_back(UR_USM_ADVICE_FLAG_CLEAR_READ_MOSTLY);
    valid_advices.emplace_back(UR_USM_ADVICE_FLAG_SET_PREFERRED_LOCATION);
    valid_advices.emplace_back(UR_USM_ADVICE_FLAG_CLEAR_PREFERRED_LOCATION);
    valid_advices.emplace_back(UR_USM_ADVICE_FLAG_SET_ACCESSED_BY_DEVICE);
    valid_advices.emplace_back(UR_USM_ADVICE_FLAG_CLEAR_ACCESSED_BY_DEVICE);
    valid_advices.emplace_back(UR_USM_ADVICE_FLAG_SET_PREFERRED_LOCATION_HOST);
    valid_advices.emplace_back(
        UR_USM_ADVICE_FLAG_CLEAR_PREFERRED_LOCATION_HOST);
    valid_advices.emplace_back(UR_USM_ADVICE_FLAG_SET_ACCESSED_BY_HOST);
    valid_advices.emplace_back(UR_USM_ADVICE_FLAG_CLEAR_ACCESSED_BY_HOST);
  } else {
    // Skip
    return 0;
  }

  if (isHip) {
    valid_advices.emplace_back(UR_USM_ADVICE_FLAG_SET_NON_COHERENT_MEMORY);
    valid_advices.emplace_back(UR_USM_ADVICE_FLAG_CLEAR_NON_COHERENT_MEMORY);
  }

  for (int advice : valid_advices) {
    q.mem_advise(ptr, size, advice);
  }

  q.wait_and_throw();
  std::cout << "Test passed." << std::endl;
  return 0;
}
