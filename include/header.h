#ifndef HEADER_H
#define HEADER_H

#include <point.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <liblas/capi/liblas.h>
#include <proj_api.h>

typedef struct proj_t {
    char proj4[PATH_MAX + 1];
} proj_t;

typedef struct bound_t {
    coord_t low;
    coord_t high; 
} bound_t;

typedef struct header_t {
    uint32_t pnt_count;
    bound_t bounds;
    char path[PATH_MAX+1];
    proj_t proj;
} header_t;

int createHeaderDataset(char* file, char* dataset, hsize_t* dims);

hid_t ProjType_create(herr_t status);

void ProjType_destroy(hid_t projtype, herr_t status);

hid_t BoundType_create(herr_t status);

void BoundType_destroy(hid_t boundtype, herr_t status);

hid_t HeaderType_create(herr_t status);

void HeaderType_destroy(hid_t headertype, herr_t status);

#endif
