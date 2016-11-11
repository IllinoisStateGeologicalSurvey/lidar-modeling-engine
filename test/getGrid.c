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
	//Only needed for parallel testing
	int mpi_size, mpi_rank, mpi_err;
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Info info = MPI_INFO_NULL;
	MPI_Status status;
	/** Init MPI **/
	MPI_Init(&argc, &argv);
	MPI_Comm_size(comm, &mpi_size);
	MPI_Comm_rank(comm, &mpi_rank);
	MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
	//char* h5_file = (char *)malloc(sizeof(char)* 4096);
	//getDataStore(h5_file);
	hid_t file_id, group_id;
	openLME(&file_id);
	parseArgs(argc, argv, &res[0], &res[1], &verbose);
	LMEcoord ll, ur;
	LMEcoord_set(&ll, -88.26681, 41.98540, -1000);
	LMEcoord_set(&ur, -88.25840, 41.99156, 5000);
	LMEbound bounds;
	LMEbound_set(&bounds, &ll, &ur);
	hsize_t rows = 5000;
	hsize_t cols = 5000;
	int dims[2] = {rows,cols};
	//printf("Opening LME dataset at %s\n", h5_file);
	LMEgrid* grid = Grid_Create(&bounds, dims, res);
	printf("Grid created.\n");
	group_id = H5Gopen(file_id, "grids", H5P_DEFAULT);
	//generateGridDataset(h5_file, "testGrid",&cols, &rows);
	LMEgrid_createDataset(file_id, grid);
	char LASfile = "/projects/isgs/lidar/mclean/LAS/882_1448.las";
	char pointSetName = "testSet";
	hsize_t pointBlock = 1000;
	
	LMEpointSet_writeFromLAS(LASfile, pointSetName, &pointBlock, group_id, comm, info);
	printf("Cleaning up\n");
	Grid_Destroy(grid);
	H5Gclose(group_id);
	H5Fclose(file_id);

	printf("Exiting\n");
	MPI_Finalize();
	return 0;
	//Bounds: -88.26681 41.98540 -88.25840 41.99156
	// Create a bound object
	//
	// Generate a grid with the bounds and resolution
	// populate the grid
	// write the grid to a file
	//

}
