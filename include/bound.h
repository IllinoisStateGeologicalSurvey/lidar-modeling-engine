#ifndef LME_BOUND_H
#define LME_BOUND_H

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <proj_api.h>
#include <liblas/capi/liblas.h>
#include "point.h"
#include "coord.h"

typedef struct LMEbound {
	LMEcoord low;
	LMEcoord high;
} LMEbound;

typedef struct LMEboundCode {
	LMEcoordCode low;
	LMEcoordCode high;
} LMEboundCode;


hid_t BoundType_create(herr_t* status);

void BoundType_destroy(hid_t boundtype, herr_t* status);


int LMEbound_intersects(LMEbound * const bound_1, LMEbound * const bound_2);
void LMEbound_set(LMEbound* bounds, LMEcoord * const low, LMEcoord * const high);

int LMEbound_copy(LMEbound* dstBounds, LMEbound * const srcBounds);

int LMEboundCode_fromLAS(LMEboundCode* bounds, LASHeaderH * const header);

int LMEbound_fromLAS(LMEbound* bounds, LASHeaderH * const header);
int LMEboundCode_intersects(LMEboundCode * const code_1, LMEboundCode * const code_2);
void LMEboundCode_set(LMEboundCode* code, LMEcoordCode * const low, LMEcoordCode * const high);

double LMEbound_length(LMEbound * const bound, int dim);

int LMEbound_subdivide(LMEbound ** boundArr, LMEbound * const bound, int countX, int countY, size_t* bound_count);

void LMEbound_encode(LMEboundCode* code, LMEbound * const bounds);

void LMEbound_decode(LMEbound* bounds, LMEboundCode * const code);

int LMEbound_project(LMEbound* bounds, LMEcrs * const srcProj, LMEcrs * const dstProj);

int LMEbound_fromLASSRS(LMEbound* bounds, LASSRSH srs);

void LMEbound_print(LMEbound * const bounds);


#endif

