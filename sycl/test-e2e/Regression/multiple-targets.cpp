// This test checks if CUDA and HIP can be compiled and run with spirv.
// It tests if the target triples can be specified with any order.
// The test is repeated for per_kernel device code splitting.
//
// REQUIRES: (target-nvidia || target-amd || target-native_cpu) && any-target-is-spir
// RUN: %clangxx -fsycl -fsycl-targets=%{sycl_triple},spir64 %if target-amd %{ %{hip_arch_opts} %} -o %t1.out %s
// RUN: %{run} %t1.out
//
// RUN: %clangxx -fsycl -fsycl-targets=spir64,%{sycl_triple} %if target-amd %{ %{hip_arch_opts} %} -o %t2.out %s
// RUN: %{run} %t2.out
//
// RUN: %clangxx -fsycl -fsycl-targets=%{sycl_triple},spir64 %if target-amd %{ %{hip_arch_opts} %} -fsycl-device-code-split=per_kernel -o %t3.out %s
// RUN: %{run} %t3.out
//
// RUN: %clangxx -fsycl -fsycl-targets=spir64,%{sycl_triple} %if target-amd %{ %{hip_arch_opts} %} -fsycl-device-code-split=per_kernel -o %t4.out %s
// RUN: %{run} %t4.out

#include <sycl/detail/core.hpp>

int main() {
  sycl::queue Q;
  int A_Data[10] = {0};
  int B_Data[10] = {4};
  int C_Data[10] = {-1};

  {
    sycl::buffer<int, 1> A_Buf(A_Data, sycl::range<1>(10));

    Q.submit([&](sycl::handler &Cgh) {
      auto A_Acc = A_Buf.get_access<sycl::access::mode::write>(Cgh);
      Cgh.parallel_for(sycl::range<1>{5},
                       [=](sycl::id<1> index) { A_Acc[index] = 5; });
    });
  }

  assert(A_Data[0] == 5);

  {
    sycl::buffer<int, 1> B_Buf(B_Data, sycl::range<1>(10));
    sycl::buffer<int, 1> C_Buf(C_Data, sycl::range<1>(10));

    Q.submit([&](sycl::handler &Cgh) {
      auto B_Acc = B_Buf.get_access<sycl::access::mode::read_write>(Cgh);
      auto C_Acc = C_Buf.get_access<sycl::access::mode::read>(Cgh);
      Cgh.parallel_for(sycl::range<1>{5}, [=](sycl::id<1> index) {
        B_Acc[index] += C_Acc[index];
      });
    });
  }

  assert(B_Data[0] == 3);

  {
    sycl::buffer<int, 1> B_Buf(B_Data, sycl::range<1>(10));
    sycl::buffer<int, 1> C_Buf(C_Data, sycl::range<1>(10));

    Q.submit([&](sycl::handler &Cgh) {
      auto B_Acc = B_Buf.get_access<sycl::access::mode::read>(Cgh);
      auto C_Acc = C_Buf.get_access<sycl::access::mode::write>(Cgh);
      Cgh.parallel_for(sycl::range<1>{5},
                       [=](sycl::id<1> index) { C_Acc[index] = B_Acc[index]; });
    });
  }

  assert(C_Data[0] == 3);

  return 0;
}
