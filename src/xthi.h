/* Interface
 *
 * xthi_print argumemts:
 *  comm - MPI communicator in which to operate (may be MPI_COMM_WORLD)
 *  argc - number of command line arguments
 *  argv - command line  arguments
 *  fp   - valid file pointer to which output will be directed
 *
 *  returns 0 on success, non-zero if an error occured.
 */

#ifndef EPCC_XTHI_INCLUDE
#define EPCC_XTHI_INCLUDE

#include <stdio.h>
#include "mpi.h"

int xthi_print(MPI_Comm comm, int argc, char ** argv, FILE * fp);

#endif
