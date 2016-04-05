#ifndef HEADER_H
#define HEADER_H

#include <point.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <hdf5.h>
#include <mpi.h>
#include <liblas/capi/liblas.h>
#include <proj_api.h>
#include "file_util.h"
#include "bound.h"

typedef struct proj_t {
    char proj4[4096];
} proj_t;

//typedef struct bound_t {
//    coord_t low;
//    coord_t high; 
//} bound_t;


typedef struct header_t {
    uint32_t id;
    uint32_t pnt_count;
    bound_t bounds;
    char path[4096];
    proj_t proj;
} header_t;

int createHeaderDataset(char* file, char* dataset, hsize_t* dims);

hid_t ProjType_create(herr_t* status);

void ProjType_destroy(hid_t projtype, herr_t* status);

int Proj_Set(LASHeaderH header, proj_t* proj);


//hid_t BoundType_create(herr_t* status);

//void BoundType_destroy(hid_t boundtype, herr_t* status);

//int Bound_Set(LASHeaderH header, bound_t* bounds);

hid_t HeaderType_create(herr_t* status);

void HeaderType_destroy(hid_t headertype, herr_t* status);

hid_t PointType_create(herr_t* status);

void PointType_destroy(hid_t pointtype, herr_t* status);

int MPI_CoordType_create(MPI_Datatype* mpi_coordtype);

//int MPI_BoundType_create(MPI_Datatype* mpi_boundtype);

int MPI_ProjType_create(MPI_Datatype* mpi_projtype);

int MPI_HeaderType_create(MPI_Datatype* mpi_headertype);

int readHeaderBlock(char paths[], int offset, int block, header_t* headers);

int writeHeaderBlock_ser(hid_t file_id, char* dataset, hsize_t* offset, hsize_t* block, header_t* headers);

int writeHeaderBlock(hid_t file_id, char* dataset, hsize_t* offset, hsize_t* block, header_t* headers, MPI_Comm comm, MPI_Info);

int Header_read(char* path, header_t* header, uint32_t id);

#endif
