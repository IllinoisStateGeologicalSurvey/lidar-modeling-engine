/**********************************************
 * This is sampled from the libLAS las2las source code
 *
 * Will work to read a las file in parallel
 **********************************************/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mpi.h"

#include <liblas/capi/liblas.h>
#include <proj_api.h>
#include "common.h"
#include "point.h"
#include "file_util.h"


#define LAS_FORMAT_10 0
#define LAS_FORMAT_11 1
#define LAS_FORMAT_12 2


/** TO DO: Write projection code using past code
            Implement MPI based las partitioner
            Create OFFSET,SCALE function to push xyz to 32bit integers
            *******************************/
void usage()
{
    fprintf(stderr,"----------------------------------------------\n");
    fprintf(stderr,"       lasReader usage:\n");
    fprintf(stderr,"\n");
    fprintf(stderr," readLas -i in.las\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"----------------------------------------------\n");

}
void ptime(const char *const msg)
{
    float t= ((float)clock())/CLOCKS_PER_SEC;
    fprintf(stderr, "cumulative CPU time thru %s = %f\n", msg, t);
}


int main (int argc, char* argv[])
{
    int i,j;
    int verbose = FALSE;
    char* file_name_in = 0;

    int fileCount;
    LASReaderH reader = NULL;
    LASHeaderH header = NULL;
    
    int mpi_size, mpi_rank;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;

    /* Initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &mpi_size);
    MPI_Comm_rank(comm, &mpi_rank);

    /* Check number of processors */
    for (i = 1; i < argc; i++) {
        if (    strcmp(argv[i],"-h") == 0 ||
                strcmp(argv[i],"--help") == 0
            )
        {
            usage();
            exit(0);
        }
        else if (strcmp(argv[i],"-v") == 0 ||
                strcmp(argv[i],"--verbose") == 0
            )
        {
            verbose = TRUE;
        }

        else if (    strcmp(argv[i],"--input") == 0 ||
                strcmp(argv[i],"-input") == 0 ||
                strcmp(argv[i],"-i") == 0 ||
                strcmp(argv[i],"-in") == 0
            )
        {
            i++;
            file_name_in = argv[i];
        }
        else if (i == argc - 1 && file_name_in == NULL)
        {
            file_name_in = argv[i];
        }
        else 
        {
            fprintf(stderr, "ERROR: unknown argument '%s'\n", argv[i]);
            usage();
            exit(1);
        }
    }
    /* Will need to support multiple input files */
    //fileCount = countLAS(file_name_in);
    printf("File count is %d\n", fileCount);
    if (file_name_in)
    {
        reader = LASReader_Create(file_name_in);
        if (!reader) {
            LASError_Print("Could not open file to read");
            exit(1);
        }
    }
    else
    {
        LASError_Print("Could not open file to read");
        usage();
        exit(1);
    }

    header = LASReader_GetHeader(reader);
    if (!header) {
        LASError_Print("Could not fetch header");
        exit(1);
    }
    
    if (verbose) {
        ptime("start.");
        fprintf(stderr, "first pass reading %d points ...\n", LASHeader_GetPointRecordsCount(header));
    }
    int pntCount = LASHeader_GetPointRecordsCount(header);
    int block_len = floor(pntCount / mpi_size);
    int remainder = pntCount % mpi_size;
    if (remainder) {
        printf("Block size is %d and Remainder is %d\n", block_len, remainder);
        block_len++;
    }
    Point* points;
    if (mpi_rank == 0) {
        points = malloc(sizeof(Point) * pntCount);
    }   

    Point* sub_points = malloc(sizeof(Point) * block_len);

    Point* point;
    point = malloc(sizeof(Point) * pntCount);
    MPI_Datatype* mpi_pointtype;
    MPI_PointType_create(mpi_pointtype, point);
    MPI_Scatter(points, block_len, MPI_FLOAT, sub_points, block_len, MPI_FLOAT, 0, MPI_COMM_WORLD);
    hsize_t offset[] = {0, 0};
    offset[0] = mpi_rank * block_len;
    // TO DO:Need to check the clean up in readBlock to make sure there is no leak 
    readBlock(reader, offset[0], block_len, sub_points);

   
    /** CAN WE MAKE THIS A 1-D array **/
    hsize_t psize[] = {pntCount, 1};
    hsize_t block[] = {block_len, 1};
    if (mpi_rank == 0) {
        createDataset("test.h5", "/pts", psize);
    }
    //MPI_Gather(&sub_points, 1, MPI_FLOAT, points, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    writeBlock("test.h5", "/pts", offset, block, sub_points, comm, info);
    printf("Cleaning up\n");
    LASReader_Destroy(reader);
    reader = NULL;
    Point_destroy(sub_points);
    Point_destroy(point);
    //free(mpi_pointtype);
    if (mpi_rank == 0) {
        Point_destroy(points);
    }
    if (verbose) ptime("done.");

    MPI_Finalize();
    
    return 0;
}


