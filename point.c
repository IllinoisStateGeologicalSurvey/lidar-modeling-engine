#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "point.h"

void Point_SetCoords(Point* p, double x, double y, double z)
{
    p->coords[0] = x;
    p->coords[1] = y;
    p->coords[2] = z;
}

void Point_SetReturns(Point* p, int returnNum, int returnCnt) {
    p->retns[0] = returnNum;
    p->retns[1] = returnCnt;
}

void Point_SetColor(Point* p, int r, int g, int b) {
    p->rgb[0] = r;
    p->rgb[1] = g;
    p->rgb[2] = b;
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
    p->coords[0] = coords[0];
    p->coords[1] = coords[1];
    p->coords[2] = coords[2];
    p->i = i;
    p->retns[0] = retns[0];
    p->retns[1] = retns[1];
    p->clss = clss;
    p->rgb[0] = rgb[0];
    p->rgb[1] = rgb[1];
    p->rgb[2] = rgb[2];

    return p;
}

void Point_destroy(struct Point *p)
{
    assert(p != NULL);
    free(p);
}

void Point_print(struct Point *p)
{
    printf("X:%9.6f, Y:%9.6f, Z: %9.6f\n", p->coords[0], p->coords[1], p->coords[2]);
    printf("Intensity: %d\n", p->i);
    printf("Return: %d of %d\n", p->retns[0], p->retns[1]);
    printf("Class: %s\n", p->clss);
};
