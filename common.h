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

projPJ loadProj(const char* proj4string);

projPJ getLASProj(LASHeaderH header);

int getPointCount(LASHeaderH header);

int project(projPJ pj_src, projPJ pj_dst, double x, double y, double z);

int createDataset(char* file, char* dataset, hsize_t dims[2]);

int readBlock(LASReaderH reader, int offset, int count, Point* points);

int writeBlock(char* file, char* dataset, hsize_t offset[2], hsize_t block[2], float* data);

#endif
