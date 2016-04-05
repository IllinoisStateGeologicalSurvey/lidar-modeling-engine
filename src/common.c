/**
  las_common.c: Common functions for reading and writing LAS files
  **/

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


/** Get PROJ4 projection from proj4 string **/
int Proj_load(const char* proj4string, projPJ* proj)
{
    //projPJ pj;
    //printf("Calling pj_init_plus\n");
    *proj = pj_init_plus(proj4string);
    printf("Projection initiated with:  %s\n", proj4string);
    if (!proj)
    {
        fprintf(stderr, "Error: Failed to initialize projection, check string: %s\n", proj4string);
		return 0;
		//TODO:Create a graceful error strategy
		//MPI_Finalize();
        //exit(1);
    } else {
        //printf("Projection Loaded\n");
        printf("Loaded projection: %s\n", pj_get_def(*proj, 0));
        return 1;
    }
}

/** Get Proj4 projection from LAS header **/
int  LASProj_get(LASHeaderH* header, projPJ* proj)
{
    //printf("Reading the projection from header\n");
    LASSRSH srs = LASHeader_GetSRS(*header);
    //printf("Project loaded from Header\n");
    char* projStr = LASSRS_GetProj4(srs);
	printf("LASProg_get found : %s\n", projStr);
    //printf("Projection is: %s\n", projStr);
    
    if (!Proj_load(projStr, proj)) {

		printf("LASProg_Get Projection Error: Failed to load projection from source\n");
		return 0;
	}

    return 1;
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

int createDataset(char* file, char* dataset, hsize_t* dim) 
{
    //hid_t   file_id, dataset_id, dataspace_id, plist_id, colortype, returntype, coordtype,  pointtype; /*identifiers */
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

    



