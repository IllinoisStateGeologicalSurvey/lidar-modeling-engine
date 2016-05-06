/***********************************************************************
 *   Copyright (c) 2015 CyberGIS Center for Spatial Studies            *
 *                                                                     *
 *                                                                     *
 *                                                                     *
 **********************************************************************/

/**
 * @file bound.c
 * @author Nathan Casler
 * @date 6 May 2016
 * @brief File containing methods for the bound object
 *
 */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <hdf5.h>
#include "point.h"
#include "bound.h"
#include "common.h"

#include <math.h>
#include <inttypes.h>
#include <limits.h>
#include <proj_api.h>
#include <liblas/capi/liblas.h>

/* LASBound_Get: Retrieve a boundary object from a LAS file header (Note: The
 * bound object will be projected to EPSG:4326 and then offset/scaled
 * @param header: LASHeaderH* = Pointer to the header of the LAS file
 * @param bounds: bound_t* = Point to bound object to hold the file bounds
 * @return 1 if successful, else 0
 */
int LASBound_Get(LASHeaderH *header, bound_t* bounds) {

	//Need to get the projection from the LAS file
	double x[2],y[2],z[2];
	coord_dbl_t ll, ur;
	projPJ pj_las, pj_wgs;
	if (!LASProj_get(header, &pj_las)) {
		fprintf(stderr, "Projection Error: Failed to get projection from source\n");
		return 0;
	}
	
	x[0] = LASHeader_GetMinX(*header);
	x[1] = LASHeader_GetMaxX(*header);
	y[0] = LASHeader_GetMinY(*header);
	y[1] = LASHeader_GetMaxY(*header);
	z[0] = LASHeader_GetMinZ(*header);
	z[1] = LASHeader_GetMaxZ(*header);

	if (!Proj_load("+proj=longlat +ellps=WGS84 +datum=WGS84 +vunits=m +no_defs", &pj_wgs)) {
		fprintf(stderr, "Projection Error: Failed to initiate the WGS84 projection\n");
		return 0;
	}
	//printf("Target projection: %s\n", pj_get_def(pj_las, 0));
	//printf("Projecting bounds\n");
	pj_transform(pj_las, pj_wgs, 2, 1, &x[0], &y[0], &z[0]);
	int i= 0;
	//printf("Post-transform : (%f,%f,%f),(%f,%f,%f)\n", x[0],y[0],z[0],x[1],y[1],z[1]);
	for ( i =0; i < 2; i++) {
		x[i] *= RAD_TO_DEG;
		y[i] *= RAD_TO_DEG;
	}
	//printf("Degree Converted : (%f,%f,%f),(%f,%f,%f)\n", x[0],y[0],z[0],x[1],y[1],z[1]);
	Coord_Set(&ll, x[0], y[0], z[0]);
	Coord_Set(&ur, x[1], y[1], z[1]);
	Coord_Encode(&bounds->low, &ll);
	Coord_Encode(&bounds->high, &ur);

	//Bound_dbl_Set(bounds, x[0],y[0],z[0],x[1],y[1],z[1]);
	pj_free(pj_las);
	pj_free(pj_wgs);

	return 1;
}

/* Bound_intersects: Checks whether two boundary objects intersect eachother
 * @param bound_1: bound_t* = Pointer to first boundary object
 * @param bound_2: bound_t* = Pointer to second boundary object
 * @return 1 if intersection is true, else 0
 */
int Bound_intersects(bound_t* bound_1, bound_t* bound_2) {
	// Check if bound_1 is below bound_2
	if (bound_1->high.y < bound_2->low.y){
		//Bound 1 is below
		return 0;
	} else if (bound_1->high.x < bound_2->low.x) {
		// Bound 1 is left
		return 0;
	} else if (bound_1->low.y > bound_2->high.y) {
		// Bound 1 is above
		return 0;
	} else if (bound_1->low.x > bound_2->high.x) {
		// Bound 2 is right
		return 0;
	} else {
		printf("bounds intersect\n");
		return 1;
	}
	//if ((bound_1->high.x < bound_2->low.x) || (bound_1->high.y < bound_2->low.y)) {
		// Bound 2 is above/right of bound 1
	//	return 0;
	//} else if ((bound_2->high.x < bound_2->low.x) || (bound_2->high.y < bound_1->low.y)){
		// Bound 1 is above/right of bounds 2
	//	return 0;
	//	printf("Y Coords: check (%"PRIu32" <= %"PRIu32") and (%"PRIu32" <= %"PRIu32")\n", bound_1->low.y, bound_2->low.y, bound_1->high.y, bound_2->high.y);
	//	printf("bounds intersect\n");
	//	return 1;
//	}
	
}

/* BoundType_create: Create an H5 datatype representing the bound_t memory
 * footprint.
 * @param status: herr_t* = Pointer to the HDF5 error object
 * @return hid_t = The HDF5 id, used to reference the data type
 */

