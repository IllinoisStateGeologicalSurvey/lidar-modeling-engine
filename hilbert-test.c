#include "hilbert.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

int main(int argc, char** argv[])
{
    double *coord1, *coord2, *coord3;
    coord1 = malloc(sizeof(double) * 3);
    coord1[0] = -180.0000;
    coord1[1] = -90.0000;
    coord1[2] = -12300.0;

    coord2 = malloc(sizeof(double) * 3);
    coord2[0] = 180.0000;
    coord2[1] = 90.0000;
    coord2[2] = 12300.0;

    coord3 = malloc(sizeof(double) * 3);
    coord3[0] = 0.0;
    coord3[1] = 0.0;
    coord3[2] = 0.0;

    Hpoint *pt1, *pt2, *pt3;
    pt1 = malloc(sizeof(Hpoint));
    pt2 = malloc(sizeof(Hpoint));
    pt3 = malloc(sizeof(Hpoint));
    scaleCoords(coord1,pt1);
    scaleCoords(coord2,pt2);
    scaleCoords(coord3,pt3);

    Hcode idx1 = H_encode(*pt1);
    Hcode idx2 = H_encode(*pt2);
    Hcode idx3 = H_encode(*pt3);

    printf("idx1: %"PRIu32", %"PRIu32", %"PRIu32"\n", idx1.hcode[0], idx1.hcode[1], idx1.hcode[2]);

    printBits(sizeof(uint32_t), &idx1.hcode[0]);
    printBits(sizeof(uint32_t), &idx1.hcode[1]);
    printBits(sizeof(uint32_t), &idx1.hcode[2]);

    printf("idx3: %"PRIu32", %"PRIu32", %"PRIu32"\n", idx3.hcode[0], idx3.hcode[1], idx3.hcode[2]);

    printBits(sizeof(uint32_t), &idx3.hcode[0]);
    printBits(sizeof(uint32_t), &idx3.hcode[1]);
    printBits(sizeof(uint32_t), &idx3.hcode[2]);
    
    printf("idx2: %"PRIu32", %"PRIu32", %"PRIu32"\n", idx2.hcode[0], idx2.hcode[1], idx2.hcode[2]);
    printBits(sizeof(uint32_t), &idx2.hcode[0]);
    printBits(sizeof(uint32_t), &idx2.hcode[1]);
    printBits(sizeof(uint32_t), &idx2.hcode[2]);
    printBits(sizeof(uint32_t)*3, &idx1.hcode[0]);
    printBits(sizeof(uint32_t)*3, &idx3.hcode[0]);
    printBits(sizeof(uint32_t)*3, &idx2.hcode[0]);
    free(pt1);
    free(pt2);
    free(pt3);
    free(coord1);
    free(coord2);
    free(coord3);
    return 0;
}
