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
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <proj_api.h>
/* Returns the total of existing headers */
hsize_t Headers_count(hid_t file_id);
/* Reads all available headers */
int Headers_read(header_t* headers, hid_t file_id);

int HeaderSet_read(int start, int numHeaders, header_t* headers, char* filename);

int HeaderPoint_get(header_t* header, char* dset_name);

int PointSet_create(header_t* header, hid_t group_id);

int PointSet_prepare(header_t* header, char* LASpath, hsize_t* pointBlock);

int PointSet_write(hid_t group_id, char* dataset_name, hsize_t* offset, hsize_t* block, Point* points, MPI_Comm comm, MPI_Info info);

int PointSet_copy(char* LASpath, char* dataset_name, hsize_t* pointBlock, hid_t group_id, MPI_Comm comm, MPI_Info info);

int LASFile_read(LASReaderH reader, hsize_t* offset, hsize_t* count, Point* points, int mpi_rank);

int checkOrphans(hid_t file_id, int mpi_rank);
#endif //READER_H
