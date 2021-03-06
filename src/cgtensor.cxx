#include <stdint.h>

#include <gtensor/capi.h>
#include <gtensor/gtensor.h>

void gt_synchronize()
{
  gt::synchronize();
}

#ifdef GTENSOR_HAVE_DEVICE

#ifndef GTENSOR_DEVICE_SYCL

int gt_backend_device_get_count()
{
  return gt::backend::device_get_count();
}

void gt_backend_device_set(int device_id)
{
  return gt::backend::device_set(device_id);
}

int gt_backend_device_get()
{
  return gt::backend::device_get();
}

uint32_t gt_backend_device_get_vendor_id(int device_id)
{
  return gt::backend::device_get_vendor_id(device_id);
}

#endif // not GTENSOR_DEVICE_SYCL

void* gt_backend_host_allocate(size_t nbytes)
{
  return (void*)gt::backend::host_allocator<uint8_t>::allocate(nbytes);
}

void* gt_backend_device_allocate(size_t nbytes)
{
  return (void*)gt::backend::device_allocator<uint8_t>::allocate(nbytes);
}

void* gt_backend_managed_allocate(size_t nbytes)
{
  return (void*)gt::backend::managed_allocator<uint8_t>::allocate(nbytes);
}

void gt_backend_host_deallocate(void* p)
{
  gt::backend::host_allocator<uint8_t>::deallocate((uint8_t*)p);
}

void gt_backend_device_deallocate(void* p)
{
  gt::backend::device_allocator<uint8_t>::deallocate((uint8_t*)p);
}

void gt_backend_managed_deallocate(void* p)
{
  gt::backend::managed_allocator<uint8_t>::deallocate((uint8_t*)p);
}

void gt_backend_memcpy_hh(void* dst, const void* src, size_t nbytes)
{
  gt::backend::device_copy_hh<uint8_t>((uint8_t*)src, (uint8_t*)dst, nbytes);
}

void gt_backend_memcpy_dd(void* dst, const void* src, size_t nbytes)
{
  gt::backend::device_copy_dd<uint8_t>((uint8_t*)src, (uint8_t*)dst, nbytes);
}

void gt_backend_memcpy_async_dd(void* dst, const void* src, size_t nbytes)
{
  gt::backend::device_copy_async_dd<uint8_t>((uint8_t*)src, (uint8_t*)dst,
                                             nbytes);
}

void gt_backend_memcpy_dh(void* dst, const void* src, size_t nbytes)
{
  gt::backend::device_copy_dh<uint8_t>((uint8_t*)src, (uint8_t*)dst, nbytes);
}

void gt_backend_memcpy_hd(void* dst, const void* src, size_t nbytes)
{
  gt::backend::device_copy_hd<uint8_t>((uint8_t*)src, (uint8_t*)dst, nbytes);
}

void gt_backend_memset(void* dst, int value, size_t nbytes)
{
  gt::backend::device_memset(dst, value, nbytes);
}

#endif
