/**********************************************
 * This is sampled from the libLAS las2las source code
 *
 * Will work to read a las file in parallel
 **********************************************/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <liblas/capi/liblas.h>

//LASPointSummary* SummarizePoints(LASReaderH reader);
//void print_point_summary(FILE *file, LASPointSummary* summary LASHeaderH header);
//void print_point(FILE *file, LASPointH point);
//void print_header(FILE *file, LASHeaderH header, const char* file_name, int bSkipVLR);

#define LAS_FORMAT_10 0
#define LAS_FORMAT_11 1
#define LAS_FORMAT_12 2

struct Point {
    double x; /** Coordinate values **/
    double y;
    double z;
    int    i; /* intensity */
    int    rn; /* return number */
    int    rt; /* total returns */
    int    c; /* classification */
};

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
    int i;
    int verbose = FALSE;
    char* file_name_in = 0;

    LASReaderH reader = NULL;
    LASHeaderH header = NULL;
    LASPointH p = NULL;
    double px, py, pz;




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

    p = LASReader_GetNextPoint(reader);
    if (!p) {
        if (LASError_GetLastErrorNum())
            LASError_Print("Not able to fetch a point. LASReader is invalid");
        else
            LASError_Print("File does not contain any points to read.");
        exit(1);
    }
    
    px = LASPoint_GetX(p) * LASHeader_GetScaleX(header) + LASHeader_GetOffsetX(header);
    py = LASPoint_GetY(p) * LASHeader_GetScaleY(header) + LASHeader_GetOffsetY(header);
    pz = LASPoint_GetZ(p) * LASHeader_GetScaleZ(header) + LASHeader_GetOffsetZ(header);
    
    printf("%9.f, %9.f, %9.f\n", px, py, pz);
    printf("Successfully opened the %s\n", file_name_in);


    /* Clean up */
    LASReader_Destroy(reader);
    reader = NULL;
    LASHeader_Destroy(header);
    header = NULL;

    if (verbose) ptime("done.");

    
    return 0;
}


