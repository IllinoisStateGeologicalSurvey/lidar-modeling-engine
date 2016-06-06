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
#include "coord.h"
#include "retn.h"
#include "color.h"
#include "crs.h"
#include "idx.h"

const double offsets[3];
const double scales[3];
/*
typedef struct idx_t {
    uint64_t bigIdx;
} idx_t;
*/

/*typedef struct coord_dbl_t {
    double x;
    double y;
    double z;
} coord_dbl_t;


typedef struct coord_t {
    uint32_t x;
    uint32_t y;
    uint32_t z;
} coord_t;
*/

/*typedef struct bound_dbl_t {
	coord_dbl_t low;
	coord_dbl_t high;
} bound_dbl_t;
*/
typedef struct LMEpointCode {
	LMEidx idx;
	LMEcoordCode code;
	int i;
	LMEreturn retns;
	unsigned char clss;
	LMEcolor color;
} LMEpointCode;

typedef struct LMEpoint {
	LMEcoord coord;
	int i;
	LMEreturn retns;
	unsigned char clss;
	LMEcolor color;
} LMEpoint;

//typedef struct Point {
//    uint64_t   idx;
//    LME_coord_int coords;
//    int     i; // Intensity
//    return_t retns; // Returns [return #, return count]
//    unsigned char   clss; // Classification
//    color_t color;  // Color [r,g,b]
//} Point;
/** TODO: Find a suitable max length for proj4 strings **/


//double Scale_init(double offset, double max);
void LMEpointCode_setIdx(LMEpointCode* p);

void LMEpointCode_setCoord(LMEpointCode* p, LMEcoordCode * const code);

void LMEpoint_setCoord(LMEpoint* p, LMEcoord * const coord);

void LMEpoint_setReturn(LMEpoint* p, int returnNum, int returnCnt);

void LMEpointCode_setReturn(LMEpointCode* p, int returnNum, int returnCnt);

void LMEpoint_setColor(LMEpoint* p, int r, int g, int b);

void LMEpointCode_setColor(LMEpointCode* p, int r, int g, int b);

void LMEpoint_setIntensity(LMEpoint* p, int intens);

void LMEpointCode_setIntensity(LMEpointCode* p, int intens);

void LMEpoint_setClassification(LMEpoint* p, unsigned char clss);

void LMEpointCode_setClassification(LMEpointCode* p, unsigned char clss);


hid_t PointType_create(herr_t* status);

void PointType_destroy(hid_t pointtype, herr_t* status);

//int MPI_PointType_create(MPI_Datatype* mpi_pointtype);

void LMEpointCode_print(LMEpointCode * const p);

int LMEpoint_fromLAS(LMEpointCode* p, double *x, double * y, double *z, LASPointH * const lasPnt);

int LASPoint_read(LASPointH* lasPnt, LMEpointCode* pnt, double* x, double* y, double* z);

int LASPoint_project(LASHeaderH* header, hsize_t* count, double* x, double* y, double* z, LMEpointCode* pnts, int mpi_rank);

//void Bound_dbl_Set(bound_dbl_t* bounds, double minX, double minY, double minZ, double maxX, double maxY, double maxZ);

//int Bound_dbl_Project(bound_dbl_t* bound, LASSRSH srs);

#endif
