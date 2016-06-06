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

void Grid_Destroy(LMEgrid *grid);


#endif
