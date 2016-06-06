#ifndef LME_RETURN_H
#define LME_RETURN_H

#include <stdlib.h>
#include <stdio.h>
#include <liblas/capi/liblas.h>
#include <hdf5.h>


typedef struct LMEreturn {
	short rNum;
	short rTot;
} LMEreturn;

void LMEreturn_set(LMEreturn* retn, int returnNum, int returnTot);

int LMEreturn_isLast(LMEreturn* retn);

int LMEreturn_isFirst(LMEreturn* retn);

hid_t ReturnType_create(herr_t* status);

void ReturnType_destroy(hid_t returntype, herr_t* status);

int LMEreturn_fromLAS(LMEreturn* retn, LASPointH * const lasPnt);

#endif
