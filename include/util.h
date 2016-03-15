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
/* This function will return error codes from MPI functions if the MPI_ErrorHandler is set 
   @Param mpi_error: code returned from MPI call */
void MPI_check_error(int mpi_error);
/* This function will divide a set of tasks amongst processes in a balanced manner
   @Param count: number of tasks to run
   @Param mpi_size: number of processes
   @Param offsets: pointer to array to hold the offsets for each process to read from
   @Param blocks: pointer to array holding the number of tasks for each processor
*/
int divide_tasks(int count, int mpi_size, int* offsets, int* blocks);

int getDataStore(char* h5Path);

#endif
