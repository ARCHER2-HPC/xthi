# xthi
`xthi` is an executable program which shows physical process/thread placement when run using MPI/OpenMP.

## Compilation

Compilation on ARCHER2 (all programming environments) is via the Makefile in the `src` directory
```
cd src
make
```
This compiles versions both with (`xthi_mpi_mp`) and without (`xthi_mpi`) OpenMP.

## Usage

For example, using `srun` as parallel launch
```
export OMP_NUM_THREADS=1
srun ./xthi [options]
```

A summary  of nodes is printed, followed by a summary of MPI  task/thread placement. As we are interested in physical placement, it is convenient to print the latter in node order. Node 0 is defined by the placement of MPI rank 0 in the communicator provided (`MPI_COMM_WORLD` for the `xthi` application), but otherwise no assumptions about order are made.


```
Options

-c 1|2|3    report on Cartesian communicator with given number of dimensions
-r          use "reorder" true in MPI_Create_create() (when using -c)
-s seconds  wait for given number of seconds before exiting with MPI_Finalize().
            This can be useful to prevent immediate exit of the application.
-t          report on thread 0 only; default is to report placement for all threads.
```
