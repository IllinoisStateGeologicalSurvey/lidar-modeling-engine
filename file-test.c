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
    
    int blockDef = floor(size / mpi_size);

    int remainder = size % mpi_size;
    if (remainder) {
        printf("Block size is %d and Remainder is %d\n", blockDef, remainder);
        blockDef++;
    }
    hsize_t block = blockDef;

    /* Root process creates dataset */
    hsize_t  dims;
    dims = 1000;
    
    char* pathBuf = malloc(sizeof(char) * (PATH_MAX+1)); /** Create a char buffer to hold the working path **/
    int pathLen; /** Counter that returns the index of the end of the path(helpful for concatenation  **/
    pathLen = getWorkingDir(pathBuf); 
    
    printf("Working Path is: %s\n", pathBuf);
    strcpy(&pathBuf[pathLen], "/test.h5\0");
    printf("File path is: %s\n", pathBuf);
    char* filename = "test.h5";
    if (mpi_rank == 0) {
        createHeaderDataset(pathBuf, "/headers", &dims);
        printf("created HDF dataset at %s\n", pathBuf);
    }
    // Add a task array to read point information
    //task_t tasks[size];
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

    printf("Process: %d reading points\n", mpi_rank);
    /** Scatter the filepaths to be read **/
    printf("Scattering paths\n");
    MPI_Scatter(outPaths, block * (PATH_MAX + 1), MPI_CHAR, sub_paths, block * (PATH_MAX + 1), MPI_CHAR, 0, MPI_COMM_WORLD);
    printf("Creating tasks\n");
    /** TODO: Make processes read points **/
    header_t *headers;
    headers = malloc(sizeof(header_t) * block);
    hsize_t offset = (mpi_rank * block);
    printf("Allocated header memory\n");
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
    /** Read the path values from the files in parallel, necessary due to speed of statting las files  **/
    readHeaderBlock(sub_paths, 0, &block, headers);
    
    printf("Loaded Header data, writing to file %s, dataset: %s\n", pathBuf, "/headers");
    /** Write the header data to the HDF dataset **/
    printf("Process %d has %zu offset\n", mpi_rank, offset);
    writeHeaderBlock(pathBuf, "/headers", &offset, &block, headers, comm, info);
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
    printf("Freeing memory\n");
    free(outPaths);
    free(sub_paths);
    free(headers);
    free(pathBuf);
    //} else {
   //     for (i =0; i < block; i++) {
   //         free(sub_paths[i]);
   //     }
   //     free(sub_paths);
    //}
    printf("%llu Files, %llu Points\n", size, totPoints);
    //free(outPaths);
    printf("Process %d done.\n", mpi_rank);
    MPI_Finalize();
    return 0;
}
    
