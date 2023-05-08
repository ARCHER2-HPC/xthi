/* No GPU */

#include "gpu.h"
#include "xpu_x86.c"   /* Include source here */

int gpu_per_node(void) {

  return 0;
}

int gpu_node_id(int device) {

  return -1;
}
