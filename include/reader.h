/** 
  reader.h :
  This file will hold functions related to reading points from LAS or HDF files.

*/
#ifndef READER_H
#define READER_H
#include <mpi.h>
#include <hdf5.h>
#include "header.h"
#include "point.h"
#include "filter.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <proj_api.h>

/* Returns the total of existing headers */
hsize_t Headers_count(hid_t file_id);
/* Reads all available headers */
int Headers_read(LMEheader* headers, hid_t file_id);

int HeaderSet_read(int start, int numHeaders, LMEheader* headers, char* filename);

int HeaderLMEpoint_get(LMEheader* header, char* dset_name);

int LMEpointSet_create(LMEheader* header, hid_t group_id);

int LMEpointSet_prepare(LMEheader* header, char* LASpath, hsize_t* pointBlock);


int LMEpointSet_write(hid_t group_id, char* dataset_name, hsize_t* offset, hsize_t* block, LMEpointCode* points, MPI_Comm comm, MPI_Info info);

int LMEpointSet_copy(char* LASpath, char* dataset_name, hsize_t* pointBlock, hid_t group_id, MPI_Comm comm, MPI_Info info);

int LASFile_read(LASReaderH reader, hsize_t* offset, hsize_t* count, LMEpointCode* points, int mpi_rank);

int checkOrphans(hid_t file_id, int mpi_rank);

int openLAS(LASReaderH* reader, LASHeaderH* header, LASSRSH* srs, uint32_t* pntCount, char* path);

int filterLAS(LASHeaderH* header, LASReaderH* reader, uint32_t* pntCount, LMEfilter* filter, LMEpointCode* points, int mpi_rank);

int closeLAS(LASReaderH* reader, LASHeaderH* header, LASSRSH* srs, uint32_t* pntCount, LMEpointCode* points);

#endif //READER_H
