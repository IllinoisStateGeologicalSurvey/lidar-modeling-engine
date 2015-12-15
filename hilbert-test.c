#include "hilbert.h"
#include "point.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

int main(int argc, char** argv[])
{
    double coords[3];
    coord_dbl_t *coord1, *coord2, *coord3;
    coord1 = malloc(sizeof(coord_dbl_t));
    coords[0] = -180.0000;
    coords[1] = -90.0000;
    coords[2] = -12300.0;
    Coord_Set(coord1, coords[0], coords[1], coords[2]);
    printf("%lf, %lf\n", coord1->x, coord1->y);

    coord2 = malloc(sizeof(coord_dbl_t));
    coords[0] = 180.0000;
    coords[1] = 90.0000;
    coords[2] = 12300.0;
    Coord_Set(coord2, coords[0], coords[1], coords[2]);
    printf("%lf, %lf\n", coord2->x, coord2->y);

    coord3 = malloc(sizeof(coord_dbl_t));
    coords[0] = 0.0;
    coords[1] = 0.0;
    coords[2] = 0.0;
    Coord_Set(coord3, coords[0], coords[1], coords[2]);

    coord_t *cc1, *cc2, *cc3;
    cc1 = malloc(sizeof(coord_t));
    cc2 = malloc(sizeof(coord_t));
    cc3 = malloc(sizeof(coord_t));
    Coord_Encode(cc1, coord1);
    Coord_Encode(cc2, coord2);
    Coord_Encode(cc3, coord3);
    printf("idx1: %"PRIu32", %"PRIu32"\n", cc1->x, cc1->y);
    printf("idx3: %"PRIu32", %"PRIu32"\n", cc3->x, cc3->y);
    printf("idx2: %"PRIu32", %"PRIu32"\n", cc2->x, cc2->y);

    
    Hpoint *pt1, *pt2, *pt3;
    pt1 = malloc(sizeof(Hpoint));
    pt2 = malloc(sizeof(Hpoint));
    pt3 = malloc(sizeof(Hpoint));
    pt1->hcode[0] = cc1->x;
    pt1->hcode[1] = cc1->y;
    pt2->hcode[0] = cc2->x;
    pt2->hcode[1] = cc2->y;
    pt3->hcode[0] = cc3->x;
    pt3->hcode[1] = cc3->y;

    printf("pt2: %"PRIu32", %"PRIu32"\n", pt2->hcode[0], pt2->hcode[1]);
    Hcode idx1 = H_encode(*pt1);
    Hcode idx2 = H_encode(*pt2);
    Hcode idx3 = H_encode(*pt3);

    //printf("idx1: %"PRIu32", %"PRIu32", %"PRIu32"\n", idx1.hcode[0], idx1.hcode[1], idx1.hcode[2]);
    printf("%lf, %lf\n", coord1->x, coord1->y);
    printf("idx1: %"PRIu32", %"PRIu32"\n", idx1.hcode[0], idx1.hcode[1]);

    printBits(sizeof(uint32_t), &idx1.hcode[0]);
    printBits(sizeof(uint32_t), &idx1.hcode[1]);
    //printBits(sizeof(uint32_t), &idx1.hcode[2]);

    printf("idx3: %"PRIu32", %"PRIu32"\n", idx3.hcode[0], idx3.hcode[1]);
    printf("%lf, %lf\n", coord3->x, coord3->y);
    printBits(sizeof(uint32_t), &idx3.hcode[0]);
    printBits(sizeof(uint32_t), &idx3.hcode[1]);
    
    
    printf("idx2: %"PRIu32", %"PRIu32"\n", idx2.hcode[0], idx2.hcode[1]);
    printf("%lf, %lf\n", coord2->x, coord2->y);
    printBits(sizeof(uint32_t), &idx2.hcode[0]);
    printBits(sizeof(uint32_t), &idx2.hcode[1]);
    //printBits(sizeof(uint32_t)*3, &idx1.hcode[0]);
    //printBits(sizeof(uint32_t)*3, &idx3.hcode[0]);
    //printBits(sizeof(uint32_t)*3, &idx2.hcode[0]);
    /** NOW REVERSE ENCODING **/
    *pt1 = H_decode(idx1);
    *pt2 = H_decode(idx2);
    *pt3 = H_decode(idx3);
    printf("idx1: %"PRIu32", %"PRIu32"\n", pt1->hcode[0], pt1->hcode[1]);
    printf("idx3: %"PRIu32", %"PRIu32"\n", pt3->hcode[0], pt3->hcode[1]);
    printf("idx2: %"PRIu32", %"PRIu32"\n", pt2->hcode[0], pt2->hcode[1]);
    cc1->x = pt1->hcode[0];
    cc1->y = pt1->hcode[1];
    cc2->x = pt2->hcode[0];
    cc2->y = pt2->hcode[1];
    cc3->x = pt3->hcode[0];
    cc3->y = pt3->hcode[1];
    Coord_Decode(coord1, cc1);
    Coord_Decode(coord2, cc2);
    Coord_Decode(coord3, cc3);
    printf("Coord1: %lf, %lf\n", coord1->x, coord1->y);
    printf("Coord3: %lf, %lf\n", coord3->x, coord3->y);
    printf("Coord2: %lf, %lf\n", coord2->x, coord2->y);
    
    free(pt1);
    free(pt2);
    free(pt3);
    free(cc1);
    free(cc2);
    free(cc3);
    free(coord1);
    free(coord2);
    free(coord3);
    return 0;
}
