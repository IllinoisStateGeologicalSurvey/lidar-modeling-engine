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

#include <liblas/capi/liblas.h>
#include <proj_api.h>

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


projPJ loadProj(const char* proj4string)
{
    projPJ pj;
    pj = pj_init_plus(proj4string);
    //printf("error-state: %s\n", pj_strerrno(*pj_get_errno_ref()));
    if (!pj)
    {
        fprintf(stderr, "Error: Failed to initialize WGS84 projection\n");
        exit(1);
    } else {
        printf("LOADED: Projection: %s\n", pj_get_def(pj, 0));
        return pj;
    }
    
}

projPJ getProj(LASHeaderH header) 
{
    LASSRSH srs = LASHeader_GetSRS(header);
    char* projStr = LASSRS_GetProj4(srs);
    printf("Projection is: %s\n", projStr);
    projPJ pj = loadProj(projStr);

    return pj;
    
}

int project(projPJ pj_src, projPJ pj_wgs, double x, double y, double z)
{
    printf("SourceProj: %s\n", pj_get_def(pj_src, 0));
    printf("DestProj: %s\n", pj_get_def(pj_wgs, 0));
    if (!(pj_src || pj_wgs)) {
        fprintf(stderr, "Error: projections not initialized.\n");
        exit(1);
    }

    pj_transform(pj_src, pj_wgs, 1, 1, &x, &y, &z);
    return 0;
}







int main (int argc, char* argv[])
{
    int i;
    int verbose = FALSE;
    char* file_name_in = 0;

    LASReaderH reader = NULL;
    LASHeaderH header = NULL;
    LASPointH p = NULL;
    //double x, y, z;
    projPJ pj_src, pj_wgs;


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

    int pntCount = LASHeader_GetPointRecordsCount(header);
    double *x,*y,*z;
    x = malloc(sizeof(double) * pntCount);
    y = malloc(sizeof(double) * pntCount);
    z = malloc(sizeof(double) * pntCount);
    

    fprintf(stderr, "first pass reading %d points ...\n", pntCount);
    printf("Checking Source projection:\n");
    pj_src = getProj(header);
    pj_wgs = loadProj("+proj=longlat +ellps=WGS84 +datum=WGS84 +vunits=m +no_defs");

    /*p = LASReader_GetNextPoint(reader);
    if (!p) {
        if (LASError_GetLastErrorNum())
            LASError_Print("Not able to fetch a point. LASReader is invalid");
        else
            LASError_Print("File does not contain any points to read.");
        exit(1);
    }*/
    /** Should refactor this to allow for mpi integration, offset and end can be used to separate processes**/
    int poff = 0;
    i = 0;
    p = LASReader_GetPointAt(reader, i);
    while ((poff + i) < (pntCount +1)) {
        x[i] = LASPoint_GetX(p) * LASHeader_GetScaleX(header) + LASHeader_GetOffsetX(header);
        y[i] = LASPoint_GetY(p) * LASHeader_GetScaleY(header) + LASHeader_GetOffsetY(header);
        z[i] = LASPoint_GetZ(p) * LASHeader_GetScaleZ(header) + LASHeader_GetOffsetZ(header);
        p = LASReader_GetNextPoint(reader);
        //fprintf(stderr,"Loading point %d: %9.6f, %9.6f, %9.6f\n",i, x[i], y[i], z[i]);
        i++;
    }

    printf("%9.6f, %9.6f, %9.6f\n", x[0], y[0], z[0]);
    printf("Projecting Point\n");
    
    //project(pj_src, pj_wgs, x, y, z);
    pj_transform(pj_src, pj_wgs, pntCount, 1, &x[0], &y[0], &z[0]);
    for (i = 0; i < pntCount; i++) {
        x[i] *= RAD_TO_DEG;
        y[i] *= RAD_TO_DEG;
    }
    printf("%9.6f, %9.6f, %9.6f\n", x[0], y[0], z[0]);
    printf("Successfully opened the %s\n", file_name_in);


    /* Clean up */
    pj_free(pj_src);
    pj_free(pj_wgs);
    free(x);
    free(y);
    free(z);
    LASReader_Destroy(reader);
    reader = NULL;
    LASHeader_Destroy(header);
    header = NULL;

    if (verbose) ptime("done.");

    
    return 0;
}


