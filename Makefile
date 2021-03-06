###############################################################
# Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  #
# Information Laboratory (CIGI). All Rights Reserved.		  #
#															  #
###############################################################
# HDF5 Makefile template
HDF_INSTALL = /sw/hdf5-1.8.15-para
MPI_INSTALL = /sw/EasyBuild/software/MPICH/3.1.4-GCC-4.9.2-binutils-2.25
GEOS_INSTALL = /sw/geosoft/geos
LIBLAS_INSTALL = /gpfs_scratch/ncasler/liblas
PROJ_INSTALL=/gpfs_scratch/ncasler/proj
GSL_INSTALL=/sw/gsl/2.1
#SZIP_INSTALL = /sw/EasyBuild/software/Szip/2.1-gmpolf-2015
EXTLIB		= -L$(HDF_INSTALL)/lib -L$(MPI_INSTALL)/lib -L$(LIBLAS_INSTALL)/lib -L$(PROJ_INSTALL)/lib -L$(GSL_INSTALL)/lib
CC			= mpicc
CFLAGS		= -g -Wall
LIB			=  -lz -lm -lrt -lmpich -lpthread -ldl -llas -llas_c -lproj -lgsl -lgslcblas 

SOURCEDIR	= src
BUILDDIR	= bin
OBJDIR		= obj
TESTDIR		= test
INCLUDE		= -I./include 
GEOS_INCLUDE = $(GEOS_INSTALL)/include
EXTINCLUDE	= $(INCLUDE) -I$(MPI_INSTALL)/include -I$(HDF_INSTALL)/include -I$(LIBLAS_INSTALL)/include -I$(GEOS_INCLUDE) -I$(PROJ_INSTALL)/include -I$(GSL_INSTALL)/include
#-I$(SZIP_INSTALL)/include
LIBGEOS		=  $(GEOS_INSTALL)/lib/libgeos_c.so
LIBSHDF		=  $(HDF_INSTALL)/lib/libhdf5_hl.a $(HDF_INSTALL)/lib/libhdf5.a
LIBSMPI		=  $(MPI_INSTALL)/lib/libmpi.a 
LIBSLAS		=  $(LIBLAS_INSTALL)/lib/liblas_c.so $(LIBLAS_INSTALL)/lib/liblas_c.so.3
LIBSPROJ	=  $(PROJ_INSTALL)/lib/libproj.a
#$(SZIP_INSTALL)/lib/libsz.a
EXTLIBS		= $(LIBSHDF) $(LIBGEOS) $(LIBSLAS) $(LIBSPROJ) $(LIBSMPI) $(EXTLIB) $(LIB)

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

all: $(BUILDDIR)/initLME \
	$(BUILDDIR)/addRegion \
	$(BUILDDIR)/quadTest \
	$(BUILDDIR)/addRegionPar \
	$(BUILDDIR)/readPoints \
	$(BUILDDIR)/gridPoints \
	$(BUILDDIR)/headerRead \
	$(BUILDDIR)/headerTest \
	$(BUILDDIR)/testRange \
	$(BUILDDIR)/checkCatalog \
	$(BUILDDIR)/getGrid \
	$(BUILDDIR)/hilbertTest \
	$(BUILDDIR)/fileCheck \

$(BUILDDIR)/hilbertTest: $(TESTDIR)/hilbert-test.c
	$(CC) $(CFlAGS) -o $@ $(TESTDIR)/hilbert-test.c $(SOURCES) $(EXTINCLUDE) $(EXTLIBS)

$(BUILDDIR)/fileCheck: $(TESTDIR)/test-file.c
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/test-file.c $(SOURCES) $(EXTINCLUDE) $(EXTLIBS)
	
$(BUILDDIR)/gridPoints: $(TESTDIR)/gridPoints.c
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/gridPoints.c $(SOURCES) $(EXTINCLUDE) $(EXTLIBS)

$(BUILDDIR)/quadTest: $(TESTDIR)/quadTest.c
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/quadTest.c $(SOURCES) $(EXTINCLUDE) $(EXTLIBS)

$(BUILDDIR)/getGrid: $(TESTDIR)/getGrid.c
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/getGrid.c $(SOURCES) $(EXTINCLUDE) $(EXTLIBS)

$(BUILDDIR)/checkCatalog: $(TESTDIR)/checkCatalog.c
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/checkCatalog.c $(SOURCES) $(EXTINCLUDE) $(EXTLIBS)

$(BUILDDIR)/initLME: $(TESTDIR)/init.c
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/init.c $(SOURCES) $(EXTINCLUDE) $(EXTLIBS)

$(BUILDDIR)/addRegion: $(TESTDIR)/addRegion.c
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/addRegion.c $(SOURCES) $(EXTINCLUDE) $(EXTLIBS)

$(BUILDDIR)/addRegionPar: $(TESTDIR)/addRegionPar.c
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/addRegionPar.c $(SOURCES) $(EXTINCLUDE) $(EXTLIBS)

$(BUILDDIR)/readPoints: $(TESTDIR)/readPoints.c
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/readPoints.c $(SOURCES) $(EXTINCLUDE) $(EXTLIBS)

#$(BUILDDIR)/readLas: $(TESTDIR)/readLas.c
#	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/readLas.c $(SOURCES) $(EXTINCLUDE) $(EXTLIBS)
$(BUILDDIR)/headerTest: $(TESTDIR)/test-headers.c
	$(CC) $(CFLAGS) -o $@ $(TESTDIR)/test-headers.c $(SOURCES) $(EXTINCLUDE) $(EXTLIBS)

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
