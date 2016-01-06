#include "mpi.h"
#include "hdf5.h"
#include "header.h"
#include "point.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <liblas/capi/liblas.h>
#include <proj_api.h>

int Proj_Set(LASHeaderH header, proj_t proj) {
    LASSRSH srs = LASHeader_GetSRS(header);
    strncpy(proj.proj4, LASSRS_GetProj4(srs), (PATH_MAX + 1));
    return 0;
}


int Bound_Set(LASHeaderH header, bound_t bounds) {
    coord_dbl_t *ur, *ll;
    coord_t *low, *high;
    ur = malloc(sizeof(coord_dbl_t));
    ll = malloc(sizeof(coord_dbl_t));
    low = malloc(sizeof(coord_t));
    high = malloc(sizeof(coord_t));

    /* Set min */
    Coord_Set(ll, LASHeader_GetMinX(header), LASHeader_GetMinY(header), LASHeader_GetMinZ(header));
    Coord_Set(ur, LASHeader_GetMaxX(header), LASHeader_GetMaxY(header), LASHeader_GetMaxZ(header));
    /* Scale/offset coordinates */
    Coord_Encode(&bounds.low, ll);
    Coord_Encode(&bounds.high, ur);
    //bounds->low = low;
    //bounds->high = high;
}
int Header_Set(uint32_t pnt_count, bound_t bounds, char fname, proj_t proj_str) {

}




hid_t ProjType_create(herr_t status) {
    hid_t projtype;
    projtype = H5Tcopy(H5T_C_S1);
    return projtype;
}

void ProjType_destroy(hid_t projtype, herr_t status) {
    status = H5Tclose(projtype);
}

hid_t BoundType_create(herr_t status) {
    hid_t boundtype, coordtype;
    coordtype = CoordType_create(status);
    boundtype = H5Tcreate(H5T_COMPOUND, sizeof(bound_t));
    status = H5Tinsert(boundtype, "low", HOFFSET(bound_t, low), coordtype);
    status = H5Tinsert(boundtype, "high", HOFFSET(bound_t, high), coordtype);
    status = H5Tclose(coordtype);
    return boundtype;
}

void BoundType_destroy(hid_t boundtype, herr_t status) {
    status = H5Tclose(boundtype);
}

hid_t HeaderType_create(herr_t status) {
    hid_t boundtype, projtype, headertype;
    boundtype = BoundType_create(status);
    projtype = ProjType_create(status);
    headertype = H5Tcreate(H5T_COMPOUND, sizeof(header_t));
    status = H5Tinsert(headertype, "bounds", HOFFSET(header_t, bounds), boundtype);
    status = H5Tinsert(headertype, "proj", HOFFSET(header_t, proj), projtype);
    status = H5Tinsert(headertype, "name", HOFFSET(header_t, path), H5T_C_S1);
    BoundType_destroy(boundtype, status);
    ProjType_destroy(projtype, status);
    return headertype;
}

void HeaderType_destroy(hid_t headertype, herr_t status) {
    status = H5Tclose(headertype);
}

int createHeaderDataset(char* file, char* dataset, hsize_t dims[2])
{
    hid_t file_id, dataset_id, dataspace_id, plist_id, projtype, boundtype, headertype; /* H5 identifiers */
    hsize_t max_dims[2];
    hsize_t chunk_dims[2];
    herr_t status;

    int rank = 2;
    // TODO: Implement the header writing functionality 

    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    /* Open an existing H5 file, if it doesn't exist, return an error */
    file_id = H5Fcreate(file, H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);

    /* Create HDF5 headertype definition for dataset */
    headertype = HeaderType_create(status);

    /** Set the maximum size, should be an unlimited length 1-d array **/
    max_dims[0] = H5S_UNLIMITED;
    max_dims[1] = 1;

    /** Set the chunking dimensions, should be tested for perfomance **/
    chunk_dims[0] = 10000;
    chunk_dims[1] = 1;
    /* Create the dataspace for the dataset */
    dataspace_id = H5Screate_simple(rank, dims, max_dims);

    /* Set the chunking definitions for the dataset */
    plist_id = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(plist_id, rank, chunk_dims);
    
    dataset_id = H5Dcreate(file_id, dataset, headertype, dataspace_id,
            H5P_DEFAULT, plist_id, H5P_DEFAULT);

    /* End access to property list id */
    status = H5Pclose(plist_id);

    /* End access to the dataset and release resources used by it */
    status = H5Dclose(dataset_id);

    /* Terminate access to the dataspace */
    status = H5Sclose(dataspace_id);

    /* Close Data type references */
    //IdxType_destroy(idxtype, status);
    HeaderType_destroy(headertype, status);
    /* Close the file */
    status = H5Fclose(file_id);

    return 0;
}


