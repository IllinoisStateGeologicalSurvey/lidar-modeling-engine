#ifndef POINT_SET_H
#define POINT_SET_H

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <limits.h>
#include <proj_api.h>
#include <liblas/capi/liblas.h>
#include "point.h"
#include "common.h"

typedef struct point_set {
	size_t count;
	bound_t extent;
	Point* points[];
} point_set;


