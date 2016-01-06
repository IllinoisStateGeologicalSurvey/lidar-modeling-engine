//This is a test to check that the file reading utilities work

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <math.h>
#include <mpi.h>
#include "file_util.h"
#include "common.h"
#include "header.h"

int main(int argc, char* argv[])
{
    char dirname[PATH_MAX];
    size_t size;
    int i;
    size_t totPoints = 0;
    int mpi_size, mpi_rank;
    mpi_size = 0;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;

    /* Initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &mpi_size);
    MPI_Comm_rank(comm, &mpi_rank);
    printf("Testing: %d of %d processes\n", mpi_rank, mpi_size);
    /* Check supplied arguments */
    if (argc < 2)
    {
        fprintf(stderr,  "Usage: fileTest <directory> <# of files>\n");
        exit(1);
    }

    strcpy(dirname, argv[1]);
    printf("Dirname: %s\n", dirname);
    size = atoi(argv[2]);
    /** Mpi task division **/
    
    int block = floor(size / mpi_size);
    int remainder = size % mpi_size;
    if (remainder) {
        printf("Block size is %d and Remainder is %d\n", block, remainder);
        block++;
    }

    /* Root process creates dataset */
    hsize_t  dims[2];
    dims[0] = 1000;
    dims[1] = 1;
    char* filename = "test.h5";
    if (mpi_rank == 0) {
        createHeaderDataset(filename, "/headers", dims);
        printf("created HDF dataset at %s\n", filename);
    }
    // Add a task array to read point information
    task_t tasks[size];
    char* outPaths;
    //printf("PathMax is: %llu\n", PATH_MAX);
    
  //  if (mpi_rank == 0) {

    outPaths = malloc(sizeof(char) * (size * (PATH_MAX + 1)));
    /** Initialize the filepath array **/
        //for (i = 0; i < size; ++i) {
        //    outPaths[i] = malloc(PATH_MAX + 1);
       // }
    //if (mpi_rank == 0) { 
    buildArray(dirname, outPaths, size);
    //}
   // }
    char* sub_paths = malloc(sizeof(char) * (block *  (PATH_MAX + 1)));

    //for (i = 0; i < block; ++i) {
    //    sub_paths[i] = malloc(PATH_MAX + 1);
    //}
    //fprintf(stdout, "Reading %i files from directory %s with block size: %i\n", size, dirname, block);
    printf("Process: %d reading points\n", mpi_rank);
    /** Scatter the filepaths to be read **/
    printf("Scattering paths\n");
    MPI_Scatter(outPaths, block * (PATH_MAX + 1), MPI_CHAR, sub_paths, block * (PATH_MAX + 1), MPI_CHAR, 0, MPI_COMM_WORLD);
    printf("Creating tasks\n");
    /** TODO: Make processes read points **/
//    hsize_t offset[] = {0, 0};
    header_t *headers;
    headers = malloc(sizeof(header_t) * block);
    hsize_t offset[] = {(mpi_rank * block), 0};

/*    for (i = 0; i < block; i++){

        // Create the file reading tasks 
        //printf("Proc: %d, Path %i: %s\n", mpi_rank, i, outPaths[i*(PATH_MAX + 1)]);
        sleep(1);
        char* path;
        path = malloc(sizeof(char) * (PATH_MAX + 1));
        strncpy(path, &sub_paths[i * (PATH_MAX + 1)], (PATH_MAX + 1)); 
        printf("FilePath: %s\n", path);
        
        //taskType_Create(&tasks[i], path, 0, 1);
        printf("Task %d created in process %d\n", i, mpi_rank);

        totPoints = totPoints + tasks[i].size;
        //printf("Point count %d\n", totPoints);
    }*/
    readHeaderBlock(sub_paths, 0, block, headers);
    /*for (i = 0; i < size; i++) 
    {
        taskType_Print(&tasks[i]);
        totPoints = totPoints + tasks[i].count;
        free(outPaths[i]);
        //taskType_Destroy(&tasks[i]);
    }*/
    //if (mpi_rank == 0) {
    //    for (i = 0; i < size; i++) {
    //        free(outPaths[i]);
    //    }
    //TODO: Write the tasks to HDF dataset 
        free(outPaths);

    //} else {
   //     for (i =0; i < block; i++) {
   //         free(sub_paths[i]);
   //     }
   //     free(sub_paths);
    //}
    printf("%llu Files, %llu Points\n", size, totPoints);
    //free(outPaths);
    printf("Process %d done.", mpi_rank);
    MPI_Finalize();
    return 0;
}
    
