/***************************************************************
 * Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  *
 * Information Laboratory (CIGI). All Rights Reserved.         *
 *                                                             *
 ***************************************************************/

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
int LMEboundCode_fromLAS(LMEBoundCode* bounds, const * LASHeaderH header) {

	//Need to get the projection from the LAS file
	double x[2],y[2],z[2];
	LMEcoord ll, ur;
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
	LMEcoord_Set(&ll, x[0], y[0], z[0]);
	LMEcoord_Set(&ur, x[1], y[1], z[1]);
	LMEcoord_encode(&bounds->low, &ll);
	LMEcoord_Encode(&bounds->high, &ur);

	//Bound_dbl_Set(bounds, x[0],y[0],z[0],x[1],y[1],z[1]);
	pj_free(pj_las);
	pj_free(pj_wgs);

	return 1;
}

void LMEbound_Set(LMEbound* bounds, const * LMEcoord low, const * LMEcoord high) {
	LMEcoord_Set(&bounds->low, LMEcoord_getX(low), LMEcoord_getY(low), LMEcoord_getZ(low));
	LMEcoord_Set(&bounds->high, LMEcoord_getX(high), LMEcoord_getY(high), LMEcoord_getZ(high));
}
/* @brief LMEbound_intersects: Checks whether two boundary objects intersect eachother
 * @param bound_1: (const LMEbound*) = Pointer to first boundary object
 * @param bound_2: (const LMEbound*) = Pointer to second boundary object
 * @return 1 if intersection is true, else 0
 */
int LMEbound_intersects(const * LMEbound bound_1, const * LMEbound bound_2) {
	// Check if bound_1 is below bound_2
	if (LMEcoord_getY(&bound_1->high) < LMEcoord_getY(&bound_2->low)){
		//Bound 1 is below
		return 0;
	} else if (LMEcoord_getX(&bound_1->high) < LMEcoord_getX(&bound_2->low)) {
		// Bound 1 is left
		return 0;
	} else if (LMEcoord_getY(&bound_1->low) > LMEcoord_getY(&bound_2->high)) {
		// Bound 1 is above
		return 0;
	} else if (LMEcoord_getX(&bound_1->low) > LMEcoord_getX(&bound_2->high)) {
		// Bound 2 is right
		return 0;
	} else {
		//printf("bounds intersect\n");
		return 1;
	}
	
}

/* @brief LMEboundCode_intersects: Checks whether two boundary objects intersect eachother
 * @param bound_1 (const * LMEboundCode):Pointer to first boundary object
 * @param bound_2 (const * LMEboundCode): Pointer to second boundary object
 * @return 1 if intersection is true, else 0
 */
int LMEboundCode_intersects(const * LMEboundCode  bound_1, const * LMEboundCode bound_2) {
	// Check if bound_1 is below bound_2
	if (LMEcoordCode_getY(&bound_1->high) < LMEcoordCode_getY(&bound_2->low)){
		//Bound 1 is below
		return 0;
	} else if (LMEcoordCode_getX(&bound_1->high) < LMEcoordCode_getX(&bound_2->low)) {
		// Bound 1 is left
		return 0;
	} else if (LMEcoordCode_getY(&bound_1->low) > LMEcoordCode_getY(&bound_2->high)) {
		// Bound 1 is above
		return 0;
	} else if (LMEcoordCode_getX(&bound_1->low) > LMEcoordCode_getX(&bound_2->high)) {
		// Bound 2 is right
		return 0;
	} else {
		//printf("bounds intersect\n");
		return 1;
	}
	
}

/**
 * @brief LMEboundCode_set: Sets the values for a coded boundary
 * @param code (LMEboundCode*): Boundary to set
 * @param low (const * LMEcoordCode): Low value encoded coordinate
 * @param high (const * LMEcoordCode): High value encoded coordinate
 */
