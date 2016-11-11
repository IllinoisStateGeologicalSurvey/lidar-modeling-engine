#ifndef POINT_SET_H
#define POINT_SET_H

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <limits.h>
#include <proj_api.h>
#include "hdf5.h"
#include "hdf5_hl.h"
#include <liblas/capi/liblas.h>
#include "point.h"
#include "common.h"
#include "grid.h"

typedef struct LMEpointSet {
	size_t count;
	LMEbound extent;
	LMEcrs crs;
	LMEpointCode* points;
} LMEpointSet;

int LMEpointSet_Create(size_t count, LMEbound extent, LMEcrs crs, LMEpointCode* points, LMEpointSet* pSet);

int LMEpointSet_createDataset(hid_t group_id, LMEpointCode* points, uint32_t n_points);

int LMEpointSet_writeDataset(hid_t file_id, char* dataset, hsize_t* offset, hsize_t* block, LMEpointCode* points, MPI_Comm comm, MPI_Info info);

int LMEpointSet_writeFromLAS(char* LASpath, char* dataset_name, hsize_t* pointBlock, hid_t group_id, MPI_Comm comm, MPI_Info info);


int LMEpointSet_read(hid_t group_id, char* dataset_name, hsize_t pointCount, LMEpointCode* points);


int LMEpointSet_grid(hid_t group_id, hid_t dset_id, hsize_t pointBlock, LMEgrid* grid);

#endif

