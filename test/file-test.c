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
	fprintf(stderr, "			   fileTest usage:\n");
	fprintf(stderr, "\n");
	fprintf(stderr, " fileTest -i <path-to-las-files> -n <number-to-read> -f <hdf5-file>\n");
	fprintf(stderr, "----------------------------------------------\n");
}


void parseArgs(int argc, char* argv[], char *lasPath, char* h5_file, int* verbose)
{
	int i;
	/* CHeck the number of processors */
	for ( i = 1; i < argc; i++)
	{
		if (	strcmp(argv[i], "-h") == 0 ||
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
	hid_t file_id, plist_id;
	size_t size;
	int i;
	int verbose;
	size_t totPoints = 0;
	header_t *headers;
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
	//Used for task division
	int blockOffs[mpi_size];
	int blockSizes[mpi_size];
	/* Check supplied arguments */
	parseArgs(argc, argv, &dirname[0], &h5_file[0], &verbose);
	printf("Dirname: %s\n", dirname);
	int file_count = countLAS(dirname);
	hsize_t dims = (size_t)file_count;
	/** Mpi task division **/	 


	/* Root process creates dataset */

	if (mpi_rank == 0) {
		printf("Need to read %i files\n", file_count);
		createHeaderDataset(h5_file, "/headers", &dims);
		printf("created HDF dataset at %s\n", h5_file);
	}

	/***************************************************
TODO: BREAK OUT THE TASK DIVISION TO SEPARATE 
FUNCTION
	 ****************************************************/
	//divide_tasks(file_count, mpi_size, &blockOffs[0], &blockSizes[0]);
	//char* sub_paths = malloc(sizeof(char) * (blockSizes[mpi_rank] *  (PATH_LEN)));

	/** Scatter the filepaths to be read **/
	printf("Scattering paths\n");
	//TODO: Fix this send/receive -> it is blocking itself
	MPI_Barrier(comm);
	if (mpi_rank == 0) {
		int blockCounter;
		// Send the data to the other processes
		char* outPaths = malloc(sizeof(char) * ((size_t)file_count * PATH_LEN));
                header_t* headers = malloc(sizeof(header_t) * file_count);
                
		buildArray(dirname, outPaths, file_count);
                readHeaderBlock(&outPaths[0], 0, file_count, headers, comm, mpi_rank);
                free(outPaths);
/*                for (i = 1; i < mpi_size; i++) {
			blockCounter = blockOffs[i];
			printf("Sending %i paths starting from idx: %i %s to %i\n", blockSizes[i], blockCounter, &outPaths[blockCounter * PATH_LEN], i);
			mpi_err = MPI_Send(&outPaths[blockCounter * PATH_LEN], (blockSizes[i] * PATH_LEN), MPI_CHAR, i, 1, comm);
			MPI_check_error(mpi_err);
			printf("Sent %i paths starting from idx: %i to %i\n", blockSizes[i], blockCounter, i);
		}

		for (i = 0; i < blockSizes[0]; i++) {

			sub_paths[i] = outPaths[(blockOffs[0] + i)];
                }
		free(outPaths);
	} else {
		printf("[%i] Receiving %i paths\n", mpi_rank, blockSizes[mpi_rank]);
		mpi_err = MPI_Recv(&sub_paths[0], (blockSizes[mpi_rank] * PATH_LEN), MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
		MPI_check_error(mpi_err);
	}
	printf("[%i] Creating tasks\n", mpi_rank);
	// TODO: Make processes read points 
	headers = malloc(sizeof(header_t) * blockSizes[mpi_rank]);
	printf("[%i] Allocated memory for %i headers\n", mpi_rank, blockSizes[mpi_rank]);

	// Read the path values from the files in parallel, necessary due to speed of statting las files  
	hsize_t hOff = (hsize_t)blockOffs[mpi_rank];
	hsize_t hBlock = (hsize_t)blockSizes[mpi_rank];
	// TODO: This is exiting when 3/4 done, need to debug
	readHeaderBlock(&sub_paths[0], hOff, &hBlock, headers, comm, mpi_rank);

	printf("[%i] Headers Read, Writing...\n", mpi_rank); */
	//printf("Loaded Header data, writing to file %s, dataset: %s\n", h5_file, "/headers");
	/** Write the header data to the HDF dataset **/
            hsize_t hBlock = file_count;
            hsize_t hOff = 0; 
            //printf("[%d] Offset is %zu\n", mpi_rank, (size_t)blockOffs[mpi_rank]);
	    //MPI_Barrier(comm);
	    plist_id = H5Pcreate(H5P_FILE_ACCESS);
	    H5Pset_fapl_mpio(plist_id, comm, info);

	    file_id = H5Fopen(h5_file, H5F_ACC_RDWR | H5F_ACC_DEBUG, plist_id);

	    writeHeaderBlock(file_id, "/headers", &hOff, &hBlock, headers, comm, info);

	    MPI_Barrier(comm);
	printf("Freeing memory\n");
	H5Pclose(plist_id);
	H5Fclose(file_id);
	//TODO: Write the tasks to HDF dataset 
	free(sub_paths);
	free(headers);

	//printf("%zu Files, %zu Points\n", size, totPoints);
	//free(outPaths);
	printf("Process %d done.\n", mpi_rank);
	MPI_Finalize();
	return 0;
}

