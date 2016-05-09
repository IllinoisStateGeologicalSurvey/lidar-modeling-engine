#ifndef BOUND_H
#define BOUND_H

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

typedef struct bound_dbl_t {
	coord_dbl_t low;
	coord_dbl_t high;
} bound_dbl_t;

typedef struct bound_t {
	coord_t low;
	coord_t high;
} bound_t;


hid_t BoundType_create(herr_t* status);

void BoundType_destroy(hid_t boundtype, herr_t* status);

int LASBound_Get(LASHeaderH* header, bound_t* bounds);

int Bound_intersects(bound_t* bound_1, bound_t* bound_2);

bound_t* Bound_Set(double minX, double minY, double maxX, double maxY);

void Bound_dbl_Set(bound_dbl_t* bounds, double minX, double minY, double minZ, double maxX, double maxY, double maxZ);

int Bound_dbl_Project(bound_dbl_t* bound, LASSRSH srs);


//int MPI_BoundType_create(MPI_Datatype* mpi_boundtype);

#endif

