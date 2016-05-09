/***************************************************************
 * Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  *
 * Information Laboratory (CIGI). All Rights Reserved.         *
 *                                                             *
 ***************************************************************/

/** 
 * @file init.c
 * @author Nathan Casler
 * @date May 6 2016
 * @brief Function will initialize an empty LME datastore
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
#include <unistd.h>
#include "file_util.h"
#include "header.h"
#include "reader.h"



void usage()
{
    fprintf(stderr, "-------------------------------------------------------\n");
    fprintf(stderr, "                  DSMinit usage:\n");
    fprintf(stderr, "\n");
    fprintf(stderr, " DSMinit \n");
    fprintf(stderr, "-------------------------------------------------------\n");
}

int main(int argc, char* argv[])
{
    char binPath[PATH_MAX];
    char h5Path[PATH_MAX];
    char* h5PathPtr = h5Path;
    char dataDir[10] = "/data";
    char h5_file[15] = "/LME.h5\0";
    getWorkingDir(&binPath[0]);
    strcpy(h5PathPtr, dirname(&binPath[0]));
    strcat(h5PathPtr, &dataDir[0]);
    strcat(h5PathPtr, &h5_file[0]);
    
   
    if ( access( h5PathPtr, F_OK ) != -1 ) {
        fprintf(stderr, "Error: data store already exists at %s\n", h5PathPtr);
        exit(1);
    } else {
        printf("Creating data store at %s\n", h5PathPtr);
    }
    double start, current;

    hid_t file_id, group_id, dset_id, fspace_id, fapl_id, fcpl_id;
    
    int mpi_size, mpi_rank, mpi_err, mpi_err_class, resultlen;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;
	// Should improve mpi-io on gpfs in HDF5
    MPI_Info_set(info, "IBM_largeblock_io", "true");
	MPI_Status status;
    char err_buffer[MPI_MAX_ERROR_STRING];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &mpi_size);
    MPI_Comm_rank(comm, &mpi_rank);
    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
    start = MPI_Wtime();

    
    fcpl_id = H5Pcreate(H5P_FILE_CREATE);
    fapl_id = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_mpio(fapl_id, comm, info);

    file_id = H5Fcreate(h5PathPtr, H5F_ACC_TRUNC | H5F_ACC_DEBUG, fcpl_id, fapl_id);
    group_id = H5Gcreate(file_id, "/regions", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    printf("Data store initialized at %s\n", h5PathPtr);
    
    H5Pclose(fcpl_id);
    H5Pclose(fapl_id);
    H5Gclose(group_id);
    H5Fclose(file_id);
    MPI_Finalize();
    return 0;
}

