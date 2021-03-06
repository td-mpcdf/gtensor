#include "gtensor/gtensor.h"

#ifdef GTENSOR_DEVICE_CUDA

#include <cufft.h>
typedef cufftHandle gpufft_handle_t;
typedef cudaStream_t gpufft_stream_t;

typedef cufftType gpufft_transform_t;
#define GPUFFT_Z2D CUFFT_Z2D
#define GPUFFT_D2Z CUFFT_D2Z
#define GPUFFT_C2R CUFFT_C2R
#define GPUFFT_R2C CUFFT_R2C
#define GPUFFT_Z2Z CUFFT_Z2Z
#define GPUFFT_C2C CUFFT_C2C

#define GPUFFT_FORWARD CUFFT_FORWARD
#define GPUFFT_INVERSE CUFFT_INVERSE
#define GPUFFT_BACKWARD CUFFT_INVERSE

typedef cufftDoubleReal gpufft_double_real_t;
typedef cufftReal gpufft_real_t;
typedef cufftDoubleComplex gpufft_double_complex_t;
typedef cufftComplex gpufft_complex_t;

#elif defined(GTENSOR_DEVICE_HIP)

#include "hipfft.h"
typedef hipfftHandle gpufft_handle_t;
typedef hipStream_t gpufft_stream_t;

typedef hipfftType gpufft_transform_t;
#define GPUFFT_Z2D HIPFFT_Z2D
#define GPUFFT_D2Z HIPFFT_D2Z
#define GPUFFT_C2R HIPFFT_C2R
#define GPUFFT_R2C HIPFFT_R2C
#define GPUFFT_Z2Z HIPFFT_Z2Z
#define GPUFFT_C2C HIPFFT_C2C

#define GPUFFT_FORWARD HIPFFT_FORWARD
#define GPUFFT_BACKWARD HIPFFT_BACKWARD
#define GPUFFT_INVERSE HIPFFT_BACKWARD

typedef hipfftDoubleReal gpufft_double_real_t;
typedef hipfftReal gpufft_real_t;
typedef hipfftDoubleComplex gpufft_double_complex_t;
typedef hipfftComplex gpufft_complex_t;

#elif defined(GTENSOR_DEVICE_SYCL)

#include "CL/sycl.hpp"
#include <vector>
//#include "oneapi/mkl.hpp"
#include "mkl.h"
#include "oneapi/mkl/dfti.hpp"

// magic numbers taken from hipfft.h. Most likely identical to CUDA values.
typedef enum gpufft_transform_enum
{
  GPUFFT_R2C = 0x2a, // Real to complex (interleaved)
  GPUFFT_C2R = 0x2c, // Complex (interleaved) to real
  GPUFFT_C2C = 0x29, // Complex to complex (interleaved)
  GPUFFT_D2Z = 0x6a, // Double to double-complex (interleaved)
  GPUFFT_Z2D = 0x6c, // Double-complex (interleaved) to double
  GPUFFT_Z2Z = 0x69  // Double-complex to double-complex (interleaved)
} gpufft_transform_t;

#define GPUFFT_FORWARD -1
#define GPUFFT_INVERSE 1
#define GPUFFT_BACKWARD GPUFFT_INVERSE

typedef oneapi::mkl::dft::descriptor<oneapi::mkl::dft::precision::DOUBLE,
                                     oneapi::mkl::dft::domain::REAL>
  gpufft_real_double_descriptor_t;
typedef oneapi::mkl::dft::descriptor<oneapi::mkl::dft::precision::SINGLE,
                                     oneapi::mkl::dft::domain::REAL>
  gpufft_real_single_descriptor_t;

typedef oneapi::mkl::dft::descriptor<oneapi::mkl::dft::precision::DOUBLE,
                                     oneapi::mkl::dft::domain::COMPLEX>
  gpufft_complex_double_descriptor_t;
typedef oneapi::mkl::dft::descriptor<oneapi::mkl::dft::precision::SINGLE,
                                     oneapi::mkl::dft::domain::COMPLEX>
  gpufft_complex_single_descriptor_t;

// Note: the handle type for SYCL MKL implementation is recoverable type
// erasure - the transform type can be used to infer the type which is
// needed inside the destroy routine to delete it. The exec routines imply
// the transform type, so they don't need to examine the transform type.
// TODO: is there a more elegant way to implement this?
typedef struct
{
  void* descriptor_p;
  gpufft_transform_t type;
} gpufft_mkl_handle_t;

typedef gpufft_mkl_handle_t* gpufft_handle_t;

typedef cl::sycl::queue* gpufft_stream_t;

typedef double gpufft_double_real_t;
typedef float gpufft_real_t;
typedef gt::complex<double> gpufft_double_complex_t;
typedef gt::complex<float> gpufft_complex_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void gpufft_plan_many(gpufft_handle_t* handle, int rank, int* n, int istride,
                      int idist, int ostride, int odist,
                      gpufft_transform_t type, int batchSize);

void gpufft_plan_destroy(gpufft_handle_t handle);

void gpufft_exec_z2d(gpufft_handle_t handle, gpufft_double_complex_t* indata,
                     gpufft_double_real_t* outdata);

void gpufft_exec_d2z(gpufft_handle_t handle, gpufft_double_real_t* indata,
                     gpufft_double_complex_t* outdata);

void gpufft_exec_c2r(gpufft_handle_t handle, gpufft_complex_t* indata,
                     gpufft_real_t* outdata);

void gpufft_exec_r2c(gpufft_handle_t handle, gpufft_real_t* indata,
                     gpufft_complex_t* outdata);

void gpufft_exec_z2z(gpufft_handle_t handle, gpufft_double_complex_t* indata,
                     gpufft_double_complex_t* outdata, int direction);

void gpufft_exec_c2c(gpufft_handle_t handle, gpufft_complex_t* indata,
                     gpufft_complex_t* outdata, int direction);

#ifdef __cplusplus
}
#endif
