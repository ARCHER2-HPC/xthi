/*****************************************************************************
 *
 *  xpu.c for HIP
 *
 *****************************************************************************/

#include "xpu.h"

#include <hip/hip_runtime.h>

xpu_err_t xpuGetDeviceCount(int * ndevice) {

  hipGetDeviceCount(ndevice);

  return 0;
}

xpu_err_t xpuGetDevice(int * device) {

  hipGetDevice(device);

  return 0;
}

xpu_err_t xpuDeviceGetPCIBusId(char * pciBusId, int len, int device) {

  hipDeviceGetPCIBusId(pciBusId, len, device);

  return 0;
}
