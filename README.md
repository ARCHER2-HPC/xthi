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
-d          report start and end time/date
-g          report sched_getcpu() as "cpu" (as well as cpu set as "affinity")
-r          use "reorder" true in MPI_Create_create() (when using -c)
-s seconds  wait for given number of seconds before exiting with MPI_Finalize().
            This can be useful to prevent immediate exit of the application.
-t          report on thread 0 only; default is to report placement for all threads.
```

If the option `-c` is specified, the Cartesian information is appended as
```
Node    0, rank    0, thread   0, (affinity =    0)  Cartesian    0 ( 0  0  0)
Node    0, rank    1, thread   0, (affinity =    1)  Cartesian    1 ( 0  0  1)
Node    0, rank    2, thread   0, (affinity =    2)  Cartesian    4 ( 0  1  0)
Node    0, rank    3, thread   0, (affinity =    3)  Cartesian    5 ( 0  1  1)
```
where the first column after `Cartesian` is the rank in the Cartesian
communicator, and the Cartesian co-ordinates follow in brackets. The
Cartesian rank may or may not be the same as the rank in `MPI_COMM_WORLD`
(reported in the fourth column).
