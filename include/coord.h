#ifndef LME_COORD_H
#define LME_COORD_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <hdf5.h>

typedef struct LMEcoordCode {
	uint32_t dims[3];
} LMEcoordCode;

typedef struct LMEcoord {
	double dims[3];
} LMEcoord;

uint32_t LMEcoordCode_getX(LMEcoordCode * const coord);
uint32_t LMEcoordCode_getY(LMEcoordCode * const coord);
uint32_t LMEcoordCode_getZ(LMEcoordCode * const coord);

void LMEcoordCode_setX(LMEcoordCode* coord, uint32_t x);
void LMEcoordCode_setY(LMEcoordCode* coord, uint32_t y);
void LMEcoordCode_setZ(LMEcoordCode* coord, uint32_t z);

void LMEcoordCode_set(LMEcoordCode* coord, uint32_t x, uint32_t y, uint32_t z);
void LMEcoordCode_get(uint32_t* dims, LMEcoordCode * const coord);

int LMEcoord_copy(LMEcoord* dstCoord, LMEcoord * const srcCoord);
void LMEcoord_print(LMEcoord * const coord);


double LMEcoord_getX(LMEcoord * const coord);
double LMEcoord_getY(LMEcoord * const coord);
double LMEcoord_getZ(LMEcoord * const coord);

void LMEcoord_set(LMEcoord* coord, double x, double y, double z);
void LMEcoord_get(double* dims, LMEcoord * const coord);

void LMEcoord_setX(LMEcoord* coord, double x);
void LMEcoord_setY(LMEcoord* coord, double y);
void LMEcoord_setZ(LMEcoord* coord, double z);

void LMEcoord_encode(LMEcoordCode* coord, LMEcoord * const raw_coord);
void LMEcoord_decode(LMEcoord* coord, LMEcoordCode * const coord_coded);

hid_t CoordType_create(herr_t* status);
void CoordType_destroy(hid_t coordtype, herr_t* status);

double LMEcoord_distance(LMEcoord* coord1, LMEcoord* coord2);

void LMEcoord_toRadians(LMEcoord* coord);
void LMEcoord_toDegrees(LMEcoord* coord);

#endif
