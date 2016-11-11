/***************************************************************
 * Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  *
 * Information Laboratory (CIGI). All Rights Reserved.         *
 *                                                             *
 ***************************************************************/

/** 
 * @file point_set.c
 * @author Nathan Casler
 * @date May 6 2016
 * @brief Contains methods for manipulating collections of points
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include "hdf5.h"
#include "hdf5_hl.h"
#include "bound.h"
#include "point.h"
#include "reader.h"
#include "header.h"
#include "point_set.h"
#include "util.h"
#include "grid.h"
/** 
 * @brief LMEpointSet_create: Creates an HDF5 dataset to hold the points
 */
int LMEpointSet_Create(size_t count, LMEbound extent, LMEcrs crs, LMEpointCode* points, LMEpointSet* pSet) {
	pSet->count = count;
	pSet->extent = extent;
	pSet->crs = crs;
	pSet->points = points;
	return 0;
}

int LMEpointSet_createDataset(hid_t group_id, LMEpointCode* points, uint32_t n_points)
{
	hid_t plist_id;
	hid_t code_type, retn_type, color_type;
	hsize_t  n_records = (hsize_t) n_points;
	herr_t status;
	/** Get the filename of the datastore **/
	//char* h5_file = (char *)malloc(sizeof(char)* PATH_SIZE);
	//getDataStore(h5_file);
	/** Open the datastore **/
	//plist_id = H5Pcreate(H5P_FILE_ACCESS);
	//file_id = H5Fopen(h5_file, H5F_ACC_RDWR | H5F_ACC_DEBUG, plist_id);
	/** Setup table configuration parameters **/
	const hsize_t n_fields = 6;	
	hid_t field_types[n_fields];
	/** Calculate the size and offsets for the pointCode struct */
	size_t dst_size = sizeof(LMEpointCode);
	size_t dst_offsets[6] = { HOFFSET(LMEpointCode, idx),
									HOFFSET(LMEpointCode, code),
									HOFFSET(LMEpointCode, i),
									HOFFSET(LMEpointCode, retns),
									HOFFSET(LMEpointCode, clss),
									HOFFSET(LMEpointCode, color)};
	
	const char *field_names[6] = 
		{"index","coordinates", "intensity", "returns", "classification", "color"};

	// ROGER uses little endian encoding
	//code_type = H5Tcopy( H5T_IEEE_F32LE);
	/* Coordinate codes are 32bit unsigned integers */
	code_type = H5Tcopy( H5T_STD_U32LE);
	H5Tset_size( code_type, 3);
	retn_type = H5Tcopy( H5T_STD_I16LE);
	H5Tset_size( retn_type, 2);
	// initialize color array type
	color_type = H5Tcopy( H5T_STD_I8LE);
	H5Tset_size( color_type, 3);
	// Hilbert indexes come out as bigendian
	field_types[0] = H5T_STD_U64BE;
	field_types[1] = code_type;
	field_types[2] = H5T_STD_I16LE;
	field_types[3] = retn_type;
	field_types[4] = H5T_NATIVE_UCHAR; // Need to check if the class if this should be 1byte
	field_types[5] = color_type;

	/** Table specific properties **/
	hsize_t chunk_size = 1000;
	int *fill_data = NULL;
	int compress = 0;
	plist_id = H5Pcreate(H5P_DATASET_ACCESS);
	char* dataset_name = "points";
	/** Make table **/
	status = H5TBmake_table("Test Points", group_id, dataset_name, n_fields, n_records, dst_size, field_names, dst_offsets, field_types, chunk_size, fill_data, compress, points);
	H5Tclose(code_type);
	H5Tclose(color_type);
	H5Tclose(retn_type);
	H5Pclose(plist_id);
	//H5Fclose(file_id);
	return 1;	
}
/*
int LMEpointSet_createDataset(LMEheader* header, hid_t group_id)
{
	hid_t dset_id, dataspace_id, plist_id, pointtype;
	hsize_t *dim, *cdim, *max_dim;
	dim = malloc(sizeof(hsize_t));
	cdim = malloc(sizeof(hsize_t));
	max_dim = malloc(sizeof(hsize_t));
	int rank = 1;
	herr_t status;
	//keep points in 1d array
	*dim = header->pnt_count;
	// This seems like an arbitrary designation for names
	char dataset_name[12];
	sprintf(dataset_name, "%i", (int)header->id);
	// Generate HDF5 point type
	pointtype = PointType_create(&status);
	// Create a data space
	dataspace_id = H5Screate_simple(rank, dim, dim);
	// Set data chunking for dataset
	plist_id = H5Pcreate(H5P_DATASET_CREATE);
	*cdim = 50000;
	status = H5Pset_chunk(plist_id, 1, cdim);
	
	dset_id = H5Dcreate(group_id, dataset_name, pointtype, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);
	status = H5Pclose(plist_id);
	status = H5Dclose(dset_id);
	status = H5Sclose(dataspace_id);
	PointType_destroy(pointtype, &status);

	free(dim);
	free(cdim);
	free(max_dim);
	return 1;
}
*/
/**
 * @brief LMEpointSet_writeDataset: Writes out a set of points to the HDF5 file
 * @note We need to encoded the points before writing and create an efficient
 * method for discovering pointsets
 */
