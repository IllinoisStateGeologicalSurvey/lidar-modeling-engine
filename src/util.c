/***************************************************************
 * Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  *
 * Information Laboratory (CIGI). All Rights Reserved.         *
 *                                                             *
 ***************************************************************/

/** 
 * @file util.c
 * @author Nathan Casler
 * @date May 6 2016
 * @brief File containing common utility functions
 *
 */
#ifndef UTIL_C
#define UTIL_C
/** Utility functions neede by the LAS HDF read/write functions **/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <sys/time.h>
#include "util.h"
#include <hdf5.h>
#include "hdf5_hl.h"
#include "file_util.h"
#include "header.h"

Jobstat jobstat; //stat;

// get current system time
double get_timemark() 
{
    struct timeval tsec;
    struct timezone tzone;
    gettimeofday(&tsec, &tzone);
    return (double)(tsec.tv_sec + tsec.tv_usec/1000000.0);
}

void print_jobstat()
{
    fprintf(stdout, "===================\n");
    fprintf(stdout, "||    Job Stat   ||\n");
    fprintf(stdout, "===================\n");
    fprintf(stdout, "Reading time: %.5lf seconds\n", jobstat.Tread);
    fprintf(stdout, "Data distribution time: %.5lf seconds\n", jobstat.Tcommdata);
    fprintf(stdout, "Running time: %.5lf seconds\n", jobstat.Tcompute);
    fprintf(stdout, "Result collection time: %.5lf seconds\n", jobstat.Tcommresult);
    fprintf(stdout, "Writing time: %.5lf seconds\n", jobstat.Twrite);
    fprintf(stdout, "TOTAL TIME: %.5lf seconds\n", jobstat.Ttotal);
}

// calc the data block to be read given the rank of the process
// rank, np: rank and number of procs
// pntCount, pntLength: number of points to read, number of point attributes
// offset x, offset y: output
// size x, size y: output, useful for procs positioned at the end of dim
int get_block(int mpi_rank, int np, int pntCount, int pntLength, int *offsetx, int *offsety, int *sizex, int *sizey)
{
    int blockid;
    int bpnts, rpnts; // num points and remainder
    blockid = mpi_rank;
    bpnts = pntCount / np; // find the number of points to allocate to each process
    rpnts = pntCount % np;
    *offsetx = blockid * bpnts;
    *offsety = 0;
    *sizex = bpnts;
    *sizey = pntLength;
    if (blockid == (np - 1)) { // end block
        *sizex = *sizex + rpnts;
    }
    fprintf(stderr, "Total proc: %d has %d points. Proc %d has %d points and %d offset\n", np, pntCount, mpi_rank, *sizex, *offsetx);
    return 1;


}

/**
 * Replace Character in string
 */
char* replace_char(char* str, char find, char replace) {
	char *current_pos = strchr(str,find);
	while (current_pos) {
		*current_pos = replace;
		current_pos = strchr(current_pos, find);
	}
	return str;
}

/** Replace last occurrence of character in string
*/
char* replace_last(char* str, char find, char replace) {
	char *current_pos = strrchr(str,find);
	*current_pos = replace;
	return str;
}


int getDataStore(char* h5Path) {
	char buf[PATH_MAX +1];
	//char basePath[PATH_MAX];
	//getWorkingDir(&binPath[0]);
	readlink("/proc/self/exe", &buf[0], PATH_MAX -1);
	replace_last(&buf[0], '/', '\0');
	replace_last(&buf[0], '/', '\0');
	strcat(&buf[0], "/data/LME.h5");
	strcpy(h5Path, &buf[0]);
	printf("DataStore Path: %s\n", h5Path);

	return 0;
}

int openLME(hid_t* file_id) {
	char buf[PATH_MAX +1];
	getDataStore(&buf[0]);
	hid_t plist_id;
	// Check if file is valid
	if (H5Fis_hdf5(&buf[0]) < 0) {
		perror("Failed to check file\n");
		return 0;
	} else if (H5Fis_hdf5(&buf[0]) == 0) {
		fprintf(stderr,"File is not valid HDF5 format\n");
		return 0;
	} else {
		plist_id = H5Pcreate(H5P_FILE_ACCESS);
		*file_id = H5Fopen(&buf[0], H5F_ACC_RDWR | H5F_ACC_DEBUG, plist_id);
		H5Pclose(plist_id);
		return 1;
	}
}
/**
 * \brief createArrDataset: Create and array-based dataset
 */
