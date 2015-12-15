#ifndef POINT_H
#define POINT_H

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>

const double offsets[3];
const double scales[3];

typedef struct idx_t {
    uint64_t bigIdx;
    uint32_t smIdx;
} idx_t;

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

typedef struct Point {
    idx_t   idx;
    coord_t coords;
    int     i; // Intensity
    return_t retns; // Returns [return #, return count]
    unsigned char   clss; // Classification
    color_t color;  // Color [r,g,b]
} Point;

double Scale_init(double offset, double max);

void Point_SetCoords(Point* p, double x, double y, double z);

void Coord_Set(coord_dbl_t* coords, double x, double y, double z);

void Coord_Get(double coord_arr[2], coord_dbl_t* coords);

void Coord_SetCode(coord_t* coord_code, uint32_t x, uint32_t y, uint32_t z);

void Coord_Encode(coord_t* coord_code, coord_dbl_t* coord_raw);

void Coord_Decode(coord_dbl_t* coord_raw, coord_t* coord_code);

void Point_SetReturns(Point* p, int returnNum, int returnCnt);

void Point_SetColor(Point* p, int r, int g, int b);

void Point_SetIntensity(Point* p, int intens);

void Point_SetClassification(Point* p, unsigned char clss);
Point* Point_new(double coords[3], int i, short retns[2], unsigned char clss, short rgb[3]);

void Point_destroy(struct Point *p);

void Point_print(struct Point *p);


#endif
