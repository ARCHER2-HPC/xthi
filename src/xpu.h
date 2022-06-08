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

#endif
