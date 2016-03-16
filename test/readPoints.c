/*****************************************
 *  readPoints.c
 *
 *  This utility will readpoints from a list of
 *  LAS files using a list of parameters to filter the
 *  data. The data will then be saved to HDF5
 */


#include <time.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <mpi.h>
#include <liblas/capi/liblas.h>
#include <proj_api.h>
#include "common.h"
#include "point.h"
#include "file_util.h"
#include "util.h"
#include "header.h"
#include "reader.h"
#include "filter.h"
#define LAS_FORMAT_10 0
#define LAS_FORMAT_11 1
#define LAS_FORMAT_12 2

/* TODO: Setup format to write out filters
 */

void usage(){
	fprintf(stderr, "---------------------------------------------------\n");
	fprintf(stderr, "        readPoints usage: \n");
	fprintf(stderr, "\n");
	fprintf(stderr, " readPoints -i <LAS-file-list> \n");
	fprintf(stderr, "\n");
	fprintf(stderr, "---------------------------------------------------\n");
}

void ptime(const char *const msg)
{
	float t = ((float)clock())/CLOCKS_PER_SEC;
	fprintf(stderr, "Cumulative CPU time thru %s = %f\n", msg, t);
}

void parse_args(int argc, char* argv[], char* file_list, bound_dbl_t* bounds, int* verbose)  {
	int i;
	for (i = 1; i < argc; i++) {
		if	(		strcmp(argv[i], "-h") == 0 ||
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

		else if (strcmp(argv[i], "--input") == 0 ||
				strcmp(argv[i], "-input") == 0 ||
				strcmp(argv[i], "-i") == 0 
			)
		{
			i++;
			strcpy(file_list, argv[i]);
		}
		else if (strcmp(argv[i], "--bounds") == 0 ||
				strcmp(argv[i], "-b") == 0) 
		{
			i++;
			double coords[4];
			int j = 0;
			for (j = 0; j < 4; j++) {
				int counter = j+i;
				sscanf(argv[counter], "%lf", &coords[j]);
			}
			Bound_dbl_Set(bounds, coords[0], coords[1], 0.0, coords[2], coords[3], 0.0);
			i = i+4;
		} else 
		{
			fprintf(stderr, "ERROR: Unknown Argument '%s'\n", argv[i]);
			usage();
			exit(1);
		}
	}
}

int main(int argc, char* argv[])
{
	int i, j;
	int verbose = FALSE;
	char* file_list = (char *)malloc(sizeof(char) * PATH_LEN);
	bound_dbl_t* bounds =  (bound_dbl_t *)malloc(sizeof(bound_dbl_t));

	// MPI Related variables
	int mpi_size, mpi_rank, mpi_err;
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Info info = MPI_INFO_NULL;

	// Initializa MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_size(comm, &mpi_size);
	MPI_Comm_rank(comm, &mpi_rank);
	
	// Variables for task division
	int t_offsets[mpi_size];
	int t_blocks[mpi_size];
	
	// Timing objects
	double starttime, endtime;

	parse_args(argc, argv, file_list, bounds, &verbose);
	char line[PATH_LEN];
	LASReaderH reader;
	LASHeaderH header;
	LASSRSH srs;
	uint32_t pntCount;
	int fileCount = 0;
	//Create filter object
	filter_t* filter = Filter_Create();
	// Open task file
	FILE* fp = fopen(file_list, "r");
	while(!feof(fp)){
		char ch;
		ch = fgetc(fp);
		if (ch == '\n')
		{	
			fileCount++;
		}
	}
	printf("Reading %i files\n", fileCount);
	char paths[fileCount][PATH_LEN];
	fseek(fp, 0, SEEK_SET);
	for (i = 0; i < fileCount; i++) {
		fgets(line, sizeof(line), fp);
		// Remove trailing new lines
		resolvePath(line, 1);
		strcpy(paths[i], line);
	}
	fclose(fp);
	printf("[%i] File Count: %i\n", mpi_rank, fileCount);


	printf("(%f, %f, %f), (%f,%f,%f)\n", bounds->low.x, bounds->low.y, bounds->low.z, bounds->high.x, bounds->high.y, bounds->high.z);
	//Filter_SetRange(filter, bounds);
	// Get the task division
	starttime = MPI_Wtime();
	divide_tasks(fileCount, mpi_size, &t_offsets[0], &t_blocks[0]);	
	uint32_t bufCount = 0, totPoints = 0;
	for (i = 0; i < t_blocks[mpi_rank]; i++) {
		int counter = t_offsets[mpi_rank] + i;
		Filter_SetRange(filter, bounds);
		Filter_SetReturn(filter, 2);
		printf("[%i] Reading %s\n", mpi_rank, paths[counter]);
		openLAS(&reader,&header, &srs, &pntCount, paths[counter]);
		Bound_dbl_Project(&filter->range, srs);
		//printf("File has Bounds: (%f,%f,%f,%f)\n",LASHeader_GetMinX(header), LASHeader_GetMinY(header), LASHeader_GetMaxX(header), LASHeader_GetMaxY(header));
		//printf("FilterBounds: (%f,%f,%f,%f)\n",filter->range.low.x,filter->range.low.y,filter->range.high.x,filter->range.high.y);
		bufCount = filterLAS(&reader, &pntCount, filter);
		totPoints = bufCount + totPoints;
		printf("[%i] Found %"PRIu32" points to process\n", mpi_rank, bufCount);;

		closeLAS(&reader, &header, &srs, &pntCount);
		endtime = MPI_Wtime();
		printf("[%i] File processed in %f seconds\n", mpi_rank, endtime-starttime);
	}
	printf("[%i] Total points to process: %"PRIu32" \n", mpi_rank, totPoints);
	endtime = MPI_Wtime();
	printf("[%i] Process finished in %f seconds\n", mpi_rank, endtime-starttime);
	printf("[%i] Cleaning up\n", mpi_rank);
	Filter_Destroy(filter);
	free(bounds);
	free(file_list);
	MPI_Finalize();
	return 0;
}
