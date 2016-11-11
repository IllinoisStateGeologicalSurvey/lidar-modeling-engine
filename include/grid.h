#ifndef GRID_H
#define GRID_H

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <hdf5.h>
#include <gsl/gsl_matrix.h>
#include <inttypes.h>

typedef struct LMEgrid {
	LMEbound *extent;
	double res[2];
	int dims[2];
	double nodata;
	gsl_matrix* data;
} LMEgrid;

LMEgrid* Grid_Create(LMEbound *extent, int dims[], double res[]);

int LMEgrid_createDataset(hid_t group_id, LMEgrid* grid);

int LMEgrid_write(hid_t group_id, int colStart, int rowStart, int colCount, int rowCount, LMEgrid* grid);

int generateGridDataset(char* file, char* dataset_name, hsize_t* cols, hsize_t* rows);

void Grid_Destroy(LMEgrid *grid);



#endif
