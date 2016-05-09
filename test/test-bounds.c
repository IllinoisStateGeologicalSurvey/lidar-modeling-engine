/***************************************************************
 * Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  *
 * Information Laboratory (CIGI). All Rights Reserved.         *
 *                                                             *
 ***************************************************************/

/** 
 * @file test-bounds.c
 * @author Nathan Casler
 * @date May 6 2016
 * @brief Deprecated: Test to check if bbox queries are functional 
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <header.h>
#include <proj_api.h>
#include <hdf5.h>
#include <mpi.h>
#include <string.h>

int intersects(bound_t* bound_1, bound_t* bound_2) {
	if ((bound_1->high.x < bound_2->low.x) || (bound_1->high.y < bound_2->low.y)) {
		// Bound 2 is above/right of bound 1
		return 0;
	} else if ((bound_2->high.x < bound_2->low.x) || (bound_2->high.y < bound_1->low.y)){
		// Bound 1 is above/right of bounds 2
		return 0;
	} else {
		printf("bounds intersect\n");
		return 1;
	}
	
}

void usage() 
{
	fprintf(stderr, "-----------------------------------------------------------\n");
	fprintf(stderr, "                     rangeTest   usage:\n");
	fprintf(stderr, "\n");
	fprintf(stderr, " rangeTest -b1 minX minY maxX maxY -b2 minX minY maxX maxY\n");
	fprintf(stderr, "-----------------------------------------------------------\n");
}

void parseArgs(int argc, char* argv[], bound_t* bound_1, bound_t* bound_2, int* verbose) {
	int i,j;
	/* Check the number of processors */
	coord_dbl_t*  ll_raw = malloc(sizeof(coord_dbl_t));
	coord_dbl_t*   ur_raw = malloc(sizeof(coord_dbl_t));
	

	for (i = 1; i < argc; i++)
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
		else if (strcmp(argv[i], "-b1") == 0 ||
				strcmp(argv[i], "--bound1") == 0
				)
		{
			printf("Found first bound, i= %i\n", i);
			double x,y;

			i++;
			j = i+1;
			sscanf(argv[i], "%lf", &x);
			sscanf(argv[j], "%lf", &y);
			printf("Setting coords to %f,%f,%f\n", x, y,0.0);
			Coord_Set(ll_raw, x, y, 0.0);
			i = i+2;
			j = i+1;
			sscanf(argv[i], "%lf", &x);
			sscanf(argv[j], "%lf", &y);
			printf("Set low coord to %f,%f\n", ll_raw->x, ll_raw->y);
			Coord_Set(ur_raw, x, y, 0.0);
			printf("Coordinates set for first bound, encoding\n");
			Coord_Encode(&bound_1->low, ll_raw);
			Coord_Encode(&bound_1->high, ur_raw);
			i = i+1;
			printf("Continuing to parse, i=%i\n", i);
			printf("Arg is %s\n", argv[i]);
		}
		else if (strcmp(argv[i], "-b2") == 0 ||
				strcmp(argv[i], "--bound2") == 0
				)
		{
			printf("Found seconds bound\n");
			double x,y;

			i++;
			j = i+1;
			sscanf(argv[i], "%lf", &x);
			sscanf(argv[j], "%lf", &y);
			printf("Setting coords to %f,%f,%f\n", x, y,0.0);
			Coord_Set(ll_raw, x, y, 0.0);
			i = i+2;
			j = i+1;
			sscanf(argv[i], "%lf", &x);
			sscanf(argv[j], "%lf", &y);
			printf("Set low coord to %f,%f\n", ll_raw->x, ll_raw->y);
			Coord_Set(ur_raw, x, y, 0.0);
			printf("Coordinates set for second bound, encoding\n");
			Coord_Encode(&bound_1->low, ll_raw);
			Coord_Encode(&bound_1->high, ur_raw);
			i = i+1;
			printf("Continuing to parse, i=%i\n", i);
		}
		else
		{
			fprintf(stderr, "ERROR: unknown argument '%s'\n", argv[i]);
			usage();
			exit(1);
		}
	}
	free(ll_raw);
	free(ur_raw);
}


int main(int argc, char* argv[]) {
	//Accept a range of coordinates , test for intersection 
	bound_t bound_1, bound_2;
	int verbose;

	parseArgs(argc, argv, &bound_1, &bound_2, &verbose);

	//bound_t iBounds;
	if (intersects(&bound_1, &bound_2)) {
		printf("Bounds intersect\n");
	} else {
		printf("Bound do not intersect\n");
	}
	
	return 0;
}