void LMEboundCord_set(LMEboundCode* code, const * LMEcoordCode low, const * LMEcoordCode high) {
	LMEcoordCode_set(&code->low, LMEcoordCode_getX(low), LMEcoordCode_getY(low), LMEcoordCode_getZ(low));
	LMEcoordCode_set(&code->high, LMEcoordCode_getX(low), LMEcoordCode_getY(low), LMEcoordCode_getZ(low));
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
    *status = H5Tinsert(boundtype, "low", HOFFSET(LMEboundCode, low), coordtype);
    *status = H5Tinsert(boundtype, "high", HOFFSET(LMEboundCode, high), coordtype);
    *status = H5Tclose(coordtype);
	
    return boundtype;
}

/**
 * @brief Bound_Set: Create a bound object with the given extent
 * @param minX (double) = Minimum Longitude value
 * @param minY (double) = Minimum Latitude value
 * @param maxX (double) = Maximum Longitude value
 * @param maxY (double) = Maximum Latitude value
 * @return bound (bound_t) 
 * @note Assumes coordinates are in EPSG:4326 decimal degrees
 */
/*bound_t Bound_Set(double minX, double minY, double maxX, double maxY) {
	coord_dbl_t ll_raw, ur_raw;
	bound_t bound;
	Coord_Set(&ll_raw, minX, minY, 0.0);
	Coord_Set(&ur_raw, maxX, maxY, 0.0);
	Coord_Encode(&bound.low, &ll_raw);
	Coord_Encode(&bound.high, &ur_raw);
	return bound;
}
*/
/**
 * @brief LMEbound_length: will return the length of the bounday object in a 
 * given dimension. 
 * @param bound (bount_t*) The bound object to measure
 * @param dim (int) The index of the dimension to measure
 * @return length (double) of the absolute distance covered by the acis
 * @note currently assumes a flat surface, which is NOT THE BEST PRACTICE, 
 * NOR ACCURATE. SHOULD MIGRATE TO GREAT CIRCLE distance ASAP
 */
double LMEbound_length(LMEbound* bound, int dim) {
	if (dim = 0){
		return abs(bound->maxX - bound->minX);
	} else if (dim = 1) {
		return abs(bound->maxY - bound->minY);
	} else {
		fprintf(stderr, "Error: Given dimension is not defined for this dataset\n");
		return -1
	}
}
/**
 * @brief Bound_subdivide: will break the given bounds into a 2d array of child
 * bounds
 * @param bound (bound_t*): The bound object to be divided
 * @param countX (int): The number of blocks to create along the X-axis
 * @param countY (int): The number of blocks to create along the Y-axis
 * @param bound_count (size_t*): The length of the bound array returned
 * @note This should be altered soon to allow the child grids to reference the 
 * parent context or vice versa (something quadtree like)
 */
LMEbound* Bound_subdivide(LMEbound * bound, int countX, int countY, size_t* bound_count) {
	*bound_count = (size_t) countX * countY;
	int i,j;
	double stepX = Bound_length(bound, 0) / countX;
	double stepY = Bound_length(bound, 1) / countY;
	LMEbound boundArr [countX][countY];
	for (i = 0; i < countX-1; i++) {
		for (j = 0; j < countY-1; j++) {
			// calculate new min max for the bounds
			double newMinX = bound->minX + (stepX * i);
			double newMinY = bound->minY + (stepY * j);
			double newMaxX = bound->minX + (stepX * (i+1));
			double newMaxY = bound->minY + (stepY * (j+1));
			boundArr[i][j] = LMEbound_Set(newMinX,newMinY, LMEcoord_getZ(&bound->low), newMaxX, newMaxY, LMEcoord_getZ(&bound->high));

		}
	}
	return &boundArr;
}

LMEcoord LMEbound_Centroid(const * LMEbound bounds) {
	double x,y,z;
	x = (LMEcoord_getX(&bounds->ur) - LMEcoord_getX(&bounds->ll)) / 2;
	y = (LMEcoord_getY(&bounds->ur) - LMEcoord_getY(&bounds->ll)) / 2;
	z = (LMEcoord_getZ(&bounds->ur) - LMEcoord_getZ(&bounds->ll)) / 2;
	LMEcoord coord;
	LMEcoord_set(&coord, x, y, z);
	return coord;
}

