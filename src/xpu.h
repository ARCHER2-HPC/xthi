/*****************************************************************************
 *
 *  xpu.h
 *
 *  Shim layer for xpu/gpu.
 *
 *****************************************************************************/

#ifndef XPU_H
#define XPU_H

typedef int xpu_err_t;

xpu_err_t xpuGetDeviceCount(int * ndevice);
xpu_err_t xpuGetDevice(int * device);
xpu_err_t xpuDeviceGetPCIBusId(char * pciBusId, int len, int device);

#endif
