# xthi
`xthi` is an executable program which shows process/thread placement when run using MPI/OpenMP.

## Usage

For example, using `srun` as parallel launch
```
export OMP_NUM_THREADS=1
srun ./xthi [options]
```


```
Options

-c 1|2|3    report on Cartesian communicator with geiven number of dimensions
-r          use "reorder" true in MPI_Create_create() (when using -c)
-s seconds  wait for given number of seconds before exiting with MPI_Finalize().
            This can be useful to prevent immediate exit of the application.
-t          report on thread 0 only; default is to report placement for all threads.
```
