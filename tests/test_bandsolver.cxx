#include <iostream>

#include <gtest/gtest.h>

#include "gtensor/bandsolver.h"
#include "gtensor/gtensor.h"

#include "test_helpers.h"

template <typename T>
void set_A0(T&& h_A)
{
  // matlab/octave:
  //  A = [1 2 2; 4 4 2; 4 6 4];
  //  L,U,p = lu(A)
  // first column
  h_A(0, 0) = 1;
  h_A(1, 0) = 4;
  h_A(2, 0) = 4;
  // second column
  h_A(0, 1) = 2;
  h_A(1, 1) = 4;
  h_A(2, 1) = 6;
  // third column
  h_A(0, 2) = 2;
  h_A(1, 2) = 2;
  h_A(2, 2) = 4;
}

template <typename T>
void set_A0_nopiv(T&& h_A)
{
  // matlab/octave:
  // does not require pivoting even if pivoting is possible
  //  A = [1 0 0; 1 2 0; 0 2 3];
  //  L,U,p = lu(A)
  // first column
  h_A(0, 0) = 1.;
  h_A(1, 0) = 0.;
  h_A(2, 0) = 0.;
  // second column
  h_A(0, 1) = 1.;
  h_A(1, 1) = 2.;
  h_A(2, 1) = 0.;
  // third column
  h_A(0, 2) = 0.;
  h_A(1, 2) = 2.;
  h_A(2, 2) = 3.;
}

template <typename T>
void set_A0_LU(T&& h_A)
{
  // first column factored
  h_A(0, 0) = 4.0;
  h_A(1, 0) = 1.0;
  h_A(2, 0) = 0.25;
  // second column
  h_A(0, 1) = 4.0;
  h_A(1, 1) = 2.0;
  h_A(2, 1) = 0.5;
  // thrid column
  h_A(0, 2) = 2.0;
  h_A(1, 2) = 2.0;
  h_A(2, 2) = 0.5;
}

template <typename T>
void set_A0_piv(T&& h_p)
{
  h_p(0) = 2;
  h_p(1) = 3;
  h_p(2) = 3;
}

template <typename C>
void set_A1_complex(C&& h_A1)
{
  using T = typename C::value_type;

  // second matrix, complex
  // matlab/octave:
  //  B = [1+i 2-i 2; 4i 4 2; 4 6i 4];
  //  L,U,p = lu(A2);
  // first column
  h_A1(0, 0) = T(1, 1);
  h_A1(1, 0) = T(0, 4);
  h_A1(2, 0) = T(4, 0);
  // second column
  h_A1(0, 1) = T(2, -1);
  h_A1(1, 1) = T(4, 0);
  h_A1(2, 1) = T(0, 6);
  // third column
  h_A1(0, 2) = T(2, 0);
  h_A1(1, 2) = T(2, 0);
  h_A1(2, 2) = T(4, 0);
}

template <typename C>
void set_A1_complex_nopiv(C&& h_A1)
{
  using T = typename C::value_type;

  // second matrix, complex
  // matlab/octave:
  //  B = [1+i 2-i 2; 4i 4 2; 4 6i 4];
  //  L,U,p = lu(A2);
  // first column
  h_A1(0, 0) = T(1, 0);
  h_A1(1, 0) = T(0, 0);
  h_A1(2, 0) = T(0, 0);
  // second column
  h_A1(0, 1) = T(1, 0);
  h_A1(1, 1) = T(2, 0);
  h_A1(2, 1) = T(0, 0);
  // third column
  h_A1(0, 2) = T(0, 0);
  h_A1(1, 2) = T(2, 0);
  h_A1(2, 2) = T(3, 0);
}

template <typename C>
void set_A1_LU_complex(C&& h_A1)
{
  using T = typename C::value_type;

  // first column
  h_A1(0, 0) = T(0, 4);
  h_A1(1, 0) = T(0, -1);
  h_A1(2, 0) = T(0.25, -0.25);
  // second column factored
  h_A1(0, 1) = T(4, 0);
  h_A1(1, 1) = T(0, 10);
  h_A1(2, 1) = T(0, -0.1);
  // third column factored
  h_A1(0, 2) = T(2, 0);
  h_A1(1, 2) = T(4, 2);
  h_A1(2, 2) = T(1.3, 0.9);
}

template <typename T>
void set_A1_piv(T&& h_p)
{
  h_p(0) = 2;
  h_p(1) = 3;
  h_p(2) = 3;
}

