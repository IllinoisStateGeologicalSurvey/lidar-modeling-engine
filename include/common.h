#ifndef COMMON_H
#define COMMON_H

#include "mpi.h"
#include "hdf5.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <liblas/capi/liblas.h>
#include <proj_api.h>
#include "point.h"


int Proj_load(const char* proj4string, projPJ* proj);

int LASProj_get(LASHeaderH* header, projPJ* proj);

int getPointCount(LASHeaderH header);

int project(projPJ pj_src, projPJ pj_dst, double x, double y, double z);

int createDataset(char* file, char* dataset, hsize_t* dim);

//int createHeaderDataset(char* file, char* dataset, hsize_t dims[2]);

int readBlock(LASReaderH reader, int offset, int count, LMEpoint* points);

int writeBlock(char* file, char* dataset, hsize_t* offset, hsize_t* block, LMEpoint* points, MPI_Comm comm, MPI_Info info);

// Type creation functions



#endif
