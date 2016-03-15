#ifndef POINT_H
#define POINT_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <mpi.h>
#include <hdf5.h>
#include <assert.h>
#include <math.h>
#include <inttypes.h>
#include <liblas/capi/liblas.h>
const double offsets[3];
const double scales[3];
/*
typedef struct idx_t {
    uint64_t bigIdx;
} idx_t;
*/
typedef uint64_t idx_t;

typedef struct coord_dbl_t {
    double x;
    double y;
    double z;
} coord_dbl_t;


typedef struct coord_t {
    uint32_t x;
    uint32_t y;
    uint32_t z;
} coord_t;

typedef struct return_t {
    short rNum;
    short rTot;
} return_t;

typedef struct color_t {
    short r;
    short g;
    short b;
} color_t;

typedef struct bound_dbl_t {
	coord_dbl_t low;
	coord_dbl_t high;
} bound_dbl_t;

typedef struct Point {
    uint64_t   idx;
    coord_t coords;
    int     i; // Intensity
    return_t retns; // Returns [return #, return count]
    unsigned char   clss; // Classification
    color_t color;  // Color [r,g,b]
} Point;
/** TODO: Find a suitable max length for proj4 strings **/


double Scale_init(double offset, double max);
void Point_SetIndex(Point* p, idx_t* idx);

void Point_SetCoords(Point* p, coord_t* coords);

void Coord_Set(coord_dbl_t* coords, double x, double y, double z);

void Coord_Get(double coord_arr[2], coord_dbl_t* coords);

void Coord_SetCode(coord_t* coord_code, uint32_t x, uint32_t y, uint32_t z);

void Coord_Encode(coord_t* coord_code, coord_dbl_t* coord_raw);

void Coord_Decode(coord_dbl_t* coord_raw, coord_t* coord_code);

void Point_SetReturns(Point* p, int returnNum, int returnCnt);

void Point_SetColor(Point* p, int r, int g, int b);

void Point_SetIntensity(Point* p, int intens);

void Point_SetClassification(Point* p, unsigned char clss);
Point* Point_new(idx_t *idx, double coords[3], int i, short retns[2], unsigned char clss, short rgb[3]);

void Point_destroy(struct Point *p);

hid_t CoordType_create(herr_t* status);

void CoordType_destroy(hid_t coordtype, herr_t* status);

hid_t ReturnType_create(herr_t* status);

void ReturnType_destroy(hid_t returntype, herr_t* status);

hid_t ColorType_create(herr_t* status);

void ColorType_destroy(hid_t colortype, herr_t* status);

hid_t PointType_create(herr_t* status);

void PointType_destroy(hid_t pointtype, herr_t* status);

int MPI_PointType_create(MPI_Datatype* mpi_pointtype);

void Point_print(struct Point *p);


int LASPoint_read(LASPointH* lasPnt, Point* pnt, double* x, double* y, double* z);

int LASPoint_project(LASHeaderH* header, hsize_t* count, double* x, double* y, double* z, Point* pnts, int mpi_rank);

void Bound_dbl_Set(bound_dbl_t* bounds, double minX, double minY, double minZ, double maxX, double maxY, double maxZ);

int Bound_dbl_Project(bound_dbl_t* bound, LASSRSH srs);

#endif
