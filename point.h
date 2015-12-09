#ifndef POINT_H
#define POINT_H

#include <assert.h>
#include <stdlib.h>
#include <math.h>

typedef struct idx_t {
    uint64_t bigIdx;
    uint32_t smIdx;
}

typedef struct coord_t {
    float x;
    float y;
    float z;
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

void Point_SetCoords(Point* p, double x, double y, double z);

void Point_SetReturns(Point* p, int returnNum, int returnCnt);

void Point_SetColor(Point* p, int r, int g, int b);

void Point_SetIntensity(Point* p, int intens);

void Point_SetClassification(Point* p, unsigned char clss);
Point* Point_new(double coords[3], int i, short retns[2], unsigned char clss, short rgb[3]);

void Point_destroy(struct Point *p);

void Point_print(struct Point *p);


#endif
