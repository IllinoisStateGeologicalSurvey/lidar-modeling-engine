

/**
 * @file coord.c
 * @author Nathan Casler <ncasler@illinois.edu>
 * @date May 10 2016
 * @brief Methods for manipulating 3-dimensional coordinates
 */
#include <stdlib.h>
#include <stdio.h>
#include <liblas/capi/liblas.h>
#include <hdf5.h>
#include <math.h>
#include <proj_api.h>
#include "coord.h"

#define COORD_RANK 1
#define COORD_DIM 3
//const double offsets[3] = {180.0, 90.0, 12300.0};
//const double max = (double)UINT_MAX;
/** @brief Scale_init: Initialize the scale used to encode the coordinates
 *  @param offset (double) The offsets needed to make the range of values n>0
 *  @param max (double) The maximum value allowed in output range
 *  @return scale (double) value to multiply coordinates in n dimension against
 *  The scales are used to put each of the coordinate values in the range
 *  0<n<UINT_MAX. This allows for hilbert encoding and 32bit storage of precise
 *  values. This could be changed in the future if more than 32bits of precision
 *  are required.
 */
double Scale_init(double offset, double max)
{
	double range = 2.0 * offset;
	double scale = max / range;
	return scale;

}

uint32_t LMEcoordCode_getX(LMEcoordCode * const coord) {
	return coord->dims[0];
}

uint32_t LMEcoordCode_getY(LMEcoordCode * const coord) {
	return coord->dims[1];
}

uint32_t LMEcoordCode_getZ(LMEcoordCode * const coord) {
	return coord->dims[2];
}



void LMEcoordCode_setX(LMEcoordCode* coord, uint32_t x) {
	coord->dims[0] = x;
}

void LMEcoordCode_setY(LMEcoordCode* coord, uint32_t y) {
	coord->dims[1] = y;
}

void LMEcoordCode_setZ(LMEcoordCode* coord, uint32_t z) {
	coord->dims[2] = z;
}

void LMEcoordCode_set(LMEcoordCode* coord, uint32_t x, uint32_t y, uint32_t z) {
	coord->dims[0] = x;
	coord->dims[1] = y;
	coord->dims[2] = z;
}

void LMEcoordCode_get(uint32_t* dims, LMEcoordCode * const coord) {
	dims[0] = LMEcoordCode_getX(coord);
	dims[1] = LMEcoordCode_getY(coord);
	dims[2] = LMEcoordCode_getZ(coord);
}

double LMEcoord_getX(LMEcoord * const coord) {
	return coord->dims[0];
}

double LMEcoord_getY(LMEcoord * const coord) {
	return coord->dims[1];
}

double LMEcoord_getZ(LMEcoord * const coord) {
	return coord->dims[2];
}

void LMEcoord_setX(LMEcoord* coord, double x) {
	coord->dims[0] = x;
}

void LMEcoord_setY(LMEcoord* coord, double y) {
	coord->dims[1] = y;
}

void LMEcoord_setZ(LMEcoord* coord, double z) {
	coord->dims[2] = z;
}

double LMEcoord_distance(LMEcoord* coord1, LMEcoord* coord2) {
	double delta[3];
	int i = 0;
	double dist;
	for (i = 0; i < 3; i++) {
		delta[i] = pow(coord2->dims[i] - coord1->dims[i], 2);
	}
	dist = sqrt(delta[0] + delta[1] + delta[2]);
	return dist;
}

/**
 * @brief LMEcoord_set: Set all dims to a coordinate at once
 * @param coord (LMEcoord*) The coordinate to update
 * @param x (double) The location along the x-axis
 * @param y (double) The location along the y-axis
 * @param z (double) The location along the z-axis
 * @note Not sure if this is more efficient than calling setters separately,
 * should check...
 */
void LMEcoord_set(LMEcoord* coord, double x, double y, double z) {
	coord->dims[0] = x;
	coord->dims[1] = y;
	coord->dims[2] = z;
}
/**
 * @brief LMEcoord_copy: Create a copy of a coordinate
 * @param: dstCoord (LMEcoord*) Destination coordinate
 * @param: srcCoord (LMEcoord*) Source coordinate
 * @return 0
 */
