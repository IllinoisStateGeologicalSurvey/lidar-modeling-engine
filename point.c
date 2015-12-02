#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "point.h"

void Point_SetCoords(Point* p, double x, double y, double z)
{
    p->coords.x = x;
    p->coords.y = y;
    p->coords.z = z;
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
Point* Point_new(double coords[3], int i, short retns[2], unsigned char clss, short rgb[3])
{
    Point* p = malloc(sizeof(Point));
    assert(p != NULL);
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
