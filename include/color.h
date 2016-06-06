#ifndef LME_COLOR_H
#define LME_COLOR_H

#include <stdlib.h>
#include <stdio.h>
#include <liblas/capi/liblas.h>
#include <hdf5.h>

typedef struct LMEcolor {
	short r;
	short g;
	short b;
} LMEcolor;

void LMEcolor_set(LMEcolor* color, int r, int g, int b);

hid_t ColorType_create(herr_t* status);

void ColorType_destroy(hid_t colortype, herr_t* status);

int LMEcolor_fromLAS(LMEcolor* color, LASPointH * const lasPnt);

#endif
