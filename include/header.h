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
#include "crs.h"

#define PATH_SIZE 1024

typedef struct proj_t {
    char proj4[PATH_SIZE];
} proj_t;

//typedef struct bound_t {
//    coord_t low;
//    coord_t high; 
//} bound_t;


typedef struct LMEheader {
	uint64_t id;
	uint32_t pnt_count;
	LMEboundCode bounds;
	LMEcrs crs;
	char path[PATH_SIZE];
	
} LMEheader;

/**
 * @deprecated: This function has been deprecated in order to utilize table
 * datasets for headers. Please use LMEheaderSet_createDataset.
 */
int createHeaderDataset(char* file, char* dataset, hsize_t* dims);

int LMEheaderSet_createDataset(hid_t group_id, LMEheader* header, uint32_t n_headers);

hid_t HeaderType_create(herr_t* status);

void HeaderType_destroy(hid_t headertype, herr_t* status);


int LMEheaderBlock_read(char paths[], int offset, int block, LMEheader* headers);

int LMEheaderBlock_writeSer(hid_t file_id, char* dataset, hsize_t* offset, hsize_t* block, LMEheader* headers);

int LMEheaderBlock_write(hid_t file_id, char* dataset, hsize_t* offset, hsize_t* block, LMEheader* headers, MPI_Comm comm, MPI_Info);

int LMEheader_read(char* path, LMEheader* header, uint32_t id);

#endif
