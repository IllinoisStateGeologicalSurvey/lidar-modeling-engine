/***************************************************************
 * Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  *
 * Information Laboratory (CIGI). All Rights Reserved.         *
 *                                                             *
 ***************************************************************/

/** 
 * @file addRegionPar.c
 * @author Nathan Casler
 * @date May 6 2016
 * @brief Function which will add a region to the LME datastore in parallel via MPI
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <hdf5.h>
#include <math.h>
#include <mpi.h>
#include <errno.h>
#include <libgen.h>
#include "util.h"
#include "file_util.h"
#include "reader.h"
#include "header.h"


void usage()
{
	fprintf(stderr, "----------------------------------------------------------------------------\n");
	fprintf(stderr, "                    regionAdd       \n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  regionAdd  -r <regionName> -p <regionPath>\n");
	fprintf(stderr, "----------------------------------------------------------------------------\n");
}

void parseArgs(int argc, char* argv[], char* rName, char* rPath, int* verbose) 
{
	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-r") == 0 ||
				strcmp(argv[i], "--region") == 0)
		{
			i++;
			strcpy(rName, argv[i]);
		}
		else if (strcmp(argv[i], "-p") == 0 ||
				strcmp(argv[i], "--path") == 0)
		{
			i++;
			strcpy(rPath, argv[i]);
		} else if (strcmp(argv[i], "-v") == 0 ||
				strcmp(argv[i], "--verbose") == 0)
		{
			*verbose = 1;
		} else 
		{	
			fprintf(stderr, "Error: Unknown Argument '%s'\n", argv[i]);
			usage();
			exit(1);
		}

	}
}

int h5_region_init(const char* h5_file, const char* rName, hid_t *plist_id, hid_t *file_id, hid_t * region_grp_id, hid_t* region_id, MPI_Comm comm, MPI_Info info) {
	/* Open the file */
	*plist_id = H5Pcreate(H5P_FILE_ACCESS);
	H5Pset_fapl_mpio(*plist_id, comm, info);

	*file_id = H5Fopen(h5_file, H5F_ACC_RDWR | H5F_ACC_DEBUG, *plist_id);
	*plist_id = H5Pcreate(H5P_GROUP_CREATE);
	*region_grp_id = H5Gopen(*file_id, "regions", H5P_DEFAULT);

	//Check if group exists
	if (H5Lexists(*region_grp_id, rName, H5P_DEFAULT)) {
		*region_id = H5Gopen(*region_grp_id, rName, H5P_DEFAULT);
	} else { 
		*region_id = H5Gcreate(*region_grp_id, rName, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	}
	return 0;
}

int h5_region_prepare(hid_t *region_id, int lasCount, hid_t* headertype, hid_t* plist_id, hid_t* dataspace_id, hid_t* dataset_id, herr_t* h5_status) {
	hsize_t dim, chunk_dim, max_dim;

	*headertype = HeaderType_create(h5_status);
	
	int rank = 1;
	dim = (hsize_t) lasCount;
	max_dim = H5S_UNLIMITED;
	// Need to look into optimal chunk size
	// NGA Mentioned 50,000 as their optimal chunking
	chunk_dim = 50000;
	*dataspace_id = H5Screate_simple(rank, &dim, &max_dim);
	// Create DataSet
	*plist_id = H5Pcreate(H5P_DATASET_CREATE);
	H5Pset_chunk(*plist_id, rank, &chunk_dim);
	if (H5Lexists(*region_id, "headers", H5P_DEFAULT)) {
		*plist_id = H5Pcreate(H5P_DATASET_ACCESS);
		*dataset_id = H5Dopen(*region_id, "headers", *plist_id);
	} else {
		*dataset_id = H5Dcreate(*region_id, "headers", *headertype, *dataspace_id, H5P_DEFAULT, *plist_id, H5P_DEFAULT);
	}
	return 0;
}

