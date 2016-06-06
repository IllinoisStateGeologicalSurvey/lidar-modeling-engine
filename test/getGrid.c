/** 
 * @file getGrid.c
 * @author Nathan Casler
 * @date May 9 2016
 * @brief Tests the grid type creation functionality
 */

#include <stdio.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_matrix.h>
#include <mpi.h>
#include "util.h"
#include "bound.h"
#include "grid.h"

void usage()
{
	fprintf(stderr, "------------------------------------------------------\n");
	fprintf(stderr, "                getGrid   usage:\n");
	fprintf(stderr, "\n");
	fprintf(stderr, " getGrid -r <resX> <resY>\n");
	fprintf(stderr, "------------------------------------------------------\n");
}

void parseArgs(int argc, char* argv[], double* resX, double* resY, int* verbose) {
	int i;


	for (i = 1; i < argc; i++) 
	{
		if (strcmp(argv[i], "-h") == 0 ||
				strcmp(argv[i], "--help") == 0)
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
		else if (strcmp(argv[i], "-r") == 0 ||
				strcmp(argv[i], "--res") == 0)
		{
			sscanf(argv[i+1], "%lf", resX);
			sscanf(argv[i+2], "%lf", resY);
			i = i+2;
		}
		else
		{
			fprintf(stderr, "ERROR: unknown argument '%s'\n", argv[i]);
			usage();
			exit(1);
		}
	}
}

int main(int argc, char* argv[]) {

	double res[2] = {0.0,0.0};
	int verbose;
	char* h5_file = (char *)malloc(sizeof(char)* 4096);
	getDataStore(h5_file);
	parseArgs(argc, argv, &res[0], &res[1], &verbose);
	LMEcoord ll, ur;
	LMEcoord_set(&ll, -88.26681, 41.98540, -1000);
	LMEcoord_set(&ur, -88.25840, 41.99156, 5000);
	LMEbound bounds;
	LMEbound_set(&bounds, &ll, &ur);
	int dims[2] = {1000,1000};
	LMEgrid* grid = Grid_Create(&bounds, dims, res);

	printf("Grid created.\n");
	printf("Cleaning up\n");
	Grid_Destroy(grid);
	free(h5_file);
	printf("Exiting\n");
	return 0;
	//Bounds: -88.26681 41.98540 -88.25840 41.99156
	// Create a bound object
	//
	// Generate a grid with the bounds and resolution
	// populate the grid
	// write the grid to a file
	//

}