int LMEbound_fromLAS(LMEbound* bounds, const * LASHeader header) {
	LMEcoord ll, ur;
	LMEcoord_set(&ll, LASHeader_getMinX(*header), LASHeader_getMinY(*header), LASHeader_getMinZ(*header));
	LMEcoord_set(&ur, LASHeader_getMaxX(*header), LASHeader_getMaxY(*header), LASHeader_getMaxZ(*header));
	LMEbound_set(&bounds, &ll, &ur);
	return 1;
}
	
	// TODO: Parameterize for different projections}
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
/*
void Bound_dbl_Set(bound_dbl_t* bounds, double minX, double minY, double minZ, double maxX, double maxY, double maxZ) {
	Coord_Set(&bounds->low, minX, minY, minZ);
	Coord_Set(&bounds->high, maxX, maxY, maxZ);
}
*/
int LMEbound_project(LMEbound* bound, char* srcPrj, char* dstPrj) {
	projPJ pj_src, pj_dst;
	LMEcoord low, high;
	if (!Proj_load(srcProj, &pj_src)) {
		fprintf(stderr, "Failed to load source projection definition\n");
		return 0;
	}
	if (!Proj_load(projStr,&pj_las)) {
		fprintf(stderr, "Failed to load projection from source\n");
		return 0;
	}
	double x[2],y[2],z[2];
	double cc[3];
	if (pj_is_latlong(pj_src) && !pj_is_latlong(pj_dst)) {
		LMEcoord_toRadians(&bound->low);
		LMEcoord_toRadians(&bound->high);
	}
	LMEcoord_get(cc, &bound->low);
	//Coord_Get(cc, &bound->low);
	x[0] = cc[0];
	y[0] = cc[1];
	z[0] = cc[2];
	LMEcoord_get(cc, &bound->high);
	x[1] = cc[0];
	y[1] = cc[1];
	z[1] = cc[2];

	//printf("Target projection: %s\n", pj_get_def(pj_las, 0));
	//printf("Projecting bounds\n");

	//printf("Pre-Test: Coords (%f,%f,%f)\n",cc[0],cc[1],cc[2]);
	pj_transform(pj_src, pj_dst, 2, 1, &x[0], &y[0], &z[0]);

	//printf("Post-Test: Coords (%f,%f,%f)\n",x[1],y[1],z[1]);
	LMEcoord_set(&low, x[0],y[0],z[0]);
	LMEcoord_set(&high, x[1],y[1],z[1]);
	LMEbound_Set(bound, &low, &high);

	//printf("New Bounds: (%f,%f), (%f,%f)\n", bound->low.x, bound->low.y, bound->high.x, bound->high.y);
	pj_free(pj_src);
	pj_free(pj_dst);
	
	return 1;
	}
/** LMEbound_fromLAS: Projects a boundary object to a given CRS
 *  @param bound: bound_dbl_t = The bounds to project
 *  @param srs: LASSRSH = The desired projection for the bounds
 *  @return: Int 
 */
int LMEbound_fromLAS(LMEbound* bound, LASSRSH srs) {
	projPJ pj_las, pj_wgs;
	LMEcoord low, high;
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
	LMEcoord_toRadians(&bound->low);
	LMEcoord_get(cc, &bound->low);

	LMEcoord_toRadians(&bound->high);
	Coord_Get(cc, &bound->high);

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
	LMEcoord_set(&low, x[0], y[0], z[0]);
	LMEcoord_set(&high, x[1], y[1], z[1]);

	LMEbound_set(bound, &low, &high);

	//printf("New Bounds: (%f,%f), (%f,%f)\n", bound->low.x, bound->low.y, bound->high.x, bound->high.y);
	pj_free(pj_wgs);
	pj_free(pj_las);
	free(projStr);
	return 1;
	}