int LMEcoord_copy(LMEcoord* dstCoord, LMEcoord * const srcCoord) {
	LMEcoord_set(dstCoord, LMEcoord_getX(srcCoord), LMEcoord_getY(srcCoord), LMEcoord_getZ(srcCoord));
	return 0;
}

void LMEcoord_print(LMEcoord* const coord) {
	printf("(%f,%f,%f)\n", LMEcoord_getX(coord), LMEcoord_getY(coord), LMEcoord_getZ(coord));
}

void LMEcoord_get(double* dims, LMEcoord * const coord) {
	dims[0] = LMEcoord_getX(coord);
	dims[1] = LMEcoord_getY(coord);
	dims[2] = LMEcoord_getZ(coord);
}
/**
 * @brief LMEcoord_encode: Scale and offset the dimensions of the coordinate
 * @param code (LMEcoordCode*): The coordCode object to write to
 * @param coord (const * LMEcoord): The coordinate to encode
 * @note Currently this assumes the coordinate is coming in as WGS84
 * Lat/Lng/Meters
 */
void LMEcoord_encode(LMEcoordCode* code, LMEcoord * const coord) {
	const double offsets[3] = {180.0, 90.0, 12300.0};
	double scales[3];
	int i;
	double scaleCoords[3];
	double max = (double)UINT_MAX;
	
	LMEcoord_get(&scaleCoords[0], coord);

	for (i=0;i<3;i++) {
		scales[i] = Scale_init(offsets[i], max);
	
		scaleCoords[i] = (scaleCoords[i] + offsets[i]) * scales[i];
	}
	LMEcoordCode_set(code, (uint32_t)scaleCoords[0], (uint32_t)scaleCoords[1], (uint32_t)scaleCoords[2]);

}

/**
 * @brief LMEcoord_decode: Remove scale and offset from encoded coordinate
 * @param coord (LMEcoord*) The coordinate to hold the decoded values
 * @param code (const * LMEcoordCode) The encoded coordinate to decode
 */
void LMEcoord_decode(LMEcoord* coord, LMEcoordCode * const code) {
	const double offsets[3] = {180.0, 90.0, 12300.0};
	double scales[3];
	double max = (double)UINT_MAX;
	int i;
	double coordVals[3];
	coordVals[0] = (double) LMEcoordCode_getX(code);
	coordVals[1] = (double) LMEcoordCode_getY(code);
	coordVals[2] = (double) LMEcoordCode_getZ(code);
	
	for (i=0; i<3;i++) {
		scales[i] = Scale_init(offsets[i], max);
	
		coordVals[i] = (coordVals[i] / scales[i]) - offsets[i];
	}
	LMEcoord_set(coord, coordVals[0], coordVals[1], coordVals[2]);
}

void LMEcoord_toRadians(LMEcoord* coord) {
	coord->dims[0] *= DEG_TO_RAD;
	coord->dims[1] *= DEG_TO_RAD;
}

void LMEcoord_toDegrees(LMEcoord* coord) {
	coord->dims[0] *= RAD_TO_DEG;
	coord->dims[1] *= RAD_TO_DEG;
}

hid_t CoordType_create(herr_t* status) {
	/** Create the coordinate data type **/
	hid_t coordtype;
	hsize_t coord_dim = COORD_DIM;
	
	coordtype = H5Tarray_create2(H5T_NATIVE_UINT, COORD_RANK, &coord_dim);
	//coordarraytype = H5Tcreate(H5T_ARRAY, sizeof(LMEcoordCode));
	//*status = H5Tinsert(coordtype, "dims", HOFFSET(LMEcoordCode, dims), H5T_NATIVE_UINT32);
	return coordtype;
}


void CoordType_destroy(hid_t coordtype, herr_t* status) {
	*status = H5Tclose(coordtype);
}


