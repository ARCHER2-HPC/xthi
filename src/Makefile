# Compile for either gnu or cce

default:
	$(MAKE) xthi xthi_mp

# Build both with and without OpenMP

xthi:
	$(CC) -fPIC -Wall -c -o xthi.o xthi.c
	$(AR) -crs libxthi_mpi.a xthi.o
	$(CC) -shared -o libxthi_mpi.so xthi.o
	$(CC) -o xthi_mpi xthi.o main.c

xthi_mp:
	$(CC) -fPIC -fopenmp -c -o xthi.o xthi.c
	$(AR) -crs libxthi_mpi_mp.a xthi.o
	$(CC) -fopenmp -shared -o libxthi_mpi_mp.so xthi.o
	$(CC) -fopenmp -o xthi_mpi_mp xthi.o main.c

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
