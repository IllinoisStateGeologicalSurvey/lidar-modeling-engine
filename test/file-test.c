//This is a test to check that the file reading utilities work

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <math.h>
#include <mpi.h>
#include <errno.h>
#include "file_util.h"
#include "util.h"
#include "common.h"
#include "header.h"
#include "reader.h"

void usage()
{
	fprintf(stderr, "----------------------------------------------\n");
	fprintf(stderr, "              fileTest usage:\n");
	fprintf(stderr, "\n");
	fprintf(stderr, " fileTest -i <path-to-las-files> -n <number-to-read> -f <hdf5-file>\n");
	fprintf(stderr, "----------------------------------------------\n");
}


void parseArgs(int argc, char* argv[], char *lasPath, size_t* fileCount, char* h5_file, int* verbose)
{
	int i;
	/* CHeck the number of processors */
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
			strcpy(lasPath, argv[i]);
		}
		else if (strcmp(argv[i], "-t") == 0 ||
				strcmp(argv[i], "--num-files") == 0
				)
		{
			i++;
			*fileCount = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "-f") == 0 ||
				strcmp(argv[i], "--h5-file") == 0)
		{
			i++;
			strcpy(h5_file, argv[i]);
		} else
		{
			fprintf(stderr, "Error: unknown argument '%s'\n", argv[i]);
			usage();
			exit(1);
		}
	}
}

