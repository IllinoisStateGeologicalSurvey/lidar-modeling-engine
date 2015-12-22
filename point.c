#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <hilbert.h>
#include <math.h>
#include "point.h"

const double offsets[3] = {180.0, 90.0, 12300.0};
const double max = (double)UINT_MAX;


double Scale_init(double offset, double max)
{
    double range = 2.0 * offset;
    double scale = max / range;
    return scale;
}

void Point_SetIndex(Point *p, idx_t* idx) {
    p->idx = *idx;
}

void Point_IndexCoords(Point *p) {
    uint64_t x, y;
    Hpoint *pt;
    Hcode idx;
    idx_t idxFull;
    //uint64_t *idxFull;
    //x = p->coords.x;
    //y = p->coords.y;

    pt = malloc(sizeof(Hpoint));
    pt->hcode[0] = p->coords.x;
    pt->hcode[1] = p->coords.y;
    //printf("Computing index\n");
    idx = H_encode(*pt);
    //printf("Combining indexes\n");
    //printf("Point index has size: %i, and index pieces are %i, %i", sizeof(p->idx), sizeof(idx.hcode[0]), sizeof(idx.hcode[1]));
    combineIndices(&idxFull, idx.hcode[0], idx.hcode[1]);
    //printf("Adding index\n");
    Point_SetIndex(p, &idxFull);
    free(pt);

}


void Point_SetCoords(Point* p, coord_t* coords)
{
    p->coords.x = coords->x;
    p->coords.y = coords->y;
    p->coords.z = coords->z;
}

void Coord_Set(coord_dbl_t* coords, double x, double y, double z)
{
    coords->x = x;
    coords->y = y;
    coords->z = z;
}

void Coord_Get(double coord_arr[3], coord_dbl_t* coords) {
    coord_arr[0] = coords->x;
    coord_arr[1] = coords->y;
    coord_arr[2] = coords->z;
    
}

void Coord_SetCode(coord_t* coords, uint32_t x, uint32_t y, uint32_t z)
{
    coords->x = x;
    coords->y = y;
    coords->z = z;
}

/** Coord_Encode: Applies scale and offset to a given coordinate set
    
    @param coord_raw: Structure holding 3 double precision coordinates
    @param coord_code: Structure holding 3 unsigned 32bit integer coordinate codes
**/
void Coord_Encode(coord_t* coord_code, coord_dbl_t* coord_raw)
{
    double scales[3];
    int i;
    double rawCoords[3];
    double max = (double)UINT_MAX;
    // Get the coordinate values as an array for iterating
    Coord_Get(rawCoords, coord_raw);
    
    //printf("CE:Raw coords: %f, %f, %f \n", rawCoords[0], rawCoords[1], rawCoords[2]); 
    for (i=0;i<3;i++) {
        // Encoding scheme code = (coordinate + offset) * scalx
        scales[i] = Scale_init(offsets[i], max);
        //printf("Offset: %f, Scale: %f\n", offsets[i], scales[i]);
        //printf("Coord with offset: %f\n", (rawCoords[i] + offsets[i]));
        rawCoords[i] = (rawCoords[i] + offsets[i]) * scales[i];
        //printf("Code %i is %f\n", i, rawCoords[i]);
        //printf("%f becomes %u\n", rawCoords[i], (uint32_t)rawCoords[i]);   
    }
    Coord_SetCode(coord_code, (uint32_t)rawCoords[0], (uint32_t)rawCoords[1], (uint32_t)rawCoords[2]);
    
}

void Coord_Decode(coord_dbl_t* coords, coord_t* coord_code) {
    double scales[3];
    int i;
    double outCoords[3];
    outCoords[0] = (double)coord_code->x;
    outCoords[1] = (double)coord_code->y;
    outCoords[2] = (double)coord_code->z;

    for (i=0;i<3;i++) {
        // Decoding scheme: coord = (code / scale) - offset
        //printf("Code is %f\n", outCoords[i]);
        scales[i] = Scale_init(offsets[i], max);
        //printf("Offset: %f, Scale: %f\n", offsets[i], scales[i]);
        //printf("Unscaled coord = %f\n", (outCoords[i] / scales[i]));
        outCoords[i] = (outCoords[i] / scales[i]) - offsets[i];
    }
    Coord_Set(coords, outCoords[0], outCoords[1], outCoords[2]);
}

void Point_SetReturns(Point* p, int returnNum, int returnCnt) {
    p->retns.rNum = returnNum;
    p->retns.rTot = returnCnt;
}

void Point_SetColor(Point* p, int r, int g, int b) {
    p->color.r = r;
    p->color.g = g;
    p->color.b = b;
}

void Point_SetIntensity(Point* p, int intens) {
    p->i = intens;
}

void Point_SetClassification(Point* p, unsigned char clss) {
    p->clss = clss;
}
Point* Point_new(idx_t *idx, double coords[3], int i, short retns[2], unsigned char clss, short rgb[3])
{
    Point* p = malloc(sizeof(Point));
    assert(p != NULL);
    Point_SetIndex(p, idx);
    p->coords.x = coords[0];
    p->coords.y = coords[1];
    p->coords.z = coords[2];
    p->i = i;
    p->retns.rNum = retns[0];
    p->retns.rTot = retns[1];
    p->clss = clss;
    p->color.r = rgb[0];
    p->color.g = rgb[1];
    p->color.b = rgb[2];

    return p;
}

void Point_destroy(struct Point *p)
{
    assert(p != NULL);
    free(p);
}

void Point_print(struct Point *p)
{
    printf("X:%9.6f, Y:%9.6f, Z: %9.6f\n", p->coords.x, p->coords.y, p->coords.z);
    printf("Intensity: %d\n", p->i);
    printf("Return: %d of %d\n", p->retns.rNum, p->retns.rTot);
    printf("Class: %hhu\n", p->clss);
};
