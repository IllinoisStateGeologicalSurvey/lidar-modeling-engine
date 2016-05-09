/***************************************************************
 * Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  *
 * Information Laboratory (CIGI). All Rights Reserved.         *
 *                                                             *
 ***************************************************************/

/** 
 * @file common.c
 * @author Nathan Casler
 * @date May 6 2016
 * @brief File containing common utility functions
 *
 */

#include "mpi.h"
#include "hdf5.h"
#include "common.h"
#include "point.h"
#include "hilbert.h"
#include "header.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <liblas/capi/liblas.h>
#include <proj_api.h>


/** 
 * @brief Proj_load: read a projPJ object from a Proj.4 String
 *
 * @param proj4string: const char* = Pointer to proj4 string
 * @param proj: projPJ* = Pointer to projection object to write the definition
 * @return 1 if successful, else 0
 */
int Proj_load(const char* proj4string, projPJ* proj)
{
    //printf("Calling pj_init_plus\n");
    *proj = pj_init_plus(proj4string);
    //printf("Projection initiated with:  %s\n", proj4string);
    if (!proj)
    {
        fprintf(stderr, "Error: Failed to initialize projection, check string: %s\n", proj4string);
		return 0;
		//TODO:Create a graceful error strategy
		//MPI_Finalize();
        //exit(1);
    } else {
        //printf("Projection Loaded\n");
        //printf("Loaded projection: %s\n", pj_get_def(*proj, 0));
        return 1;
    }
}

/** 
 * @brief LASProj_get: Retrive a projPJ projection definition from a LAS file header
 *
 * @param header: LASHeaderH* = Pointer to LAS file header
 * @param proj: projPJ* = Pointer to projPJ object to write the projection
 * definition
 * @return 1 if successful, else 0
 */
int  LASProj_get(LASHeaderH* header, projPJ* proj)
{
    //printf("Reading the projection from header\n");
    LASSRSH srs = LASHeader_GetSRS(*header);
    //printf("Project loaded from Header\n");
    char* projStr = LASSRS_GetProj4(srs);
	//printf("LASProg_get found : %s\n", projStr);
    //printf("Projection is: %s\n", projStr);
    
    if (!Proj_load(projStr, proj)) {

		printf("LASProj_Get Projection Error: Failed to load projection from source\n");
		return 0;
	}

    return 1;
}


/** 
 * @brief getPointCount: Read the number of points from a LAS file header
 *
 * @param header: LASHeaderH = Header object containing LAS metadata
 * @return Int = Number of points in the file
 */
int getPointCount(LASHeaderH header)
{
    int pntCount = 0;
    pntCount = LASHeader_GetPointRecordsCount(header);
    
    return pntCount;
}

/** @brief project: Project a point from one projection to another
 *
 * @param pj_src: projPJ = Source projection definition
 * @param pj_dst: projPJ = Destination projection definition
 * @param x: double = The coordinate on the X-axis
 * @param y: double = The coordinate on the Y-axis
 * @param z: double = The coordinate on the Z-axis
 * @return 0 if projection succeeds, else 1 (TODO: Should probably reverse these
 * return values for consitency)
 */
int project(projPJ pj_src, projPJ pj_dst, double x, double y, double z)
{
    //printf("Source projection: %s\n", pj_get_def(pj_src, 0));
    //printf("Target projection: %s\n", pj_get_def(pj_dst, 0));
    if (!(pj_src || pj_dst)) {
        fprintf(stderr, "Error: projections not initialized.\n");
        exit(1);
    }

    pj_transform(pj_src, pj_dst, 1, 1, &x, &y, &z);
    return 0;
}

/**
 * @brief createDataset: Create an HDF5 dataset witha given name and size
 *
 * @param file: char* = Pointer to string containing HDF5 filename
 * @param dataset: char* = Pointer to string containing the desired dataset name
 * @param dim: hsize_t* = Array containing the lengths of the dataset dimensions
 */
int createDataset(char* file, char* dataset, hsize_t* dim) 
{
    hid_t	file_id, dataset_id, dataspace_id, plist_id, pointtype;

    hsize_t* max_dim = malloc(sizeof(hsize_t));
    hsize_t* chunk_dim = malloc(sizeof(hsize_t));
    herr_t status;
    int rank = 1;

    /* Set file access property list with parallel I/O access */
    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    /* Create a new file using the default properties  This will create the file if
       it doesn't already exist */
    file_id = H5Fcreate(file, H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);
    
    //idxtype = IdxType_create(status);

 //   coordtype = CoordType_create(status);

    /** Create the return data type **/
 //   returntype = ReturnType_create(status);

    /** Create the color data type **/
 //   colortype = ColorType_create(status);

    /* Create the point data type **/
    pointtype = PointType_create(&status);
    //printf("Point Size is %d\n", sizeof(Point));
    //printf("PointType Size is %d\n",H5T_get_size(pointtype));

    /* Set the max dimensions and chunking for the dataset */
    //NOTE: Should change this to be dynamic in future 
    *max_dim = H5S_UNLIMITED;
 //   max_dims[1] = H5S_UNLIMITED;

    *chunk_dim = 500000;
 //   chunk_dims[1] = 1;

    /* create the dataspace for the dataset */
    dataspace_id =  H5Screate_simple(rank, dim, max_dim);

    /* Set the chunking definitions for dataset */
    plist_id = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(plist_id, rank, chunk_dim);


    dataset_id = H5Dcreate(file_id, dataset, pointtype, dataspace_id,
            H5P_DEFAULT, plist_id, H5P_DEFAULT);

    /* End access to property list id */
    status = H5Pclose(plist_id);

    /* End access to the dataset and release resources used by it */
    status = H5Dclose(dataset_id);

    /* Terminate access to the dataspace */
    status = H5Sclose(dataspace_id);

    /* Close Data type references */
 //   CoordType_destroy(coordtype, status);
 //   ReturnType_destroy(returntype, status);
 //   ColorType_destroy(colortype, status);
    PointType_destroy(pointtype, &status);
    /* Close the file */
    status = H5Fclose(file_id);
    free(chunk_dim);
    free(max_dim);
    return 0;
}

    



