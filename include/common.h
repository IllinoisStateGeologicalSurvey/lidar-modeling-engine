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

//int createHeaderDataset(char* file, char* dataset, hsize_t dims[2]);

int readBlock(LASReaderH reader, int offset, int count, Point* points);

int writeBlock(char* file, char* dataset, hsize_t offset[2], hsize_t block[2], Point* points, MPI_Comm comm, MPI_Info info);

// Type creation functions
hid_t CoordType_create(herr_t status);

void CoordType_destroy(hid_t coordtype, herr_t status);

hid_t ReturnType_create(herr_t status);

void ReturnType_destroy(hid_t returntype, herr_t status);

hid_t ColorType_create(herr_t status);

void ColorType_destroy(hid_t colortype, herr_t status);

hid_t PointType_create(herr_t status);

void PointType_destroy(hid_t pointtype, herr_t status);

int MPI_PointType_create(MPI_Datatype *pointtype, Point* point);


#endif
