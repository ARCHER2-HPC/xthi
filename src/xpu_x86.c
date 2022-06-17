/*****************************************************************************
 *
 *  xpu.c
 *
 *****************************************************************************/

#include "xpu.h"

xpu_err_t xpuGetDeviceCount(int * ndevice) {

  *ndevice = 0;

  return 0;
}

xpu_err_t xpuGetDevice(int * device) {

  *device = -1;

  return -1;
}

xpu_err_t xpuDeviceGetPCIBusId(char * pciBusId, int len, int device) {

  /* No op. */

  return 0;
}
