#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <header.h>
#include <reader.h>
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
	fprintf(stderr, " rangeTest -b1 <minX> <minY> <maxX> <maxY> -f <HDF5 File>\n");
	fprintf(stderr, "-----------------------------------------------------------\n");
}

void parseArgs(int argc, char* argv[], bound_t* bound_1, char* h5_file, int* verbose) {
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
			printf("Found bound\n");
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
		}
		else if (strcmp(argv[i], "-f") == 0 ||
				strcmp(argv[i], "--file") == 0
				)
		{
			i++;
			strcpy(h5_file, argv[i]);
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
	bound_t bound_1;
	int i, intcount = 0;
	uint32_t pntCounter = 0;
	int verbose;
	char h5_file[PATH_MAX];
	parseArgs(argc, argv, &bound_1, &h5_file[0], &verbose);
	hid_t file_id, plist_id;
	hsize_t nHeaders;
	header_t* headers;
	plist_id = H5Pcreate(H5P_FILE_ACCESS);
	file_id = H5Fopen(h5_file, H5F_ACC_RDONLY | H5F_ACC_DEBUG, plist_id);
	nHeaders = Headers_count(file_id);
	headers = malloc(sizeof(header_t)*(int)nHeaders);
	Headers_read(headers, file_id);	
	for (i=0; i < nHeaders; i++) {
		if (intersects(&bound_1, &headers[i].bounds)) {
			printf("Header [%i] intersects, and has %i pts\n", i, headers[i].pnt_count);
			intcount++;
			pntCounter = pntCounter + headers[i].pnt_count;
		}
	}
	printf("Need to read %i files with %i for surface\n", intcount, pntCounter);
	free(headers);
	H5Pclose(plist_id);
	H5Fclose(file_id);
	//bound_t iBounds;
	/*
	if (intersects(&bound_1, &bound_2)) {
		printf("Bounds intersect\n");
	} else {
		printf("Bound do not intersect\n");
	}
	*/
	return 0;
}



