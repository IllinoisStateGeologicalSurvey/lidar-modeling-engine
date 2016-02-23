#include "header.h"
#include "reader.h"
#include "point.h"
#include "common.h"
#include "file_util.h"
#include <hdf5.h>
#include <math.h>
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <liblas/capi/liblas.h>
#include <proj_api.h>

/* THIS TEST WILL READ A SET OF HEADER FROM THE HEADER DATASET, 
   then READ THE Associated LAS files from the filesystem into the hdf5 file*/

void usage()
{
    fprintf(stderr, "----------------------------------------\n");
    fprintf(stderr, "            headerTest  usage:\n");
    fprintf(stderr, "\n");
    fprintf(stderr, " headerRead -i <h5file> -t <numFiles>\n");
    fprintf(stderr, "----------------------------------------\n");
}

void ptime(const char* const msg)
{
    float t = ((float)clock())/CLOCKS_PER_SEC;
    fprintf(stderr, "cumulative CPU time thru %s = %f\n", msg, t);
}


void parseArgs(int argc, char* argv[],int* fileCount, char* file_name, int* verbose) 
{
    int i;
    /* Check the number of processors */
    for ( i = 1; i < argc; i++)
    {
        if (    strcmp(argv[i], "-h") == 0 ||
                strcmp(argv[i], "--help") == 0
            )
        {
            usage();
            exit(0);
        }
        else if (strcmp(argv[i], "-v") == 0 ||
                strcmp(argv[i], "--verbose") == 0
                )
        {
            *verbose = 1;
        }
        else if (strcmp(argv[i], "-i") == 0 ||
                strcmp(argv[i], "-in") == 0 ||
                strcmp(argv[i], "--input") == 0
                )
        {
            i++;
            strcpy(file_name, argv[i]);
        }
        else if (strcmp(argv[i], "-t") == 0 ||
                strcmp(argv[i], "--toRead") == 0)
        {
            i++;
            *fileCount = atoi(argv[i]);
        }
        else 
        {
            fprintf(stderr, "ERROR: unknown argument '%s'\n", argv[i]);
            usage();
            exit(1);
        }
    }
}

