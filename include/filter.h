#ifndef FILTER_H
#define FILTER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <hdf5.h>
#include <math.h>
#include <inttypes.h>
#include <limits.h>
#include <proj_api.h>
#include <liblas/capi/liblas.h>
#include "point.h"

typedef struct filter_t {
	bound_dbl_t range;
	int retn;
	int intens;
	char clas;
} filter_t;

filter_t* Filter_Create();

int Filter_Set(filter_t* filter, bound_dbl_t* range, int retn, int intensity, char classification);

int Filter_SetRange(filter_t* filter, bound_dbl_t* range);

int Filter_SetReturn(filter_t* filter, int retnFlag);

void Filter_Destroy(filter_t* filter);

int Filter_RangeCheck(filter_t* filter, LASPointH* lasPnt);

int Filter_ReturnCheck(filter_t* filter, LASPointH* lasPnt);

#endif // FILTER_H




