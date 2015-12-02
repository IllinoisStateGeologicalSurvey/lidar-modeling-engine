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
    
hid_t CoordType_create(herr_t status) {
    /** Create the coordinate data type **/
    hid_t coordtype;
    coordtype = H5Tcreate(H5T_COMPOUND, sizeof(coord_t));
    status = H5Tinsert(coordtype, "x", HOFFSET (coord_t, x), H5T_NATIVE_FLOAT);
    status = H5Tinsert(coordtype, "y", HOFFSET (coord_t, y), H5T_NATIVE_FLOAT);
    status = H5Tinsert(coordtype, "z", HOFFSET (coord_t, z), H5T_NATIVE_FLOAT);
    return coordtype;
}

void CoordType_destroy(hid_t coordtype, herr_t status) {
    status = H5Tclose(coordtype);

}
hid_t ReturnType_create(herr_t status) { 
    /** Create the return data type **/
    hid_t returntype;
    returntype = H5Tcreate(H5T_COMPOUND, sizeof(coord_t));
    status = H5Tinsert(returntype, "rNum", HOFFSET (return_t, rNum), H5T_NATIVE_SHORT);
    status = H5Tinsert(returntype, "rTot", HOFFSET (return_t, rTot), H5T_NATIVE_SHORT);
    return returntype;
}

void ReturnType_destroy(hid_t returntype, herr_t status) {
    status = H5Tclose(returntype);
}

hid_t ColorType_create(herr_t status) {
    hid_t colortype;
    colortype = H5Tcreate(H5T_COMPOUND, sizeof(color_t));
    status = H5Tinsert(colortype, "red", HOFFSET (color_t, r), H5T_NATIVE_SHORT);
    status = H5Tinsert(colortype, "green", HOFFSET (color_t, g), H5T_NATIVE_SHORT);
    status = H5Tinsert(colortype, "blue", HOFFSET (color_t, b), H5T_NATIVE_SHORT);
    return colortype;
}

void ColorType_destroy(hid_t colortype, herr_t status) {
    status = H5Tclose(colortype);
}

hid_t PointType_create(herr_t status) {
    hid_t coordtype, returntype, colortype, pointtype;
    coordtype = CoordType_create(status);
    returntype = ReturnType_create(status);
    colortype = ColorType_create(status);
    pointtype = H5Tcreate(H5T_COMPOUND, sizeof(Point));
    status = H5Tinsert(pointtype, "coords", HOFFSET(Point, coords), coordtype);
    status = H5Tinsert(pointtype, "intensity", HOFFSET(Point, i), H5T_NATIVE_INT);
    status = H5Tinsert(pointtype, "returns", HOFFSET(Point, retns), returntype);
    status = H5Tinsert(pointtype, "class", HOFFSET(Point, clss), H5T_NATIVE_UCHAR);
    status = H5Tinsert(pointtype, "color", HOFFSET(Point, color), colortype);
    
    status = H5Tclose(coordtype);
    status = H5Tclose(returntype);
    status = H5Tclose(colortype);
    return pointtype;
}

