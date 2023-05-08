# Compile for either gnu or cce

default:
	$(MAKE) xthi xthi_mp

CC = cc
MACHINE = gpu_null

# E.g.
#CFLAGS_EXTRA = -x hip --offload-arch=gfx90a
#MACHINE = gpu_amd8

# Build both with and without OpenMP

xthi:
	$(CC) $(CFLAGS_EXTRA) -fPIC -c -o xthi.o xthi.c
	$(CC) $(CFLAGS_EXTRA) -fPIC -c $(MACHINE).c
	$(CC) $(CFLAGS_EXTRA) -fPIC -c main.c
	$(AR) -crs libxthi_mpi.a $(MACHINE).o xthi.o
	$(CC) -shared -o libxthi_mpi.so $(MACHINE).o xthi.o
	$(CC) -o xthi_mpi $(MACHINE).o xthi.o main.o

xthi_mp:
	$(CC) $(CFLAGS_EXTRA) -fPIC -fopenmp -c -o xthi.o xthi.c
	$(CC) $(CFLAGS_EXTRA) -fPIC -c $(MACHINE).c
	$(CC) $(CFLAGS_EXTRA) -fPIC -c main.c
	$(AR) -crs libxthi_mpi_mp.a $(MACHINE).o xthi.o
	$(CC) -fopenmp -shared -o libxthi_mpi_mp.so $(MACHINE).o xthi.o
	$(CC) -fopenmp -o xthi_mpi_mp $(MACHINE).o xthi.o main.o

install:
	echo "Install location: ${PREFIX}" 
	mkdir -p ${PREFIX}/bin
	mkdir -p ${PREFIX}/include
	mkdir -p ${PREFIX}/lib
	cp xthi.h ${PREFIX}/include
	cp xthi_mpi ${PREFIX}/bin
	cp xthi_mpi_mp ${PREFIX}/bin
	cp libxthi_mpi.* ${PREFIX}/lib
	cp libxthi_mpi_mp.* ${PREFIX}/lib
	cd ${PREFIX}/bin; ln -s ./xthi_mpi_mp xthi

clean:
	$(RM) libxthi_mpi_mp.* libxthi_mpi.* *.o xthi xthi_mpi xthi_mpi_mp