hid_t BoundType_create(herr_t *status) {
    hid_t boundtype;
    hid_t coordtype;
    coordtype = CoordType_create(status);
    boundtype = H5Tcreate(H5T_COMPOUND, sizeof(bound_t));
    *status = H5Tinsert(boundtype, "low", HOFFSET(bound_t, low), coordtype);
    *status = H5Tinsert(boundtype, "high", HOFFSET(bound_t, high), coordtype);
    *status = H5Tclose(coordtype);
	
    return boundtype;
}


/* BoundType_destroy: Closes the HDF5 reference to the HDF5 representation of
 * the bound_t datatype. (Note: Necessary for a proper close on the HDF5 file
 * @param boundtype: hid_t = HDF5 id holding the reference of the boundary
 * datatype
 * @param status: herr_t* = Pointer to the HDF5 error object
 */

void BoundType_destroy(hid_t boundtype, herr_t* status) {
    *status = H5Tclose(boundtype);
    
}

/* MPI_BoundType_create: Create a MPI datatype representing the memory footprint
 * of the bound_t data type (Note: nested datatypes cause compiling issues)
 * @param mpi_boundtype: MPI_Datatype* = Pointer to the MPI_Datatype that will
 * hold the definition
 * @return Int
 */

int MPI_BoundType_create(MPI_Datatype *mpi_boundtype) {
    int nitems=2;
    int blocklengths[2] = {1,1};
    MPI_Datatype types[2];
    // Set the member types
    MPI_CoordType_create(&types[0]);
    MPI_CoordType_create(&types[1]);
    MPI_Aint offsets[2];
    offsets[0] = offsetof(bound_t, low);
    offsets[1] = offsetof(bound_t, high);
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, mpi_boundtype);
    MPI_Type_commit(mpi_boundtype);
   // printf("Bound type created\n");
    
    return 0;
}

/* Bound_dbl_Set: Set the the boundary coordinates for a given bound_dbl_t
 * object
 * @param bounds: bound_dbl_t = Boundary object to assign the coordinates
 * @param minX: double = Minimum coordinate in on the X-axis(Left)
 * @param minY: double = Minimum coordinate on the Y-axis (Lower)
 * @param minZ: double = Minimum coordinate on the Z-axis (Bottom)
 * @param maxX: double = Maximum coordinate on the X-axis (Right)
 * @param maxY: double = Maximum coordinate on the Y-axis (Upper)
 * @param maxZ: double = Maximum coordinate on the Z-axis (Top)
 */
void Bound_dbl_Set(bound_dbl_t* bounds, double minX, double minY, double minZ, double maxX, double maxY, double maxZ) {
	Coord_Set(&bounds->low, minX, minY, minZ);
	Coord_Set(&bounds->high, maxX, maxY, maxZ);
}

/** Bound_dbl_Project: Projects a boundary object to a given CRS
 *  @param bound: bound_dbl_t = The bounds to project
 *  @param srs: LASSRSH = The desired projection for the bounds
 *  @return: Int 
 */
int Bound_dbl_Project(bound_dbl_t* bound, LASSRSH srs) {
	projPJ pj_las, pj_wgs;
	char* projStr = (char *)malloc(sizeof(char)*PATH_MAX);
	projStr = LASSRS_GetProj4(srs);
	if (!projStr) {
		printf("File has no projection definition\n");
		free(projStr);
		return 0;
	}
	if (!Proj_load(projStr,&pj_las)) {
		fprintf(stderr, "Failed to load projection from source\n");
		free(projStr);
		return 0;
	}
	double x[2],y[2],z[2];
	double cc[3];
	Coord_Get(cc, &bound->low);
	x[0] = cc[0] * DEG_TO_RAD;
	y[0] = cc[1] * DEG_TO_RAD;
	z[0] = cc[2] * DEG_TO_RAD;
	Coord_Get(cc, &bound->high);
	x[1] = cc[0] * DEG_TO_RAD;
	y[1] = cc[1] * DEG_TO_RAD;
	z[1] = cc[2] * DEG_TO_RAD;;

	if (!Proj_load("+proj=longlat +ellps=WGS84 +datum=WGS84 +vunits=m +no_defs", &pj_wgs)) {	
		fprintf(stderr, "Failed to load WGS84 projection\n");
		pj_free(pj_las);
		free(projStr);
		return 0;
	}
	//printf("Target projection: %s\n", pj_get_def(pj_las, 0));
	//printf("Projecting bounds\n");

	//printf("Pre-Test: Coords (%f,%f,%f)\n",cc[0],cc[1],cc[2]);
	pj_transform(pj_wgs, pj_las, 2, 1, &x[0], &y[0], &z[0]);

	//printf("Post-Test: Coords (%f,%f,%f)\n",x[1],y[1],z[1]);
	Bound_dbl_Set(bound, x[0],y[0],z[0],x[1],y[1],z[1]);

	//printf("New Bounds: (%f,%f), (%f,%f)\n", bound->low.x, bound->low.y, bound->high.x, bound->high.y);
	pj_free(pj_wgs);
	pj_free(pj_las);
	free(projStr);
	return 1;
	}
