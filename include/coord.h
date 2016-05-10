#ifndef LME_COORD_H
#define LME_COORD_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <hdf5.h>

typedef struct LMEcoordCode {
	uint32_t dims[3];
} LMEcoord_int;

typedef struct LMEcoord {
	double dims[3];
} LMEcoord_dbl;

uint32_t LMEcoordCode_getX(const * LMEcoordCode coord);
uint32_t LMEcoordCode_getY(const * LMEcoordCode coord);
uint32_t LMEcoordCode_getZ(const * LMEcoordCode coord);

void LMEcoordCode_setX(LMEcoordCode* coord, uint32_t x);
void LMEcoordCode_setY(LMEcoordCode* coord, uint32_t y);
void LMEcoordCode_setZ(LMEcoordCode* coord, uint32_t z);

void LMEcoordCode_set(LMEcoordCode* coord, uint32_t x, uint32_t y, uint32_t z);
void LMEcoordCode_get(uint32_t dims[3], const * LMEcoordCode coord);

double LMEcoord_getX(const * LMEcoord coord);
double LMEcoord_getY(const * LMEcoord coord);
double LMEcoord_getZ(const * LMEcoord coord);

void LMEcoord_set(LMEcoord* coord, double x, double y, double z);
void LMEcoord_get(double dims[3], const * LMEcoord coord);

void LMEcoord_setX(LMEcoord* coord, double x);
void LMEcoord_setY(LMEcoord* coord, double y);
void LMEcoord_setZ(LMEcoord* coord, double z);

void LMEcoord_encode(LMEcoordCode* coord, const * LMEcoord *raw_coord);
void LMEcoord_decode(LMEcoord* coord, const * LMEcoordCode coord_coded);

hid_t CoordType_create(herr_t* status);
void CoordType_destroy(hid_t coordtype, herr_t* status);

#endif
