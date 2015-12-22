# HDF5 Makefile template
HDF_INSTALL = /sw/hdf5-1.8.15-para
MPI_INSTALL = /sw/EasyBuild/software/MPICH/3.1.4-GCC-4.9.2-binutils-2.25
LIBLAS_INSTALL = /gpfs_scratch/ncasler/liblas
PROJ_INSTALL=/gpfs_scratch/ncasler/proj
#SZIP_INSTALL = /sw/EasyBuild/software/Szip/2.1-gmpolf-2015
EXTLIB      = -L$(HDF_INSTALL)/lib -L$(MPI_INSTALL)/lib -L$(LIBLAS_INSTALL)/lib -L$(PROJ_INSTALL)/lib
CC          = mpicc
CFLAGS      = -g
LIB         =  -lz -lm -lrt -lmpich -lpthread -ldl -llas -llas_c -lproj

INCLUDE     = -I./include 
EXTINCLUDE  = $(INCLUDE) -I$(MPI_INSTALL)/include -I$(HDF_INSTALL)/include -I$(LIBLAS_INSTALL)/include -I$(PROJ_INSTALL)/include
#-I$(SZIP_INSTALL)/include

LIBSHDF     = $(EXTLIB) $(HDF_INSTALL)/lib/libhdf5.a $(MPI_INSTALL)/lib/libmpi.a 
LIBSLAS     = $(EXTLIB) $(LIBLAS_INSTALL)/lib/liblas_c.so $(LIBLAS_INSTALL)/lib/liblas_c.so.3
LIBSPROJ    = $(EXTLIB) $(PROJ_INSTALL)/lib/libproj.a
#$(SZIP_INSTALL)/lib/libsz.a

all: h5_crtdat \
	h5_writechunk \
	readLas \
	testFileUtils

testFileUtils: file-test.c
	$(CC) $(CFLAGS) -o $@ file-test.c file_util.c $(EXTINCLUDE) $(EXTLIB) $(LIB)

h5_crtdat: h5_crtdat.c
	$(CC) $(CFLAGS) -o $@ h5_crtdat.c $(EXTINCLUDE) $(LIBSHDF) $(LIB)

h5_rdwt: h5_rdwt.c
	$(CC) $(CFLAGS) -o $@ h5_rdwt.c $(EXTINCLUDE) $(LIBSHDF) $(LIB)

h5_crtatt: h5_crtatt.c
	$(CC) $(CFLAGS) -o $@ h5_crtatt.c $(EXTINCLUDE) $(LIBSHDF) $(LIB)

h5_writechunk: h5_writechunk.c
	$(CC) $(CFLAGS) -o $@ h5_writechunk.c $(EXTINCLUDE) $(LIBSHDF) $(LIB)

readLas: readLas.c
	$(CC) $(CFLAGS) -o $@ readLas.c common.c point.c hilbert.c file_util.c $(EXTINCLUDE) $(LIBSHDF) $(LIBSLAS) $(LIBSPROJ) $(LIB)


clean: 
	rm -f *.h5 *.o \
		h5_crtdat \
		h5_writechunk \
		readLas \
		common \
		testFileUtils

.SUFFIXES:.o.c