template <typename T>
void test_getrs_batch_real()
{
  constexpr int N = 3;
  constexpr int NRHS = 2;
  constexpr int batch_size = 1;

  gt::gtensor<T*, 1> h_Aptr(batch_size);
  gt::gtensor_device<T*, 1> d_Aptr(batch_size);
  gt::gtensor<T, 3> h_A(gt::shape(N, N, batch_size));
  gt::gtensor_device<T, 3> d_A(gt::shape(N, N, batch_size));

  gt::gtensor<T*, 1> h_Bptr(batch_size);
  gt::gtensor_device<T*, 1> d_Bptr(batch_size);
  gt::gtensor<T, 3> h_B(gt::shape(N, NRHS, batch_size));
  gt::gtensor_device<T, 3> d_B(gt::shape(N, NRHS, batch_size));

  gt::gtensor<gt::bandsolver::index_t, 2> h_p(gt::shape(N, batch_size));
  gt::gtensor_device<gt::bandsolver::index_t, 2> d_p(gt::shape(N, batch_size));

  // set up first (and only) batch
  set_A0_LU(h_A.view(gt::all, gt::all, 0));
  h_Aptr(0) = gt::raw_pointer_cast(d_A.data());
  set_A0_piv(h_p.view(gt::all, 0));

  // set up two col vectors to solve in first (and only) batch
  // first RHS, col vector [11; 18; 28]
  h_B(0, 0, 0) = 11;
  h_B(1, 0, 0) = 18;
  h_B(2, 0, 0) = 28;
  // second RHS, col vector [73; 78; 154]
  h_B(0, 1, 0) = 73;
  h_B(1, 1, 0) = 78;
  h_B(2, 1, 0) = 154;
  h_Bptr(0) = gt::raw_pointer_cast(d_B.data());

  gt::copy(h_Aptr, d_Aptr);
  gt::copy(h_A, d_A);
  gt::copy(h_Bptr, d_Bptr);
  gt::copy(h_B, d_B);
  gt::copy(h_p, d_p);

  gt::bandsolver::getrs_banded_batched(
    N, NRHS, gt::raw_pointer_cast(d_Aptr.data()), N,
    gt::raw_pointer_cast(d_p.data()), gt::raw_pointer_cast(d_Bptr.data()), N,
    batch_size, N - 1, N - 1);

  gt::copy(d_B, h_B);

  // solution vector [1; 2; 3]
  EXPECT_EQ(h_B(0, 0, 0), 1.0);
  EXPECT_EQ(h_B(1, 0, 0), 2.0);
  EXPECT_EQ(h_B(2, 0, 0), 3.0);
  // solution vector [-3; 7; 31]
  EXPECT_EQ(h_B(0, 1, 0), -3.0);
  EXPECT_EQ(h_B(1, 1, 0), 7.0);
  EXPECT_EQ(h_B(2, 1, 0), 31.0);
}

TEST(bandsolve, sgetrs_batch)
{
  test_getrs_batch_real<float>();
}

TEST(bandsolve, dgetrs_batch)
{
  test_getrs_batch_real<double>();
}

namespace test
{
// little hack to make tests parameterizable on managed vs device memory

template <typename T, gt::size_type N, typename S = gt::space::device>
struct gthelper
{
  using gtensor = gt::gtensor<T, N, S>;
};

template <typename T, gt::size_type N>
struct gthelper<T, N, gt::space::managed>
{
  using gtensor = gt::gtensor_container<gt::space::managed_vector<T>, N>;
};

template <typename T, gt::size_type N, typename S = gt::space::device>
using gtensor2 = typename gthelper<T, N, S>::gtensor;
} // namespace test