int LMEpointSet_writeDataset(hid_t file_id, char* dataset, hsize_t* offset, hsize_t* block, LMEpointCode* points, MPI_Comm comm, MPI_Info info) {
	hid_t dset_id, pointtype, memspace_id, fspace_id, plist_id;
	herr_t status;
	int rank = 1;
	int mpi_rank;
	MPI_Comm_rank(comm, &mpi_rank);
	fprintf(stdout, "[%i] LMEpointSet_writeDataset called\n", mpi_rank);

	plist_id = H5Pcreate(H5P_DATASET_ACCESS);
	dset_id = H5Dopen(file_id, dataset, plist_id);
	fprintf(stdout,"[%i] Dataset opened: %s \n", mpi_rank, dataset);
	fspace_id = H5Dget_space(dset_id);
	memspace_id = H5Screate_simple(rank, block, NULL);
	fprintf(stdout, "[%i] FileSpace Created\n", mpi_rank);

	pointtype = PointType_create(&status);
	status = H5Pclose(plist_id);
	status = H5Sclose(fspace_id);
	status = H5Sclose(memspace_id);
	PointType_destroy(pointtype, &status);
	MPI_Barrier(comm);
	return 1;
}

int LMEpointSet_writeFromLAS(char* LASpath, char* dataset_name, hsize_t* pointBlock, hid_t group_id, MPI_Comm comm, MPI_Info info) {
	LMEpointCode* points;
	hsize_t pointCnt = *pointBlock;
	hsize_t pointOffset = 0;
	points = malloc(sizeof(LMEpointCode) * pointCnt);

	LASReaderH lasReader;
	int mpi_rank;
	MPI_Comm_rank(comm, &mpi_rank);
	lasReader = LASReader_Create(LASpath);
	fprintf(stdout, "[%i] Beginning read from %s\n", mpi_rank, LASpath);

	if (!lasReader) {
		LASError_Print("Could not open the file for reading\n");
		return 0;
	}
	MPI_Barrier(comm);
	if (!LASFile_read(lasReader, &pointOffset, &pointCnt, points, mpi_rank)) {
		fprintf(stderr, "[%i] IO Error: Failed to read LAS file %s\n", mpi_rank, LASpath);
		free(points);
		return 0;
	}
	fprintf(stdout, "[%i] Successfully read file: %s\n", mpi_rank, LASpath);
	MPI_Barrier(comm);
	LMEpointSet_write(group_id, dataset_name, &pointOffset, pointBlock, points, comm, info);
	free(points);
	return 1;
}
/**
 * LMEpointSet_read: will read all the points in a point set into memory
 */
int LMEpointSet_read(hid_t group_id, char* dataset_name, hsize_t pointCount, LMEpointCode* points) {
	hid_t plist_id;
	herr_t status;
	
	plist_id = H5Pcreate(H5P_DATASET_ACCESS);
	// Open the dataset
	size_t dst_size = sizeof(LMEpointCode);
	size_t dst_offsets[6] = { HOFFSET(LMEpointCode, idx),
									HOFFSET(LMEpointCode, code),
									HOFFSET(LMEpointCode, i),
									HOFFSET(LMEpointCode, retns),
									HOFFSET(LMEpointCode, clss),
									HOFFSET(LMEpointCode, color)};
	// Note: Need to calculate this
	size_t sizes[6] = { sizeof(points[0].idx),
						sizeof(points[0].code),
						sizeof(points[0].i),
						sizeof(points[0].retns),
						sizeof(points[0].clss),
						sizeof(points[0].color)};

	status = H5TBread_table(group_id, dataset_name, dst_size, dst_offsets, sizes, points);
	printf("Point set read successfully");
	return 0;
	

}

int LMEpointSet_grid(hid_t group_id, hid_t dset_id, hsize_t pointBlock, LMEgrid* grid) {
	//Get Upper left corner, and interpolate points into cells
	
	int i = 0;
	LMEpointCode* points = malloc(sizeof(LMEpointCode) * pointBlock);
	//Note should use Vicenty's inverse algorithm to calculate distance if we are using
	//WGS84 for our base projection
	LMEpointSet_read(group_id, "testGrid", pointBlock, points);
	for (i = 0; i < pointBlock; i++) {
		// Unscale back to WGS84
		// Find the distance from raster origin
		// interpolate to raster based on distance and grid resolution
		printf("TEST");
	}
	return 0;
}
	

// TODO: Finish methods for outputting points
// TODO: GET SOME VISUALS!!
// int PointSet_toLAS()
//
