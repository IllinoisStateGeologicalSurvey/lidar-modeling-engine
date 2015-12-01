/**
  las_common.c: Common functions for reading and writing LAS files
  **/

#include "mpi.h"
#include "hdf5.h"
#include "common.h"
#include "point.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <liblas/capi/liblas.h>
#include <proj_api.h>


/** Get PROJ4 projection from proj4 string **/
projPJ loadProj(const char* proj4string)
{
    projPJ pj;
    pj = pj_init_plus(proj4string);
    if (!pj)
    {
        fprintf(stderr, "Error: Failed to initialize projection, check string: %s\n", proj4string);
        exit(1);
    } else {
        printf("Loaded projection: %s\n", pj_get_def(pj, 0));
        return pj;
    }
}

/** Get Proj4 projection from LAS header **/
projPJ getLASProj(LASHeaderH header)
{
    LASSRSH srs = LASHeader_GetSRS(header);
    char* projStr = LASSRS_GetProj4(srs);
    // printf("Projection is: %s\n", projStr);
    projPJ pj = loadProj(projStr);

    return pj;
}

int getPointCount(LASHeaderH header)
{
    int pntCount = 0;
    pntCount = LASHeader_GetPointRecordsCount(header);
    
    return pntCount;
}

/** Project point from source projection to target projection. One point at a time **/
int project(projPJ pj_src, projPJ pj_dst, double x, double y, double z)
{
    printf("Source projection: %s\n", pj_get_def(pj_src, 0));
    printf("Target projection: %s\n", pj_get_def(pj_dst, 0));
    if (!(pj_src || pj_dst)) {
        fprintf(stderr, "Error: projections not initialized.\n");
        exit(1);
    }

    pj_transform(pj_src, pj_dst, 1, 1, &x, &y, &z);
    return 0;
}

int createDataset(char* file, char* dataset, hsize_t dims[2]) 
{
    hid_t   file_id, dataset_id, dataspace_id, plist_id; /*identifiers */
    
    hsize_t max_dims[2];
    hsize_t chunk_dims[2];
    herr_t status;
    int rank = 2;

    /* Set file access property list with parallel I/O access */
    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    /* Create a new file using the default properties  This will create the file if
       it doesn't already exist */
    file_id = H5Fcreate(file, H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);

    /* Set the max dimensions and chunking for the dataset */
    //NOTE: Should change this to be dynamic in future 
    max_dims[0] = H5S_UNLIMITED;
    max_dims[1] = H5S_UNLIMITED;

    chunk_dims[0] = 500000;
    chunk_dims[1] = 3;

    /* create the dataspace for the dataset */
    H5Screate_simple(rank, dims, max_dims);

    /* Set the chunking definitions for dataset */
    plist_id = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(plist_id, rank, chunk_dims);


    dataset_id = H5Dcreate(file_id, dataset, H5T_NATIVE_FLOAT, dataspace_id,
            H5P_DEFAULT, plist_id, H5P_DEFAULT);

    /* End access to property list id */
    status = H5Pclose(plist_id);

    /* End access to the dataset and release resources used by it */
    status = H5Dclose(dataset_id);

    /* Terminate access to the dataspace */
    status = H5Sclose(dataspace_id);

    /* Close the file */
    status = H5Fclose(file_id);

    return 0;
}

int readBlock(LASReaderH reader, int offset, int count, Point* points)
{
    int i; // counter
    LASPointH p = NULL;
    LASHeaderH header = NULL;
    LASColorH color = NULL;
    projPJ pj_src, pj_wgs;
    header = LASReader_GetHeader(reader);
    /* Check that the point count is not larger than file */
    if ((offset + count) > LASHeader_GetPointRecordsCount(header)) {
        fprintf(stderr, "ERROR: Point list out of file bounds\n");
        exit(1);
    }
    double *x, *y, *z;
    x = malloc(sizeof(double) * count);
    y = malloc(sizeof(double) * count);
    z = malloc(sizeof(double) * count);
    
    /* Initialize projection parameters */
    pj_src = getLASProj(header);
    pj_wgs = loadProj("+proj=longlat +ellps=WGS84 +datum=WGS84 +vunits=m +no_defs");

    //struct Point* points;
    //points = malloc(sizeof(Point) * count);
    //printf("%d points allocated in memory\n", count);
    // Run GetPointAt outside loop as it is considerably slower than getNextPoint
    for (i = 0; i < count; i++)  {
        //printf("Point:%d\n", i);
        if (i == 0) {
            p = LASReader_GetPointAt(reader, offset);
            if (!p) {
                LASError_Print("Could not read point");
            }

        } else {
            p = LASReader_GetNextPoint(reader);
        }
        x[i] = LASPoint_GetX(p);
        y[i] = LASPoint_GetY(p);
        z[i] = LASPoint_GetZ(p);
        //points[i].i = LASPoint_GetIntensity(p);
        Point_SetIntensity(&points[i], LASPoint_GetIntensity(p));
        
        Point_SetReturns(&points[i], LASPoint_GetReturnNumber(p), LASPoint_GetNumberOfReturns(p));
        //printf("Class is %hhu\n", LASPoint_GetClassification(p));
        //points[i].clss = LASPoint_GetClassification(p);
        Point_SetClassification(&points[i], LASPoint_GetClassification(p));
        color = LASPoint_GetColor(p);
        Point_SetColor(&points[i], LASColor_GetRed(color), LASColor_GetGreen(color), LASColor_GetBlue(color));
    }

    printf("%9.6f, %9.6f, %9.6f\n", x[0], y[0], z[0]);
    printf("Projecting Points\n");
    // Projecting points is faster if they are done at the same time, using an array and count
    pj_transform(pj_src, pj_wgs, count, 1, &x[0], &y[0], &z[0]);
    for (i = 0; i < count; i++) {
        Point_SetCoords(&points[i], x[i] * RAD_TO_DEG, y[i] * RAD_TO_DEG, z[i]);
    }
    Point_print(&points[0]);

    /* Clean up */
    pj_free(pj_src);
    pj_free(pj_wgs);
    free(x);
    free(y);
    free(z);
    LASHeader_Destroy(header);
    header = NULL;

    return 0;
}


        

/** Write block of data to the dataset using offset and block dimensions **/
int writeBlock(char* file, char* dataset, hsize_t offset[2], hsize_t block[2], float* data) 
{
    hid_t   file_id, dset_id, fspace_id, memspace_id, plist_id;
    herr_t status;
    hsize_t rank = 2;
    hsize_t stride[] = {1, 1};
    hsize_t count[] = {1, 1};


    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    file_id = H5Fopen(file, H5F_ACC_RDWR, plist_id);
    dset_id = H5Dopen(file_id, dataset, plist_id);

    fspace_id = H5Dget_space(dset_id);
    
    memspace_id = H5Screate_simple(rank, block, NULL);
    
    status = H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, offset, stride, count, block);

    plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE);

    status = H5Dwrite(dset_id, H5T_NATIVE_FLOAT, memspace_id, fspace_id, plist_id, data);

    status = H5Dclose(dset_id);
    status = H5Sclose(fspace_id);
    status = H5Sclose(memspace_id);
    status = H5Pclose(plist_id);
    status = H5Fclose(file_id);

}





