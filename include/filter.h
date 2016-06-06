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
#include "bound.h"
typedef struct LMEfilter {
	LMEbound range;
	int retn;
	int intens;
	char clas;
} LMEfilter;

/** @brief Constructor argument to generate a filter
 * This will create a default filter definition to use
 * @param filter (LMEfilter*): A pointer to an allocated LMEfilter object
 */
int LMEfilter_create(LMEfilter* filter);

int LMEfilter_set(LMEfilter* filter, LMEbound* range, int retn, int intensity, char classification);

int LMEfilter_setRange(LMEfilter* filter, LMEbound* range);

int LMEfilter_setReturn(LMEfilter* filter, int retnFlag);

void LMEfilter_destroy(LMEfilter* filter);

int LMEfilter_rangeCheck(LMEfilter* filter, LASPointH* lasPnt);

int LMEfilter_returnCheck(LMEfilter* filter, LASPointH* lasPnt);

#endif // FILTER_H