int main(int argc, char* argv[]) {

	//Variables to hold user arguments
	char* h5_file = (char *)malloc(sizeof(char)* PATH_LEN);
	// Get the path to HDF5 file
	getDataStore(h5_file);
	char* rName = (char *)malloc(sizeof(char)* PATH_LEN);
	char* rPath = (char *)malloc(sizeof(char)* PATH_LEN);
	int verbose;
	// Parse user arguments
	parseArgs(argc, argv, rName, rPath, &verbose);
	// Get the numbe of files to read
	int lasCount = countLAS(rPath);
	// Variables to hold offset and block for each process
	int i;
	// H5 Variables
	hid_t file_id, region_grp_id, region_id, plist_id, dataset_id, dataspace_id, headertype;
	herr_t h5_status;
	//hsize_t dim, max_dim, chunk_dim;
	//MPI Variables
	int mpi_size, mpi_rank, mpi_err;
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Info info = MPI_INFO_NULL;
	MPI_Status status;

	/* Init MPI */
	MPI_Init(&argc, &argv);
	MPI_Comm_size(comm, &mpi_size);
	MPI_Comm_rank(comm, &mpi_rank);
	MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN); /* Return info about errors */

	// Get the task counts,offsets for each process
	int t_offsets[mpi_size];
	int t_blocks[mpi_size];
	divide_tasks(lasCount, mpi_size, &t_offsets[0], &t_blocks[0]);

	// Open the File and region
	h5_region_init(h5_file, rName, &plist_id, &file_id, &region_grp_id, &region_id, comm, info);
	// Create/Open the dataset for writing
	h5_region_prepare(&region_id, lasCount, &headertype, &plist_id, &dataspace_id, &dataset_id, &h5_status);
	// READ
	// Allocate space for headers
	char* subPaths = malloc(sizeof(char) * t_blocks[mpi_rank]* PATH_LEN);
	//Allocate space for headers
	LMEheader* headers = (LMEheader *) malloc(sizeof(LMEheader) * t_blocks[mpi_rank]);

	// Read the filenames
	if (mpi_rank == 0) {
		// Allocate memory for headers
		char *outPaths = malloc(sizeof(char) * ((size_t)lasCount * PATH_LEN));
		// Read file paths from folder path
		buildArray(rPath, outPaths, lasCount);
		printf("Gathered %i paths, send paths to processes\n", lasCount);
		// Send the headers to the processes
		for (i =1; i < mpi_size; i++) {
			printf("Sending paths to %i\n", i);
			mpi_err = MPI_Send(&outPaths[t_offsets[i] * PATH_LEN], t_blocks[i] * PATH_LEN, MPI_CHAR, i, 1, comm);
			MPI_check_error(mpi_err);
			printf("Sent paths[%i]-[%i] to %i\n", t_offsets[i], (t_offsets[i] + t_blocks[i]), i);
		}
		for (i = 0; i < t_blocks[0]; i++) {
			strcpy(&subPaths[i*PATH_LEN], &outPaths[i * PATH_LEN]);
			//subPaths[i] = outPaths[(i) * PATH_LEN];
		}
		free(outPaths);
	} else {
		mpi_err = MPI_Recv(&subPaths[0], t_blocks[mpi_rank] * PATH_LEN, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
		MPI_check_error(mpi_err);
	}
		MPI_Barrier(comm);
		// Stat the file paths for summary info
		printf("[%i] Has File: %s\n", mpi_rank, &subPaths[0]);

		if (!LMEheaderBlock_read(&subPaths[0], 0, t_blocks[mpi_rank], headers)) {
			fprintf(stderr, "IO Error: Failed to read header data");
			return 1;
		}
		free(subPaths);
		// Send header values to each process
		//for (i = 1; i < mpi_size; i++) {
		//	mpi_err = MPI_Send(&headers[t_offsets[i]], t_blocks[i] * sizeof(header_t), MPI_BYTE, i, 1, comm);
		//	MPI_check_error(mpi_err);
		//	printf("Sent header[%i]-[%i] to %i\n", t_offsets[i], (t_offsets[i] + t_blocks[i]), i);
		//}

	// Write
	hsize_t hBlock = t_blocks[mpi_rank];
	hsize_t hOffset = t_offsets[mpi_rank];

	plist_id = H5Pcreate(H5P_FILE_ACCESS);
	H5Pset_fapl_mpio(plist_id, comm, info);

	LMEheaderBlock_write(region_id, "headers", &hOffset, &hBlock, headers, comm, info); 


	H5Dclose(dataset_id);
	H5Sclose(dataspace_id);
	HeaderType_destroy(headertype, &h5_status);
	H5Gclose(region_id);
	H5Gclose(region_grp_id);
	H5Pclose(plist_id);
	H5Fclose(file_id);
	free(headers);
	free(h5_file);
	free(rName);
	free(rPath);
	MPI_Finalize();

	return 0;
}