int createArrDataset(hid_t parent_id, char* dset_name, hsize_t *const dims, hsize_t rank, hid_t datatype) {
	
	hid_t dataset_id, dataspace_id, plist_id;
	hsize_t max_dims[rank];
	hsize_t chunk_dims[rank];
	herr_t status;
	int i;
	/** Configure the space for the dataset **/
	for (i = 0; i < rank; i++) {
		max_dims[i] = H5S_UNLIMITED;
		chunk_dims[i] = 1000;
	}
	dataspace_id = H5Screate_simple(rank, dims, &max_dims[0]);

	/* Set the chunking definitions for the dataset */
	plist_id = H5Pcreate(H5P_DATASET_CREATE);
	H5Pset_chunk(plist_id, rank, &chunk_dims[0]);

	/** Create the dataset **/
	dataset_id = H5Dcreate(parent_id, dset_name, datatype, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);

	/** Clean up **/
	status = H5Pclose(plist_id);
	status = H5Dclose(dataset_id);
	status = H5Sclose(dataspace_id);

	return 1;

}
/** createTableDataset: Create a HDF5 table dataset in the LME datastore
 * @brief Create a table formatted dataset in hdf5.
 * @param parent_id (hid_t) Identifier for the parent dataset
 * @param dset_name (char*) Name of the dataset to create
 * @param n_fields (hsize_t) Number of fields to include in the table
 * @param field_types (hid_t *) HDF5T types to include in the table
 * @param field_offsets (size_t *) The byte offsets of the datatypes in the source struct
 * @param field_names (char **) Names for the table fields
 * @param dst_size (size_t) Output datatype size
 * @param n_records (hsize_t) Number of records to write
 * @param data (void *) The data to write to the table
 * @return 1 if successful
 */
int createTableDataset(hid_t parent_id, char* dset_name, hsize_t n_fields, hid_t * const field_types, size_t * const field_offsets, const char **  field_names, size_t dst_size, hsize_t n_records, void * data){
	hid_t plist_id;
	herr_t status;


	hsize_t chunk_size = 1000;
	int *fill_data = NULL;
	int compress = 0;
	plist_id = H5Pcreate(H5P_DATASET_ACCESS);
	
	status = H5TBmake_table(dset_name, parent_id, dset_name, n_fields, n_records, dst_size, field_names, field_offsets, field_types, chunk_size, fill_data, compress, data);
	H5Pclose(plist_id);
	return 1;
}




void MPI_check_error(int mpi_err) {
	int mpi_err_class, resultlen;
	char err_buffer[MPI_MAX_ERROR_STRING];
	if (mpi_err != MPI_SUCCESS) {
		MPI_Error_class(mpi_err, &mpi_err_class);
		if (mpi_err_class == MPI_ERR_COUNT) {
			fprintf(stderr, "Invalid count used in MPI call\n");
		}
		MPI_Error_string(mpi_err, err_buffer, &resultlen);
		fprintf(stderr, err_buffer);
		MPI_Finalize();
	}
}

int divide_tasks(int count, int mpi_size, int* offsets, int* blocks) {
	int i = 0;
	int remainder = 0;
	int blockDef = 0;
	int leftOver = 0;
	blockDef = floor(count/mpi_size);
	remainder = count % mpi_size;
	printf("Remainder is %i\n", remainder);
	if (remainder > 0) {
		blockDef++;
		printf("Block template is %i\n", blockDef);
		for (i = 0; i < mpi_size; i++) {
			leftOver = i - remainder;
			if (leftOver >= 0) {
				offsets[i] = (i * blockDef) - (leftOver);
				blocks[i] = blockDef - 1;
			} else {
				offsets[i] = i * blockDef;
				blocks[i] = blockDef;
			}
			printf("[%i] Block offset: %i, size: %i, leftOver: %i\n", i, offsets[i], blocks[i], leftOver);
		}
	} else {
		for (i = 0; i < mpi_size; i++) {
			offsets[i] = i * blockDef;
			blocks[i] = blockDef;
		}
	}
	return remainder;
}

/*
 * Generate random string, useful for creating unique names for datasets 
 */

char *rand_string(char* dest, size_t size) 
{
	// Seed random number
	srand((unsigned int) time(0) + getpid());
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
	while (size-- > 0) {
		size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
		*dest++ = charset[index];		
	}
	*dest = '\0';
	return dest;
}


/** 
 * h5_user_region_init: Create a region within the LME data store
*/
int h5_user_region_init(char * const rName, hid_t *plist_id, hid_t *file_id, hid_t *user_region_id, hid_t *user_id, MPI_Comm comm, MPI_Info info) {
	char *h5_file = (char *)malloc(sizeof(char) * PATH_SIZE);
	getDataStore(h5_file);
	*plist_id = H5Pcreate(H5P_FILE_ACCESS);
	H5Pset_fapl_mpio(*plist_id, comm, info);
	*file_id = H5Fopen(h5_file, H5F_ACC_RDWR |
		H5F_ACC_DEBUG, *plist_id);
	
	*user_region_id = H5Gopen(*file_id, "users", H5P_DEFAULT);
	// Check if group exists
	if(H5Lexists(*user_region_id, rName, H5P_DEFAULT)) {
		*user_id = H5Gopen(*user_region_id, rName, H5P_DEFAULT);
	} else {
		*user_id = H5Gcreate(*user_region_id, rName, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	}
	free(h5_file);
	return 0;
}

uint64_t swap_uint64(uint64_t val) 
{
	val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
	val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
	return (val << 32) | (val >> 32);
}


#endif