int main(int argc, char* argv[])
{
    // Indexes
    int i;
    // Flag to determine logging level
    int verbose;
    // Placeholder to test different files in the future
    char* file_name_in = malloc(sizeof(char) * (PATH_MAX+1)); // Holds HDF5 filename
    //char dset_name[PATH_MAX]; // Buffer for HDF5 point dataset name
    int fileCount; // Holds the number of LAS files to read
    /* LAS Specific variables */
    LASReaderH lasReader = NULL; // Used to read points from a LAS file
    /* Header related variables */
    char* pathBuf = malloc(sizeof(char)* (PATH_MAX+1)); // Buffer to hold las file paths
    /* HDF5 variables */
    hid_t file_id, group_id, plist_id;
    
    /* Timer related variables */
    double start, current;
    /* MPI related variables */
    
    int mpi_size, mpi_rank,  mpi_err, mpi_err_class, resultlen;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;
    MPI_Datatype* mpi_headertype;
    MPI_Status status;
    char err_buffer[MPI_MAX_ERROR_STRING];

    hsize_t headerBlock, pointBlock, headerOffset;
    int remainder;
    //Point* points;
    header_t* sub_headers;
    header_t* headers;
    /* Initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &mpi_size);
    MPI_Comm_rank(comm, &mpi_rank);
    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN); /* Return info about errors*/
    //printf("Setting MPI Heaader type\n");
    //MPI_HeaderType_create(mpi_headertype);
	start = MPI_Wtime();
    /* Read supplied arguments */
    parseArgs(argc, argv, &fileCount, file_name_in, &verbose);
    
    printf("File count is %d\n", fileCount);
    // Allocate space for headers
    //if (mpi_rank == 0) {
    headers = malloc(sizeof(header_t) * fileCount); // Buffer to hold the header data
    //}
    if (mpi_size > 1) {
        headerBlock = floor(fileCount / mpi_size);
        remainder = fileCount % mpi_size;
        if (remainder) {
            headerBlock++;
        }
        sub_headers = malloc(sizeof(header_t) * headerBlock);
        headerOffset =(mpi_rank * headerBlock); 
        printf("[%i] Offset: %i, Block: %i, Remainder: %i\n", mpi_rank, (int)headerOffset, (int)headerBlock, remainder);
    } else {
        headerBlock = fileCount;
        headerOffset = 0;
    }
    if (file_name_in)
    {
        MPI_Barrier(comm);
        plist_id = H5Pcreate(H5P_FILE_ACCESS);
        H5Pset_fapl_mpio(plist_id, comm, info);
        file_id = H5Fopen(file_name_in, H5F_ACC_RDWR | H5F_ACC_DEBUG, plist_id);
        // Create a group for the points
        plist_id = H5Pcreate(H5P_GROUP_CREATE);
        group_id = H5Gcreate(file_id, "/points", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        // Master process reads <fileCount> from headers
        //if (mpi_rank == 0) {
            printf("[%i] Reading headers\n", mpi_rank);
            fprintf(stderr, "FILENAME: %s\n", file_name_in);
            HeaderSet_read(0, fileCount, headers, file_name_in);
            printf("[%i] Successfully loaded headers.\n", mpi_rank);
            
        //}
        // Scatter the header information, prevent other processes from running
        // before they are given their headers
        //MPI_HeaderType_create(mpi_headertype);
        
        //mpi_err = MPI_Scatter(headers, headerBlock, *mpi_headertype, sub_headers, headerBlock, *mpi_headertype, 0, comm);
        //mpi_err = MPI_Scatter(headers, (sizeof(header_t) * headerBlock), MPI_BYTE, sub_headers, (sizeof(header_t)*headerBlock), MPI_BYTE, 0, comm);
        // Error Checking 
        //if (mpi_err != MPI_SUCCESS) {
        //    MPI_Error_class(mpi_err, &mpi_err_class);
        //    if (mpi_err_class == MPI_ERR_COUNT) {
        //        printf("Invalid count used in MPI scatter call\n");
        //    }
        //    MPI_Error_string(mpi_err, err_buffer, &resultlen);
        //    fprintf(stderr, err_buffer);
        //    MPI_Finalize();
        //}
            
        //printf("[%i] Successfully scattered headers\n", mpi_rank);
		current = MPI_Wtime();
		printf("[%i] Elapsed time: %f\n", mpi_rank, (current - start));
        // Barrier prevent processes from running without paths
        MPI_Barrier(comm);
        for (i = 0; i < fileCount; i++) {
            printf("[%d] Iteration %d started\n", mpi_rank, i); 
            //if (mpi_size > 1) {
                printf("[%d] Reading %d  of %d files.\n", mpi_rank,i+1, (int)headerBlock);
                // Get the h5 datasetname to write to
                //printf(" POINT_SET_PREPARE called on :%s\n", &headers[i].path);
                //printf(" NEXT HEADER IS: %s\n", &sub_headers[i+1].path);
                //PointSet_prepare(&sub_headers[i],  pathBuf, &pointBlock);
                //HeaderPoint_get(&sub_headers[i], &dset_name[0]);
                //strcpy(pathBuf, sub_headers[i].path);
                // Retrieve the length of the h5 dataset
                //pointBlock = sub_headers[i].pnt_count;
                //Creates the hdf5 dataset in file
                PointSet_create(&headers[i], group_id);
                printf("[%i] Point set created successfully\n", mpi_rank);
                current = MPI_Wtime();
                printf("[%i] Elapsed time: %f\n", mpi_rank, (current - start));
                MPI_Barrier(comm);
        }
        int j;
        char dataset_name[12];
        for (i = 0; i < headerBlock; i++) {
            j = headerOffset + i;
            pointBlock = headers[j].pnt_count;
            //TODO: Integrate offset, count here
            //} else {
            //    printf("Reading %d of %zu files.\n", i, (size_t)headerBlock);
            //    PointSet_prepare(&headers[i], pathBuf, &pointBlock);
            //    printf("Created space for dataset\n");
            //    PointSet_create(&headers[i], file_id, &dset_name[0]);
            //}
            //printf("[%d] Outside conditional\n", mpi_rank);
            // Allocate space to hold the points for the LAS file
            sprintf(dataset_name, "%i", (int)headers[j].id);
            printf("[%i], writing dataset %i of %i, with name %s, pntCount=%zu\n", mpi_rank, i, headerBlock, dataset_name, pointBlock);
            PointSet_copy(headers[j].path, dataset_name, &pointBlock, group_id, comm, info);
            current = MPI_Wtime();
            printf("[%i] Elapsed time: %f\n", mpi_rank, (current - start));
        }
    }

    printf("[%i] Cleaning up\n", mpi_rank);
    current = MPI_Wtime();
    printf("[%i] Elapsed time: %f\n", mpi_rank, (current-start));
    LASReader_Destroy(lasReader);
    lasReader = NULL;
    if (mpi_rank == 0){
        free(headers);
    }
    free(sub_headers);
    //free(dset_name);
    H5Gclose(group_id);
    checkOrphans(file_id, mpi_rank);
    H5Fclose(file_id);
    free(file_name_in);
    
    //Point_destroy(sub_points);
    //Point_destroy(point);
    //if (mpi_rank == 0) {
    //Point_destroy(points);
    //}
    
    if(verbose) ptime("done.");
	current = MPI_Wtime();       
    printf("[%i] Exiting. Total Run Time: %f\n", mpi_rank, (current-start));
    MPI_Finalize();
    return 0;
    
}
/* Steps:

   Read 10 lines from header dataset
   Read the corresponding LAS files 
   Write the LAS datasets
   Exit
*/
