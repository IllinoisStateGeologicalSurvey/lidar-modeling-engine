#ifndef GRID_H
#define GRID_H

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <hdf5.h>
#include <gsl/gsl_matrix.h>
#include <inttypes.h>

typedef struct grid_t {
	bound_t *extent;
	double res[2];
	int dims[2];
	double nodata;
	gsl_matrix* data;
} grid_t;

grid_t* Grid_Create(bound_t *extent, int dims[], double res[]);

void Grid_Destroy(grid_t *grid);


#endif
