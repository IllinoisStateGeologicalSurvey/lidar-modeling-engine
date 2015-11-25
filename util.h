#ifndef UTIL_H
#define UTIL_H
/** util.h: common utility functions needed for LAS/HDF read/write 
  * Adapted from work by Yan Liu */

typedef struct {
    double Tread;
    double Tcommdata;
    double Tcompute;
    double Tcommresult;
    double Twrite;
    double Ttotal;
} Jobstat;

double get_timemark();
void print_jobstat();
int get_block(int rank, int np, int pntCount, int pntLength, int *offsetx, int*offsety, int *sizex, int *sizey);

#endif