template <typename R, typename S = gt::space::device>
void test_getrs_batch_complex()
{
  constexpr int N = 3;
  constexpr int NRHS = 2;
  constexpr int batch_size = 2;
  using T = gt::complex<R>;

  gt::gtensor<T*, 1> h_Aptr(batch_size);
  test::gtensor2<T*, 1, S> d_Aptr(batch_size);
  gt::gtensor<T, 3> h_A(gt::shape(N, N, batch_size));
  test::gtensor2<T, 3, S> d_A(gt::shape(N, N, batch_size));

  gt::gtensor<T*, 1> h_Bptr(batch_size);
  test::gtensor2<T*, 1, S> d_Bptr(batch_size);
  gt::gtensor<T, 3> h_B(gt::shape(N, NRHS, batch_size));
  test::gtensor2<T, 3, S> d_B(gt::shape(N, NRHS, batch_size));

  gt::gtensor<gt::bandsolver::index_t, 2> h_p(gt::shape(N, batch_size));
  test::gtensor2<gt::bandsolver::index_t, 2, S> d_p(gt::shape(N, batch_size));

  // setup input for first batch
  set_A0_LU(h_A.view(gt::all, gt::all, 0));
  h_Aptr(0) = gt::raw_pointer_cast(d_A.data());
  set_A0_piv(h_p.view(gt::all, 0));

  // setup input for second batch
  set_A1_LU_complex(h_A.view(gt::all, gt::all, 1));
  h_Aptr[1] = h_Aptr(0) + N * N;
  set_A1_piv(h_p.view(gt::all, 1));

  // first batch, first rhs col vector   (11; 18; 28)
  h_B(0, 0, 0) = 11;
  h_B(1, 0, 0) = 18;
  h_B(2, 0, 0) = 28;
  // first batch, second rhs col vector  (73; 78; 154)
  h_B(0, 1, 0) = 73;
  h_B(1, 1, 0) = 78;
  h_B(2, 1, 0) = 154;
  // second batch, first rhs col vector  (73; 78; 154)
  h_B(0, 0, 1) = T(11, -1);
  h_B(1, 0, 1) = T(14, 4);
  h_B(2, 0, 1) = T(16, 12);
  // second batch, second rhs col vector (73-10i; 90-12i; 112 + 42i)
  h_B(0, 1, 1) = T(73, -10);
  h_B(1, 1, 1) = T(90, -12);
  h_B(2, 1, 1) = T(112, 42);

  h_Bptr(0) = gt::raw_pointer_cast(d_B.data());
  h_Bptr(1) = h_Bptr(0) + N * NRHS;

  gt::copy(h_Aptr, d_Aptr);
  gt::copy(h_A, d_A);
  gt::copy(h_Bptr, d_Bptr);
  gt::copy(h_B, d_B);
  gt::copy(h_B, d_B);
  gt::copy(h_p, d_p);

  gt::bandsolver::getrs_banded_batched(
    N, NRHS, gt::raw_pointer_cast(d_Aptr.data()), N,
    gt::raw_pointer_cast(d_p.data()), gt::raw_pointer_cast(d_Bptr.data()), N,
    batch_size, N - 1, N - 1);

  gt::copy(d_B, h_B);

  // first batch, first solution vector [1; 2; 3]
  expect_complex_near(h_B(0, 0, 0), 1.0);
  expect_complex_near(h_B(1, 0, 0), 2.0);
  expect_complex_near(h_B(2, 0, 0), 3.0);
  // first batch, second solution vector [-3; 7; 31]
  expect_complex_near(h_B(0, 1, 0), -3.0);
  expect_complex_near(h_B(1, 1, 0), 7.0);
  expect_complex_near(h_B(2, 1, 0), 31.0);
  // second batch, first solution vector [1; 2; 3]
  expect_complex_near(h_B(0, 0, 1), 1.0);
  expect_complex_near(h_B(1, 0, 1), 2.0);
  expect_complex_near(h_B(2, 0, 1), 3.0);
  // second batch, second solution vector [-3; 7; 31]
  expect_complex_near(h_B(0, 1, 1), -3.0);
  expect_complex_near(h_B(1, 1, 1), 7.0);
  expect_complex_near(h_B(2, 1, 1), 31.0);
}

TEST(bandsolve, cgetrs_batch)
{
  test_getrs_batch_complex<float>();
}

TEST(bandsolve, zgetrs_batch)
{
  test_getrs_batch_complex<double>();
}

TEST(bandsolve, cgetrs_batch_managed)
{
  test_getrs_batch_complex<float, gt::space::managed>();
}

TEST(bandsolve, zgetrs_batch_managed)
{
  test_getrs_batch_complex<double, gt::space::managed>();
}

template <typename T>
void test_get_max_bandwidth()
{
  constexpr int N = 16;
  constexpr int batch_size = 2;

  gt::gtensor<T*, 1> h_Aptr(batch_size);
  gt::gtensor_device<T*, 1> d_Aptr(batch_size);
  auto h_A = gt::zeros<T>(gt::shape(N, N, batch_size));
  auto d_A = gt::zeros_device<T>(h_A.shape());

  h_Aptr(0) = gt::raw_pointer_cast(d_A.data());
  h_Aptr(1) = h_Aptr(0) + N * N;

  // first matrix is diag, lbw=ubw=0
  for (int i = 0; i < N; i++) {
    h_A(i, i, 0) = T(1);
  }
  // second matrix, set nonzero near lower left corner, on upper second diag
  h_A(N - 3, 0, 1) = T(1);
  h_A(0, 1, 1) = T(1);

  gt::copy(h_A, d_A);
  gt::copy(h_Aptr, d_Aptr);

  auto bw0 = gt::bandsolver::get_max_bandwidth(
    N, gt::raw_pointer_cast(d_Aptr.data()), N, 1);
  EXPECT_EQ(bw0.lower, 0);
  EXPECT_EQ(bw0.upper, 0);

  auto bw1 = gt::bandsolver::get_max_bandwidth(
    N, gt::raw_pointer_cast(d_Aptr.data()) + 1, N, 1);
  EXPECT_EQ(bw1.lower, N - 3);
  EXPECT_EQ(bw1.upper, 1);

  auto bw = gt::bandsolver::get_max_bandwidth(
    N, gt::raw_pointer_cast(d_Aptr.data()), N, batch_size);
  EXPECT_EQ(bw.lower, N - 3);
  EXPECT_EQ(bw.upper, 1);
}

TEST(bandsolve, sget_max_bandwidth)
{
  test_get_max_bandwidth<double>();
}

TEST(bandsolve, dget_max_bandwidth)
{
  test_get_max_bandwidth<double>();
}

TEST(bandsolve, cget_max_bandwidth)
{
  test_get_max_bandwidth<gt::complex<double>>();
}

TEST(bandsolve, zget_max_bandwidth)
{
  test_get_max_bandwidth<gt::complex<double>>();
}
