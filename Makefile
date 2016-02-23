# HDF5 Makefile template
HDF_INSTALL = /sw/hdf5-1.8.15-para
MPI_INSTALL = /sw/EasyBuild/software/MPICH/3.1.4-GCC-4.9.2-binutils-2.25
LIBLAS_INSTALL = /gpfs_scratch/ncasler/liblas
PROJ_INSTALL=/gpfs_scratch/ncasler/proj
#SZIP_INSTALL = /sw/EasyBuild/software/Szip/2.1-gmpolf-2015
EXTLIB		= -L$(HDF_INSTALL)/lib -L$(MPI_INSTALL)/lib -L$(LIBLAS_INSTALL)/lib -L$(PROJ_INSTALL)/lib
CC			= mpicc
CFLAGS		= -g -Wall
LIB			=  -lz -lm -lrt -lmpich -lpthread -ldl -llas -llas_c -lproj

SOURCEDIR	= src
BUILDDIR	= bin
OBJDIR		= obj
TESTDIR		= test
INCLUDE		= -I./include 
EXTINCLUDE	= $(INCLUDE) -I$(MPI_INSTALL)/include -I$(HDF_INSTALL)/include -I$(LIBLAS_INSTALL)/include -I$(PROJ_INSTALL)/include
#-I$(SZIP_INSTALL)/include

LIBSHDF		=  $(HDF_INSTALL)/lib/libhdf5.a $(MPI_INSTALL)/lib/libmpi.a 
LIBSLAS		=  $(LIBLAS_INSTALL)/lib/liblas_c.so $(LIBLAS_INSTALL)/lib/liblas_c.so.3
LIBSPROJ	=  $(PROJ_INSTALL)/lib/libproj.a
#$(SZIP_INSTALL)/lib/libsz.a
EXTLIBS		= $(LIBSHDF) $(LIBSLAS) $(LIBSPROJ) $(EXTLIB) $(LIB)

SOURCES		  := $(wildcard $(SOURCEDIR)/*.c)
OBJECTS		  := $(subst $(SOURCEDIR),$(BUILDDIR),$(SOURCES:%.c=%.o))


# TARGETS
#compile: $(OBJECTS)

# MAIN Target 
#$(EXEC): $(OBJECTS)
#	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTINCLUDE) $(EXTLIBS) $< -o $@

#$(BINARY): $(OBJECTS)
#	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTINCLUDE) $(EXTLIBS) -o $(BINARY)

#$(BUILDDIR)/%.o: %.c
#	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTINCLUDE) $(EXTLIBS) -c $< -o $@

#test: $(SRCS)

all: $(BUILDDIR)/testFileUtils \
	$(BUILDDIR)/readLas \
	$(BUILDDIR)/headerRead \
	$(BUILDDIR)/testRange
	

$(BUILDDIR)/testFileUtils: $(TESTDIR)/file-test.c
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/file-test.c $(SOURCES) $(EXTINCLUDE) $(EXTLIBS)

$(BUILDDIR)/readLas: $(TESTDIR)/readLas.c
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/readLas.c $(SOURCES) $(EXTINCLUDE) $(EXTLIBS)

$(BUILDDIR)/headerRead: $(TESTDIR)/headerRead.c
	$(CC) $(CFLAGS) -o $@  $(TESTDIR)/headerRead.c $(SOURCES) $(EXTINCLUDE) $(EXTLIBS)

$(BUILDDIR)/testRange: $(TESTDIR)/range-test.c
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/range-test.c $(SOURCES) $(EXTINCLUDE) $(EXTLIBS)

clean: 
	rm -f bin/* \
	log/* \
	*.h5 \
	*.o 

#.SUFFIXES:.o.c
