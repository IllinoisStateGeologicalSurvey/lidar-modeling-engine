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

typedef struct LMEbound {
	LMEcoord low;
	LMEcoord high;
} LME_bound;

typedef struct LMEboundCode {
	LMEcoordCode low;
	LMEcoordCode high;
} LME_boundCode;


hid_t BoundType_create(herr_t* status);

void BoundType_destroy(hid_t boundtype, herr_t* status);


int LMEbound_intersects(const * LMEbound bound_1, const * LMEbound  bound_2);
void LMEbound_set(LMEbound* bounds, const * LMEcoord low, const * LMEcoord high);

int LMEboundCode_fromLAS(const * LASHeaderH header, LMEboundCode* bounds);
int LMEboundCode_intersects(const * LMEboundCode code_1, const* LMEboundCode code_2);
void LMEboundCode_set(LMEboundCode* code, const * LMEcoordCode low, const * LMEcoordCode high);

double LMEbound_length(const * LMEbound bound, int dim);

LMEbound* LMEbound_subdivide(const * LMEbound bound, int countX, int countY, size_t* bound_count);



void LMEbound_encode(LMEboundCode* code, const * LMEbound bounds);

void LMEbound_decode(LMEbound* bounds, const * LMEboundCode* code);

int LMEbound_project(LMEbound* bounds, char* srcProj, char* dstProj);

int LMEbound_fromLAS(LMEbound* bounds, LASSRSH srs);


#endif

