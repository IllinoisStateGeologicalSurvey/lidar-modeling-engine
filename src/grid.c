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
LMEgrid* Grid_Create(LMEbound *extent, int dims[2], double res[2]) {
	LMEgrid* grid = (LMEgrid *)malloc(sizeof(LMEgrid));
	grid->extent = extent; // should probably do this through geos and create 
	//serialier method for saving to HDF5 (-180,-90,180,90)
	grid->dims[0] = dims[0];
	grid->dims[1] = dims[1];
	grid->res[0] = res[0];
	grid->res[1] = res[1];
	grid->data = gsl_matrix_alloc((size_t)dims[0], (size_t)dims[1]);
	return grid;	
}

/**
 * @brief Generate_grid: Create a grid dataset in HDF5 to hold the points
 */
int LMEgrid_createDataset(hid_t group_id, LMEgrid* grid) {
	hid_t plist_id, dataset_id, dataspace_id;
	hid_t pixel_type;
	hsize_t cols = grid->dims[0];
	hsize_t rows = grid->dims[1];
	herr_t status;
	// ROGER uses little endian encoding for data, so force consistency
	pixel_type = H5Tcopy(H5T_IEEE_F32LE);
	// Specify the dataset dimensions from the GRID struct
	hsize_t dims[] = {rows, cols};
	hsize_t max_dims[] = {rows, cols};
	hsize_t chunk_dims[] = {2500,2500};
	// Grid is a 2d array so rank = 2;
	int rank = 2;
	plist_id = H5Pcreate(H5P_GROUP_ACCESS);
	// Create a dataspace for the dataset
	dataspace_id = H5Screate_simple(rank, &dims[0], &max_dims[0]);
	plist_id = H5Pcreate(H5P_DATASET_CREATE);
	// Set the chunking dims for the dataset
	H5Pset_chunk(plist_id, rank, &chunk_dims[0]);
	dataset_id = H5Dcreate(group_id, "testGrid", pixel_type, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);

	// Close up the handles
	/* End access to property list id */
	status = H5Pclose(plist_id);

	/* End access to the dataset */
	status = H5Dclose(dataset_id);

	/* Terminate access to dataset */
	status = H5Sclose(dataspace_id);
	status = H5Tclose(pixel_type);
	
	return 1;
}

/** 
 * \brief LMEgrid_write: Write points to a gridded dataset. 
 *
 * This should be a collective
 * call as H5 requires writes to be collective. This will not perform any
 * interpolation, but will fit points into their corresponding cells. If
 * multiple points fall in a cell, the average elevation will be used.
 */


int LMEgrid_write(hid_t group_id, int colStart, int rowStart, int colCount, int rowCount, LMEgrid* grid)
{
	hid_t dset_id, fspace_id, pixeltype, memspace_id, plist_id;
	herr_t status;
	int rank = 2;
	hsize_t stride[] = {1,1};
	hsize_t offset[] = {colStart, rowStart};
	hsize_t block[] ={colCount, rowCount};
	hsize_t count[] = {1,1};
	//Open Dataset
	char grid_name[] = "testgrid";
	plist_id = H5Pcreate(H5P_DATASET_ACCESS);
	// Open the header dataset
	dset_id = H5Dopen(group_id, grid_name, plist_id);
	// get the id for the header dataspace
	fspace_id = H5Dget_space(dset_id);
	memspace_id = H5Screate_simple(rank, &block[0], NULL);

	pixeltype = H5Tcopy(H5T_IEEE_F64LE);
	status = H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, &offset[0], &stride[0], &count[0], &block[0]);

	plist_id = H5Pcreate(H5P_DATASET_XFER);
	// Check the data
	printf("Writing grid data\n");
	status = H5Dwrite(dset_id, pixeltype, memspace_id, fspace_id, plist_id, &grid->data->data);
	status = H5Dclose(dset_id);
	status = H5Sclose(fspace_id);
	status = H5Sclose(memspace_id);
	status = H5Pclose(plist_id);
	status = H5Tclose(pixeltype);
	return 0;
}

int generateGridDataset(char* file, char* dataset_name, hsize_t* cols, hsize_t* rows) 
{
	// @note most of this can be refactored out into a catalog operation
	// Should have a catalog to manage datasets/operations
	hid_t file_id, dataset_id, dataspace_id, plist_id;
	hsize_t dims[] = {*rows, *cols};
	hsize_t max_dims[] = {*rows, *cols};
	hsize_t chunk_dims[] = {2500, 2500};
	herr_t status;

	int rank = 2;

	plist_id = H5Pcreate(H5P_FILE_ACCESS);
	file_id = H5Fcreate(file, H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);
	// Using row major since our code is all C currently
	//dims = {rows, cols};
	//@note for now make grids a fixed size
	//max_dims = {rows, cols};
	
	// @todo check the best chunk size
	//chunk_dims = {2500,2500};
	
	/* Get the dataspace, for the dataset */
	dataspace_id = H5Screate_simple(rank, &dims[0], &max_dims[0]);

	/* Set the chunking definitions for the dataset*/
	plist_id = H5Pcreate(H5P_DATASET_CREATE);
	H5Pset_chunk(plist_id, rank, &chunk_dims[0]);

	dataset_id = H5Dcreate(file_id, dataset_name, H5T_NATIVE_DOUBLE, dataspace_id, 
			H5P_DEFAULT, plist_id, H5P_DEFAULT);
	
	/* End access to property list id */
	status = H5Pclose(plist_id);

	/* End access to the dataset */
	status = H5Dclose(dataset_id);

	/* Terminate access to dataset */
	status = H5Sclose(dataspace_id);

	status = H5Fclose(file_id);

	return EXIT_SUCCESS;
}



void Grid_Destroy(LMEgrid *grid) {
	gsl_matrix_free(grid->data);
	free(grid);
}
