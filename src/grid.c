/**
 * @file grid.c
 * @author Nathan Casler
 * @date May 9 2016
 * @brief File containing methods for gridded data
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <hdf5.h>
#include <math.h>
#include <inttypes.h>
#include <limits.h>
#include <proj_api.h>
#include <liblas/capi/liblas.h>

#include <gsl/gsl_matrix.h>
#include "point.h"
#include "bound.h"
#include "grid.h"

/** 
 * @brief Grid_Create: Create a grid data structure
 *
 * @return grid: grid_t = Grid data structure
 */
grid_t* Grid_Create(bound_t *extent, int dims[2], double res[2]) {
	grid_t* grid = (grid_t *)malloc(sizeof(grid_t));
	grid->extent = extent; // should probably do this through geos and create serialier method for saving to HDF5 (-180,-90,180,90)
	grid->dims[0] = dims[0];
	grid->dims[1] = dims[1];
	grid->res[0] = res[0];
	grid->res[1] = res[1];
	grid->data = gsl_matrix_alloc((size_t)dims[0], (size_t)dims[1]);
	return grid;	
}



void Grid_Destroy(grid_t *grid) {
	gsl_matrix_free(grid->data);
	free(grid);
}