int readHeaderBlock(char paths[], int offset, int count, header_t* headers)
{
    int i; // counter
    int strlen = PATH_MAX + 1;
    char* fpath;
    fpath = malloc(sizeof(char) * strlen);
    //char* projStr;
    //projStr = malloc(sizeof(char) * strlen);
    LASReaderH reader = NULL;
    LASHeaderH header = NULL;
    LASSRSH srs = LASHeader_GetSRS(header);
    coord_dbl_t ll, ur; // min/max
    coord_t low, high;
    bound_t bounds;
    herr_t status;
    hid_t headertype;
    for (i = 0; i < count; i++) {
        /* Set reader and header null to allow for error checking */
        reader = NULL;
        header = NULL;

        strncpy(fpath, &paths[(strlen) * i], strlen);
        printf("Opening file %s\n", fpath);
        reader = LASReader_Create(fpath);
        if (!reader) {
            LASError_Print("Count not open file to read");
            exit(1);
        }
        header = LASReader_GetHeader(reader);
        if (!header) {
            LASError_Print("Could not fetch header");
            exit(1);
        }
        /* Get the point count */
        int pntCount = LASHeader_GetPointRecordsCount(header);
        /* Get the projection definition */
        //strncpy(projStr, LASSRS_GetProj4(srs), strlen);
        /* Get the boundary values */
        Bound_Set(header, headers[i].bounds);
        Proj_Set(header, headers[i].proj);
        strncpy(headers[i].path, fpath, strlen);
        //headers[i].path = fpath;
        
        headers[i].bounds = bounds;
        headers[i].pnt_count = pntCount;
    }


    return 0;
}


//int writeHeaderBlock(char* file, char* dataset, hsize_t offset[2], hsize_t block[2], header_t* headers, MPI_Comm comm, MPI_Info info) 
int writeHeaderBlock(char* file, char* dataset, hsize_t offset[2], hsize_t block[2], header_t* headers, MPI_Comm comm, MPI_Info info)
{
    hid_t file_id, dset_id, fspace_id, headertype, memspace_id, plist_id;
    herr_t status;
    hsize_t rank = 2;
    hsize_t stride[] = {1, 1};
    hsize_t count[] = {1, 1};

    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    /** Set parallel reading property flag **/
    H5Pset_fapl_mpio(plist_id, comm, info);
    /** Open file for reading **/
    file_id = H5Fopen(file, H5F_ACC_RDWR | H5F_ACC_DEBUG, plist_id);
    /** Open Dataset **/
    plist_id = H5Pcreate(H5P_DATASET_ACCESS);
    dset_id = H5Dopen(file_id, dataset, plist_id);

    fspace_id = H5Dget_space(dset_id);

    memspace_id = H5Screate_simple(rank, block, NULL);

    headertype = HeaderType_create(status);

    status = H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, offset, stride, count, block);
    /** Set flag for parallel writing **/
    plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE);

    status = H5Dwrite(dset_id, headertype, memspace_id, fspace_id, plist_id, headers);

    status = H5Dclose(dset_id);
    status = H5Sclose(fspace_id);
    status = H5Sclose(memspace_id);
    status = H5Pclose(plist_id);
    HeaderType_destroy(headertype, status);
    status = H5Fclose(file_id);
    return 0;
}