void PointType_destroy(hid_t pointtype, herr_t status) {
    status = H5Tclose(pointtype);
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
    hid_t   file_id, dataset_id, dataspace_id, plist_id, colortype, returntype, coordtype, pointtype; /*identifiers */
    
    hsize_t max_dims[2];
    hsize_t chunk_dims[2];
    herr_t status;
    int rank = 2;

    /* Set file access property list with parallel I/O access */
    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    /* Create a new file using the default properties  This will create the file if
       it doesn't already exist */
    file_id = H5Fcreate(file, H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);
    
    /** Create the coordinate data type /
    coordtype = H5Tcreate(H5T_COMPOUND, sizeof(coord_t));
    status = H5Tinsert(coordtype, "x", HOFFSET (coord_t, x), H5T_NATIVE_FLOAT);
    status = H5Tinsert(coordtype, "y", HOFFSET (coord_t, y), H5T_NATIVE_FLOAT);
    status = H5Tinsert(coordtype, "z", HOFFSET (coord_t, z), H5T_NATIVE_FLOAT);
    **/
    coordtype = CoordType_create(status);

    /** Create the return data type /
    returntype = H5Tcreate(H5T_COMPOUND, sizeof(coord_t));
    status = H5Tinsert(returntype, "rNum", HOFFSET (return_t, rNum), H5T_NATIVE_SHORT);
    status = H5Tinsert(returntype, "rTot", HOFFSET (return_t, rTot), H5T_NATIVE_SHORT);
    **/
    returntype = ReturnType_create(status);

    /**colortype = H5Tcreate(H5T_COMPOUND, sizeof(color_t));
    status = H5Tinsert(colortype, "red", HOFFSET (color_t, r), H5T_NATIVE_SHORT);
    status = H5Tinsert(colortype, "green", HOFFSET (color_t, g), H5T_NATIVE_SHORT);
    status = H5Tinsert(colortype, "blue", HOFFSET (color_t, b), H5T_NATIVE_SHORT);
    **/
    colortype = ColorType_create(status);

    /**pointtype = H5Tcreate(H5T_COMPOUND, sizeof(Point));
    status = H5Tinsert(pointtype, "coords", HOFFSET(Point, coords), coordtype);
    status = H5Tinsert(pointtype, "intensity", HOFFSET(Point, i), H5T_NATIVE_INT);
    status = H5Tinsert(pointtype, "returns", HOFFSET(Point, retns), returntype);
    status = H5Tinsert(pointtype, "class", HOFFSET(Point, clss), H5T_NATIVE_UCHAR);
    status = H5Tinsert(pointtype, "color", HOFFSET(Point, rgb), colortype);
    **/
    pointtype = PointType_create(status);

    /* Set the max dimensions and chunking for the dataset */
    //NOTE: Should change this to be dynamic in future 
    max_dims[0] = H5S_UNLIMITED;
    max_dims[1] = H5S_UNLIMITED;

    chunk_dims[0] = 500000;
    chunk_dims[1] = 1;

    /* create the dataspace for the dataset */
   dataspace_id =  H5Screate_simple(rank, dims, max_dims);

    /* Set the chunking definitions for dataset */
    plist_id = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(plist_id, rank, chunk_dims);


    dataset_id = H5Dcreate(file_id, dataset, pointtype, dataspace_id,
            H5P_DEFAULT, plist_id, H5P_DEFAULT);

    /* End access to property list id */
    status = H5Pclose(plist_id);

    /* End access to the dataset and release resources used by it */
    status = H5Dclose(dataset_id);

    /* Terminate access to the dataspace */
    status = H5Sclose(dataspace_id);

    /* Close Data type references */
    CoordType_destroy(coordtype, status);
    ReturnType_destroy(returntype, status);
    ColorType_destroy(colortype, status);
    PointType_destroy(pointtype, status);
    /**status = H5Tclose(coordtype);
    status = H5Tclose(returntype);
    status = H5Tclose(colortype);
    status = H5Tclose(pointtype);**/
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
        Point_SetCoords(&points[i], ((float)x[i] * RAD_TO_DEG), ((float)y[i] * RAD_TO_DEG), (float)z[i]);
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
    LASColor_Destroy(color);

    return 0;
}


        

/** Write block of data to the dataset using offset and block dimensions **/
int writeBlock(char* file, char* dataset, hsize_t offset[2], hsize_t block[2], Point* points, MPI_Comm comm, MPI_Info info) 
{
    hid_t   file_id, dset_id, fspace_id, pointtype, memspace_id, plist_id;
    herr_t status;
    hsize_t rank = 2;
    hsize_t stride[] = {1, 1};
    hsize_t count[] = {1, 1};


    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_mpio(plist_id, comm, info);
    file_id = H5Fopen(file, H5F_ACC_RDWR | H5F_ACC_DEBUG , plist_id);
    plist_id = H5Pcreate(H5P_DATASET_ACCESS);
    dset_id = H5Dopen(file_id, dataset, plist_id);

    fspace_id = H5Dget_space(dset_id);
    
    memspace_id = H5Screate_simple(rank, block, NULL);
    
    pointtype = PointType_create(status);

    status = H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, offset, stride, count, block);

    plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE);

    status = H5Dwrite(dset_id, pointtype, memspace_id, fspace_id, plist_id, points);

    status = H5Dclose(dset_id);
    status = H5Sclose(fspace_id);
    status = H5Sclose(memspace_id);
    status = H5Pclose(plist_id);
    PointType_destroy(pointtype, status);
    status = H5Fclose(file_id);
    
    return 0;
}





