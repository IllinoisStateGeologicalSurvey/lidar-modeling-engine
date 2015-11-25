#ifndef UTIL_C
#define UTIL_C
/** Utility functions neede by the LAS HDF read/write functions **/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include "util.h"

Jobstat jobstat; //stat;

// get current system time
double get_timemark() 
{
    struct timeval tsec;
    struct timezone tzone;
    gettimeofday(&tsec, &tzone);
    return (double)(tsec.tv_sec + tsec.tv_usec/1000000.0);
}

void print_jobstat()
{
    fprintf(stdout, "===================\n");
    fprintf(stdout, "||    Job Stat   ||\n");
    fprintf(stdout, "===================\n");
    fprintf(stdout, "Reading time: %.5lf seconds\n", jobstat.Tread);
    fprintf(stdout, "Data distribution time: %.5lf seconds\n", jobstat.Tcommdata);
    fprintf(stdout, "Running time: %.5lf seconds\n", jobstat.Tcompute);
    fprintf(stdout, "Result collection time: %.5lf seconds\n", jobstat.Tcommresult);
    fprintf(stdout, "Writing time: %.5lf seconds\n", jobstat.Twrite);
    fprintf(stdout, "TOTAL TIME: %.5lf seconds\n", jobstat.Ttotal);
}

// calc the data block to be read given the rank of the process
// rank, np: rank and number of procs
// pntCount, pntLength: number of points to read, number of point attributes
// offset x, offset y: output
// size x, size y: output, useful for procs positioned at the end of dim
int get_block(int mpi_rank, int np, int pntCount, int pntLength, int *offsetx, int *offsety, int *sizex, int *sizey)
{
    int dimx, dimy, blockid;
    int bpnts, rpnts; // num points and remainder
    blockid = mpi_rank;
    bpnts = pntCount / np; // find the number of points to allocate to each process
    rpnts = pntCount % np;
    *offsetx = blockid * bpnts;
    *offsety = 0;
    *sizex = bpnts;
    *sizey = pntLength;
    if (blockid == (np - 1)) { // end block
        *sizex = *sizex + rpnts;
    }
    fprintf(stderr, "Total proc: %d has %d points. Proc %d has %d points and %d offset\n", np, pntCount, mpi_rank, *sizex, *offsetx);
    return 1;


}

#endif
