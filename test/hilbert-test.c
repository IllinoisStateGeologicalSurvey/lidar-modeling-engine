#include "hilbert.h"
#include "point.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
/** TODO FIND the reason that Y coordinates are sometimes offset by +/- 90 degrees. **/

/** Function to test whether the encoding is functioning properly **/
int checkCoding(double x, double y, double z)
{
    double coords[3];
    coord_dbl_t *coordIn, *coordOut;
    coord_t *ccIn, *ccOut;
    Hpoint *pt;
    Hcode idx;
    uint64_t *idxFull;
    coordIn = malloc(sizeof(coord_dbl_t));
    coordOut = malloc(sizeof(coord_dbl_t));
    printf("Input coordinates:[ %f, %f, %f ]\n", x, y, z);
    Coord_Set(coordIn, x, y, z);
    ccIn = malloc(sizeof(coord_t));
    ccOut = malloc(sizeof(coord_t));
    Coord_Encode(ccIn, coordIn);
    pt = malloc(sizeof(Hpoint));
    pt->hcode[0] = ccIn->x;
    pt->hcode[1] = ccIn->y;
    idx = H_encode(*pt);
    idxFull = malloc(sizeof(uint64_t));
    combineIndices(idxFull, idx.hcode[0], idx.hcode[1]);
    *pt = H_decode(idx);
    ccOut->x = pt->hcode[0];
    ccOut->y = pt->hcode[1];
    ccOut->z = ccIn->z;
    Coord_Decode(coordOut, ccOut);
    printf("Output coordinates:[ %f, %f, %f ]\n", coordOut->x, coordOut->y, coordOut->z);
    printf("YCodeDiff is %u\n", (coordIn->y - coordOut->y));
    printf("YDiff is %f\n", (ccIn->y - ccOut->y));

    free(coordIn);
    free(coordOut);
    free(ccIn);
    free(ccOut);
    free(idxFull);
    free(pt);
    return 0;
}

uint64_t getCode(double x, double y, double z) {
    
    double coords[3];
    coord_dbl_t *coordIn;
    coord_t *ccIn;
    Hpoint *pt;
    Hcode idx;
    uint64_t *idxFull;
    coordIn = malloc(sizeof(coord_dbl_t));
    printf("Input coordinates:[ %f, %f, %f ]\n", x, y, z);
    Coord_Set(coordIn, x, y, z);
    ccIn = malloc(sizeof(coord_t));
    Coord_Encode(ccIn, coordIn);
    pt = malloc(sizeof(Hpoint));
    pt->hcode[0] = ccIn->x;
    pt->hcode[1] = ccIn->y;
    idx = H_encode(*pt);
    idxFull = malloc(sizeof(uint64_t));
    combineIndices(idxFull, idx.hcode[0], idx.hcode[1]);
    free(coordIn);
    free(ccIn);
    free(pt);

    return *idxFull;
}

int main(int argc, char** argv[])
{
    double coords[3];
    //coord_dbl_t *coord1, *coord2, *coord3;
    //coord1 = malloc(sizeof(coord_dbl_t));
    coords[0] = -180.0000;
    coords[1] = -90.0000;
    coords[2] = -12300.0;
    uint64_t test1 = getCode(coords[0], coords[1], coords[2]);
    checkCoding(coords[0], coords[1], coords[2]);

    //Coord_Set(coord1, coords[0], coords[1], coords[2]);
    //printf("%lf, %lf\n", coord1->x, coord1->y);

    //coord2 = malloc(sizeof(coord_dbl_t));
    coords[0] = 180.0000;
    coords[1] = 90.0000;
    coords[2] = 12300.0;
    checkCoding(coords[0], coords[1], coords[2]);
    //Coord_Set(coord2, coords[0], coords[1], coords[2]);
    //printf("%lf, %lf\n", coord2->x, coord2->y);

    //coord3 = malloc(sizeof(coord_dbl_t));
    coords[0] = 0.000000657;
    coords[1] = 0.000000657;
    coords[2] = 0.0;
    checkCoding(coords[0], coords[1], coords[2]);

    coords[0] = -50.0000;
    coords[1] = -50.0000;
    coords[2] = 1500;
    checkCoding(coords[0], coords[1], coords[2]);

    coords[0] = -1.0000;
    coords[1] = -1.0000;
    coords[2] = -1.0000;
    checkCoding(coords[0], coords[1], coords[2]);

    coords[0] = 1.0000;
    coords[1] = 1.00000;
    coords[2] = 1.0000;
    checkCoding(coords[0], coords[1], coords[2]);

    coords[0] = -89.0000;
    coords[1] = -89.00000;
    coords[2] = 1.0000;
    checkCoding(coords[0], coords[1], coords[2]);



    return 0;
}
