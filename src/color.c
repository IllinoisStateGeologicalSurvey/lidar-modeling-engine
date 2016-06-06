#include <stdlib.h>
#include <stdio.h>
#include <hdf5.h>
#include <liblas/capi/liblas.h>
#include "color.h"

void LMEcolor_set(LMEcolor* color, int r, int g, int b) {
	color->r = (short) r;
	color->g = (short) g;
	color->b = (short) b;
}

int LMEcolor_fromLAS(LMEcolor* color, LASPointH * const lasPnt) {
	LASColorH lasclr = LASPoint_GetColor(*lasPnt);
	LMEcolor_set(color, LASColor_GetRed(lasclr), LASColor_GetGreen(lasclr), LASColor_GetBlue(lasclr));
	return 0;	
}

hid_t ColorType_create(herr_t* status) {
	hid_t colortype;
	colortype = H5Tcreate(H5T_COMPOUND, sizeof(LMEcolor));
	*status = H5Tinsert(colortype, "red", HOFFSET (LMEcolor, r), H5T_NATIVE_SHORT);
	*status = H5Tinsert(colortype, "green", HOFFSET(LMEcolor, g), H5T_NATIVE_SHORT);
	*status = H5Tinsert(colortype, "blue", HOFFSET(LMEcolor, b), H5T_NATIVE_SHORT);
	return colortype;
}



void ColorType_destroy(hid_t colortype, herr_t* status) {
	*status = H5Tclose(colortype);
	
}
