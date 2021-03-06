/***************************************************************
 * Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  *
 * Information Laboratory (CIGI). All Rights Reserved.         *
 *                                                             *
 ***************************************************************/

/** 
 * @file readPoints.c
 * @author Nathan Casler
 * @date May 6 2016
 * @brief Test will read points from a list of LAS files using filter
 * parameters, point data is then saved to HDF5
 *
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

void parse_args(int argc, char* argv[], char* file_list, LMEbound* bounds, int* verbose)  {
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
			LMEcoord ll, ur;
			for (j = 0; j < 4; j++) {
				int counter = j+i;
				sscanf(argv[counter], "%lf", &coords[j]);
			}
			LMEcoord_set(&ll, coords[0], coords[1], 0.0);
			LMEcoord_set(&ur, coords[2], coords[3], 0.0);
			LMEbound_set(bounds, &ll, &ur);
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
	char* file_list = (char *)malloc(sizeof(char) * PATH_SIZE);
	LMEbound* bounds =  (LMEbound *)malloc(sizeof(LMEbound));

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
	char line[PATH_SIZE];
	LASReaderH reader;
	LASHeaderH header;
	LASSRSH srs;
	uint32_t pntCount;
	int fileCount = 0;
	//Create filter object
	LMEfilter* filter = (LMEfilter *)malloc(sizeof(LMEfilter));
	LMEcrs srcCRS, dstCRS;
	LMEcrs_setWGS(&srcCRS);
	LMEfilter_create(filter);
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
	char paths[fileCount][PATH_SIZE];
	fseek(fp, 0, SEEK_SET);
	for (i = 0; i < fileCount; i++) {
		fgets(line, sizeof(line), fp);
		// Remove trailing new lines
		resolvePath(line, 1);
		strcpy(paths[i], line);
	}
	fclose(fp);
	printf("[%i] File Count: %i\n", mpi_rank, fileCount);


	printf("(%f, %f, %f), (%f,%f,%f)\n", LMEcoord_getX(&bounds->low), LMEcoord_getY(&bounds->low), LMEcoord_getZ(&bounds->low), LMEcoord_getX(&bounds->high), LMEcoord_getY(&bounds->high), LMEcoord_getZ(&bounds->high));
	//Filter_SetRange(filter, bounds);
	// Get the task division
	starttime = MPI_Wtime();
	divide_tasks(fileCount, mpi_size, &t_offsets[0], &t_blocks[0]);	
	uint32_t bufCount = 0, totPoints = 0;
	for (i = 0; i < t_blocks[mpi_rank]; i++) {
		int counter = t_offsets[mpi_rank] + i;
		LMEfilter_setRange(filter, bounds);
		LMEfilter_setReturn(filter, 2);
		printf("[%i] Reading %s\n", mpi_rank, paths[counter]);
		openLAS(&reader,&header, &srs, &pntCount, paths[counter]);
		LMEpointCode* points = (LMEpointCode *)malloc(sizeof(LMEpointCode) * pntCount);
		// Put the bounds into LAS file's projection
		LMEcrs_fromLAS(&dstCRS, &header);
		LMEbound_project(&filter->range, &srcCRS, &dstCRS);
		//printf("File has Bounds: (%f,%f,%f,%f)\n",LASHeader_GetMinX(header), LASHeader_GetMinY(header), LASHeader_GetMaxX(header), LASHeader_GetMaxY(header));
		//printf("FilterBounds: (%f,%f,%f,%f)\n",filter->range.low.x,filter->range.low.y,filter->range.high.x,filter->range.high.y);
		printf("[%i] Filtering file\n", mpi_rank);
		bufCount = filterLAS(&header, &reader, &pntCount, filter, points, mpi_rank);
		// Need to add the points to something
		// Can write to HDF, but will require a collective call. 
		// Could also call a write after the number of points hits a certain
		// number to protect the heap. 
		// TODO: NEED TO IMPLEMENT A THRESHOLD FOR THE NUMBER OF POINTS A PROCESS CAN
		// HANDLE, it looks like the process is choking on a buffer of 1325556
		// points
		if (bufCount > 0) {	
			totPoints = bufCount + totPoints;
			printf("[%i] Found %"PRIu32" points to process\n", mpi_rank, bufCount);
			printf("[%i] POint [%i] Is (%d,%d,%d)\n", mpi_rank, 1, LMEcoordCode_getX(&points[1].code), LMEcoordCode_getY(&points[1].code), LMEcoordCode_getZ(&points[1].code));
		}
			// TODO: Find method to aggregate points for writing without
			// overflowing buffer
		closeLAS(&reader, &header, &srs, &pntCount, points);
		endtime = MPI_Wtime();
		free(points);
		printf("[%i] File processed in %f seconds\n", mpi_rank, endtime-starttime);
	}
	printf("[%i] Total points to process: %"PRIu32" \n", mpi_rank, totPoints);
	endtime = MPI_Wtime();
	printf("[%i] Process finished in %f seconds\n", mpi_rank, endtime-starttime);
	printf("[%i] Cleaning up\n", mpi_rank);
	LMEfilter_destroy(filter);
	free(bounds);
	free(file_list);
	MPI_Finalize();
	return 0;
}
