/* GPU local details */

#ifndef GPU_LOCAL_H
#define GPU_LOCAL_H

#include "xpu.h"

int gpu_per_node(void);         /* Number of GPU device per node */
int gpu_node_id(int device);    /* Logical id of this GPU device in node */

#endif