int main(int argc, char* argv[])
{
    char dirname[PATH_LEN];
    char h5_file[PATH_LEN];
    size_t size;
    int i;
    int verbose;
    size_t totPoints = 0;
    int mpi_size, mpi_rank, mpi_err;
    mpi_size = 0;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;
    MPI_Status status;
    /* Initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &mpi_size);
    MPI_Comm_rank(comm, &mpi_rank);
    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN); /* Return info about errors*/
    printf("[%d] Starting  %d processes\n", mpi_rank, mpi_size);
    /* Check supplied arguments */
    //if (argc < 2)
    //{
    //    fprintf(stderr,  "Usage: fileTest <directory> <# of files>\n");
    //    exit(1);
    //}
	parseArgs(argc, argv, &dirname[0], &size, &h5_file[0], &verbose);
	//realpath(argv[1], dirname);
    //strcpy(dirname, argv[1]);
    printf("Dirname: %s\n", dirname);
    int file_count = countLAS(dirname);
    size = (size_t)file_count;
    hsize_t dims = size;
    printf("Need to read %i files\n", file_count);
    /** Mpi task division **/    

	
    /* Root process creates dataset */
    
    //char* pathBuf = malloc(sizeof(char) * (PATH_MAX+1)); /** Create a char buffer to hold the working path **/
    //char pathBuf[PATH_MAX+1];
    //int pathLen; /** Counter that returns the index of the end of the path(helpful for concatenation  **/
    //pathLen = getWorkingDir(pathBuf); 
    
    //printf("Working Path is: %s\n", pathBuf);
    //strcpy(&pathBuf[pathLen], "/test.h5\0");
    //printf("File path is: %s\n", pathBuf);
    //char* filename = "test.h5";
    if (mpi_rank == 0) {
        createHeaderDataset(h5_file, "/headers", &dims);
        printf("created HDF dataset at %s\n", h5_file);
    }
    // Add a task array to read point information
    //task_t tasks[size];
    //printf("PathMax is: %llu\n", PATH_MAX);
    
  //  if (mpi_rank == 0) {
  	char* outPaths = malloc(sizeof(char) * (file_count * (PATH_LEN)));
    /** Initialize the filepath array **/
        //for (i = 0; i < size; ++i) {
        //    outPaths[i] = malloc(PATH_MAX + 1);
       // }
    /***************************************************
			TODO: BREAK OUT THE TASK DIVISION TO SEPARATE 
			FUNCTION
    ****************************************************/
    if (mpi_rank == 0) { 
		buildArray(dirname, outPaths, size);
    }
    int blockOffs[mpi_size];
    int blockSizes[mpi_size];
    divide_tasks(file_count, mpi_size, &blockOffs[0], &blockSizes[0]);
	for (i = 0; i < mpi_size; i++) {
		printf("Block offset: %i, size: %i\n", blockOffs[i], blockSizes[i]);
	}
    char* sub_paths = malloc(sizeof(char) * (blockSizes[mpi_rank] *  (PATH_LEN)));

    printf("Process: %d reading points\n", mpi_rank);
    /** Scatter the filepaths to be read **/
    printf("Scattering paths\n");
    //TODO: Fix this send/receive -> it is blocking itself
    if (mpi_rank == 0) {
    	int blockCounter;
    	// Send the data to the other processes
		for (i = 1; i < mpi_size; i++) {
			blockCounter = blockOffs[mpi_rank];
			printf("Sending paths to %i\n", i);
			mpi_err = MPI_Send(&outPaths[blockCounter], (blockSizes[mpi_rank] * PATH_LEN), MPI_CHAR, mpi_rank, 1, MPI_COMM_WORLD);
			MPI_check_error(mpi_err);
		}

		//for (i = 0; i < blockSizes[0]; i++) {
			
		//	sub_paths[i] = outPaths[(blockOffs[0] + i)];
		//}
	}
	if (mpi_rank > 0) {
		printf("[%i] Receiving %i paths\n", mpi_rank, blockSizes[mpi_rank]);
		mpi_err = MPI_Recv(&sub_paths, (blockSizes[mpi_rank] * PATH_LEN), MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
    //mpi_err = MPI_Scatter(outPaths, block * (PATH_MAX), MPI_CHAR, sub_paths, block * (PATH_MAX), MPI_CHAR, 0, MPI_COMM_WORLD);
		MPI_check_error(mpi_err);
	}
    printf("Creating tasks\n");
    /** TODO: Make processes read points **/
    header_t *headers;
    headers = malloc(sizeof(header_t) * blockSizes[mpi_rank]);
    printf("Allocated header memory\n");
/*    for (i = 0; i < block; i++){

        // Create the file reading tasks 
        //printf("Proc: %d, Path %i: %s\n", mpi_rank, i, outPaths[i*(PATH_MAX + 1)]);
        sleep(1);
        char* path;
        path = malloc(sizeof(char) * (PATH_MAX + 1));
        strncpy(path, &sub_paths[i * (PATH_MAX + 1)], (PATH_MAX + 1)); 
        printf("FilePath: %s\n", path);
        
        //taskType_Create(&tasks[i], path, 0, 1);
        printf("Task %d created in process %d\n", i, mpi_rank);

        totPoints = totPoints + tasks[i].size;
        //printf("Point count %d\n", totPoints);
    }*/
    /** Read the path values from the files in parallel, necessary due to speed of statting las files  **/
    hsize_t hOff = (hsize_t)blockOffs[mpi_rank];
    hsize_t hBlock = (hsize_t)blockSizes[mpi_rank];
	if (mpi_rank == 0) {
		readHeaderBlock(outPaths, 0, &hBlock, headers, mpi_rank);
	} else {
		readHeaderBlock(sub_paths, 0, &hBlock, headers, mpi_rank);
	}
    
    //printf("Loaded Header data, writing to file %s, dataset: %s\n", h5_file, "/headers");
    /** Write the header data to the HDF dataset **/
    printf("[%d] Offset is %zu\n", mpi_rank, (size_t)blockOffs[mpi_rank]);
    writeHeaderBlock(h5_file, "/headers", &hOff, &hBlock, headers, comm, info);
    /*for (i = 0; i < size; i++) 
    {
        taskType_Print(&tasks[i]);
        totPoints = totPoints + tasks[i].count;
        free(outPaths[i]);
        //taskType_Destroy(&tasks[i]);
    }*/
    //if (mpi_rank == 0) {
    //    for (i = 0; i < size; i++) {
    //        free(outPaths[i]);
    //    }
    //TODO: Write the tasks to HDF dataset 
    printf("Freeing memory\n");
    free(outPaths);
    free(sub_paths);
    free(headers);
   // free(pathBuf);
    //} else {
   //     for (i =0; i < block; i++) {
   //         free(sub_paths[i]);
   //     }
   //     free(sub_paths);
    //}
    printf("%zu Files, %zu Points\n", size, totPoints);
    //free(outPaths);
    printf("Process %d done.\n", mpi_rank);
    MPI_Finalize();
    return 0;
}
    
