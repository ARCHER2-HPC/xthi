
/* GPU node id etc */
/* AMD MI250X 8 per node */

#include <assert.h>
#include <string.h>

#include "gpu.h"
#include "xpu_hip.c"  /* Just include this to ease the Makefile ... */

int gpu_per_node(void) {
  return 8;
}

int gpu_node_id(int device) {

  /* There's no direct way to do this. */
  /* This is just the way the PCI addresses happen to map */

  int gpu_id = -1;
  char pci[3] = {};
  
  {
    char busid[32] = {};
    xpuDeviceGetPCIBusId(busid, 32, device);
    memcpy(pci, busid + 5, 2);
    pci[2] = '\0';
  }

  if (strncmp(pci, "c1", 2) == 0) gpu_id = 0;
  if (strncmp(pci, "c6", 2) == 0) gpu_id = 1;
  if (strncmp(pci, "c9", 2) == 0) gpu_id = 2;
  if (strncmp(pci, "ce", 2) == 0) gpu_id = 3;
  if (strncmp(pci, "d1", 2) == 0) gpu_id = 4;
  if (strncmp(pci, "d6", 2) == 0) gpu_id = 5;
  if (strncmp(pci, "d9", 2) == 0) gpu_id = 6;
  if (strncmp(pci, "de", 2) == 0) gpu_id = 7;

  assert(gpu_id != -1);
  return gpu_id;
}

