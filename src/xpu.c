/*****************************************************************************
 *
 *  xpu.c
 *
 *****************************************************************************/

#include "xpu.h"

#ifdef __HIPCC__

#include <hip/hip_runtime.h>

xpu_err_t xpuGetDeviceCount(int * ndevice) {

  hipGetDevice_count(ndevice);

  return 0;
}

xpu_err_t xpuGetDevice(int * device) {

  hipGetDevice(device);

  return 0;
}

#else

xpu_err_t xpuGetDeviceCount(int * ndevice) {

  *ndevice = 0;

  return 0;
}

xpu_err_t xpuGetDevice(int * device) {

  *device = -1;

  return -1;
}

#endif
