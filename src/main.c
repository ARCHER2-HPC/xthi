/* Interrogation of task placement for mixed MPI/OpenMP architectures */
/* Compile with options appropriate for MPI/OpenMP e.g.,
 *
 * Allows for differing numbers of MPI tasks and OpenMP threads
 * on different nodes.
 */

#include <stdio.h>

#include "mpi.h"
#include "xthi.h"

int main(int argc, char ** argv) {

  MPI_Init(&argc, &argv);

  xthi_print(MPI_COMM_WORLD, argc, argv, stdout);

  MPI_Finalize();

  return 0;
}
