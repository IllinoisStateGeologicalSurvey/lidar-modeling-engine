/***************************************************************
 * Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  *
 * Information Laboratory (CIGI). All Rights Reserved.         *
 *                                                             *
 ***************************************************************/

/** 
 * @file readLas.c
 * @author Nathan Casler
 * @date May 6 2016
 * @deprecated This function no longer fits with the H5 architecture of the
 * program, some heavy refactoring will be needed to update it.
 * @brief Test to check if a parallel point reading of an LAS file is functional
 *
 */
/**********************************************
 * This is sampled from the libLAS las2las source code
 *
 * Will work to read a las file in parallel
 **********************************************/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mpi.h"
#include <liblas/capi/liblas.h>
#include <proj_api.h>
#include "common.h"
#include "point.h"
#include "file_util.h"
#include "reader.h"
#include "header.h"

#define LAS_FORMAT_10 0
#define LAS_FORMAT_11 1
#define LAS_FORMAT_12 2


/** TO DO: Write projection code using past code
            Implement MPI based las partitioner
            Create OFFSET,SCALE function to push xyz to 32bit integers
            *******************************/
void usage()
{
    fprintf(stderr,"----------------------------------------------\n");
    fprintf(stderr,"       lasReader usage:\n");
    fprintf(stderr,"\n");
    fprintf(stderr," readLas -i in.las\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"----------------------------------------------\n");

}
void ptime(const char *const msg)
{
    float t= ((float)clock())/CLOCKS_PER_SEC;
    fprintf(stderr, "cumulative CPU time thru %s = %f\n", msg, t);
}


int main (int argc, char* argv[])
{
    int i;
	//int i,j;
    int verbose = FALSE;
    char* file_name_in = 0;
	hid_t file_id;
    int fileCount = 0;
    LASReaderH reader = NULL;
    LASHeaderH header = NULL;
    
    int mpi_size, mpi_rank, mpi_err;;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;

    /* Initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &mpi_size);
    MPI_Comm_rank(comm, &mpi_rank);

    /* Check number of processors */
    for (i = 1; i < argc; i++) {
        if (    strcmp(argv[i],"-h") == 0 ||
                strcmp(argv[i],"--help") == 0
            )
        {
            usage();
            exit(0);
        }
        else if (strcmp(argv[i],"-v") == 0 ||
                strcmp(argv[i],"--verbose") == 0
            )
        {
            verbose = TRUE;
        }

        else if (    strcmp(argv[i],"--input") == 0 ||
                strcmp(argv[i],"-input") == 0 ||
                strcmp(argv[i],"-i") == 0 ||
                strcmp(argv[i],"-in") == 0
            )
        {
            i++;
            file_name_in = argv[i];
        }
        else if (i == argc - 1 && file_name_in == NULL)
        {
            file_name_in = argv[i];
        }
        else 
        {
            fprintf(stderr, "ERROR: unknown argument '%s'\n", argv[i]);
            usage();
            exit(1);
        }
    }
    /* Will need to support multiple input files */
    //fileCount = countLAS(file_name_in);
    printf("File count is %d\n", fileCount);
    //NOTE: THIS LOGIC SHOULD BE CLEANED UP and moved to other function
    if (file_name_in)
    {
        reader = LASReader_Create(file_name_in);
        if (!reader) {
            LASError_Print("Could not open file to read");
            exit(1);
        }
    }
    else
    {
        LASError_Print("Could not open file to read");
        usage();
        exit(1);
    }

    header = LASReader_GetHeader(reader);
    if (!header) {
        LASError_Print("Could not fetch header");
        exit(1);
    }
    
    if (verbose) {
        ptime("start.");
        fprintf(stderr, "[%i] First pass reading %d points ...\n", mpi_rank, LASHeader_GetPointRecordsCount(header));
    }
    int pntCount = LASHeader_GetPointRecordsCount(header);
    int block_len = floor(pntCount / mpi_size);
    int remainder = pntCount % mpi_size;
    if (remainder) {
        printf("Block size is %d and Remainder is %d\n", block_len, remainder);
        block_len++;
    }
    printf("[%i] Block size is %d\n", mpi_rank, block_len);
    ptime("Block created");
    LMEpointCode* points;
    if (mpi_rank == 0) {
        points = malloc(sizeof(LMEpointCode) * pntCount);
    }   

    LMEpointCode* sub_points = (LMEpointCode*)malloc(sizeof(LMEpointCode) * block_len);
    LMEpointCode* point =  (LMEpointCode*)malloc(sizeof(LMEpointCode));
    //MPI_Datatype* mpi_pointtype = NULL;
    char * h5path = (char *)malloc(sizeof(char) * PATH_MAX);
	LMEheader lmeHeader;
	// TODO: Placeholder for now, should switch this so that the header is a table
	// dataset with a primary index which could have a link to point datasets
	uint32_t header_id = 9999
	/* TODO: Make file reading for headers and point less convoluted process.
	 * Could make the functions pass the h_id tp each other, make a dataset
	 * creation metaclass */
	getDataStore(h5Path);
	plist_id = H5Pcreate(H5P_FILE_ACCESS);
	H5Fopen(h5_path, H5F_ACC_RDWR | H5F_ACC_DEBUG, *plist_id);
	Header_read(file_name_in, LMEheader, header_id);
	
    
    /** CAN WE MAKE THIS A 1-D array **/
    hsize_t* psize = malloc(sizeof(hsize_t));
    hsize_t* block = malloc(sizeof(hsize_t));
    hsize_t* offset = malloc(sizeof(hsize_t));
    *psize = pntCount;
    *block = block_len;
    *offset = mpi_rank * block_len;
    printf("[%i] Creating point type for MPI\n", mpi_rank);
    // Todo: Find out why mpi struct creation causes segfaults
    mpi_err = MPI_Scatter(points, (sizeof(LMEpointCode) *  block_len), MPI_BYTE, sub_points, (sizeof(LMEpointCode) * block_len), MPI_BYTE, 0, comm);
    //MPI_PointType_create(mpi_pointtype);
    printf("[%i] Scattering points to processes\n", mpi_rank);
    
    //MPI_Scatter(points, block_len, *mpi_pointtype, sub_points, block_len, *mpi_pointtype, 0, MPI_COMM_WORLD);
    
    // TO DO:Need to check the clean up in readBlock to make sure there is no leak 
    printf("[%i] Reading points from file\n", mpi_rank);
    if (!LASFile_read(reader, offset, block, sub_points, mpi_rank)) {
		fprintf(stderr, "IO Error: Failed to read %s\n", file_name_in);
		LASReader_Destroy(reader);
		LMEpointCode_destroy(sub_points);
		LMEpointCode_destroy(point);
		if (mpi_rank == 0) {
			free(points);
		}
		MPI_Finalize();
		return 1;
	}
    printf("[%i] Finished reading points\n", mpi_rank); 
	ptime("Points read");
   
    if (mpi_rank == 0) {
        // Should rename this function to specify the type of dataset
		// NEED TO READ THE Header data into a LMEheader object, before writing
		
		createDataset("test.h5", "/pts", psize);
        printf("[%i] Dataset created\n", mpi_rank);
        ptime("Dataset created.");
    }
    printf("[%i] writing points\n", mpi_rank);
    MPI_Barrier(comm);

    //MPI_Gather(&sub_points, 1, MPI_FLOAT, points, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    
	PointSet_write("test.h5", "/pts", offset, block, sub_points, comm, info);
	ptime("Points written");
	printf("[%i] Points written, cleaning up\n", mpi_rank);

    
    // Clean up
    LASReader_Destroy(reader);
    reader = NULL;
//    for (i = 0; i < *block; i++) {
//        Point_destroy(&sub_points[i]);
//    }
    LMEpointCode_destroy(sub_points);
    LMEpointCode_destroy(point);
    //free(mpi_pointtype);
    if (mpi_rank == 0) {
  //      for (i = 0; i < pntCount; i++) {
  //          Point_destroy(&points[i]);
  //      }
        free(points);
		//Point_destroy(points);
    }
	printf("[%i] Clean up successful, exiting.\n", mpi_rank);
    if (verbose) ptime("done.");

    MPI_Finalize();
    
    return 0;
}


