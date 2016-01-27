#include "mpi.h"
#include "hdf5.h"
#include "reader.h"
#include "header.h"
#include "point.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <proj_api.h>


/* Tests to run: 
   check that file exists
   check the number of points to be read
   check if the file has already been read
*/
/* Future
   Load datasets based on bounds
   Sort datasets
   Grid datasets
*/

int readHeaders(int start, int numHeaders, header_t* headers, char* filename)
{
    hid_t file_id, dset_id, fspace_id, plist_id, headertype;
    herr_t status;
    hsize_t offset, block, stride, count;
    header_t* headers;
    /* Allocate memory to hold the headers */
    headers=malloc(sizeof(header_t)*numHeaders);
    // For now give these hard coded values 
    stride = 1;
    count = 1;
    // Get the offset and block from the parameters 
    offset = start;
    block = numHeaders;
    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    /* Open an existing file, if it doesn't exist, return error */
    // MAY NEED TO ADD THE PARALLEL PLIST FLAG HERE for later
    file_id = H5Fopen(filename, H5F_ACC_RDONLY | H5F_ACC_DEBUG, plist_id);
        
    /* Create the header type to read the files */
    headertype HeaderType_create(status);
    

    /* Open the dataset */
    plist_id =  H5Pcreate(H5P_DATASET_ACCESS);
    char* dataset_name = "/headers";
    dset_id = H5Dopen(file_id, dataset_name, plist_id);

    /** Get the data space **/
    fspace_id = H5Dget_space(dset_id);

    /* Get the headers */
    status = H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, &offset, &stride, &count, &block);

    /* Read the headers */
    status = H5Dread(dset_id, headertype, H5S_ALL, fspace_id, H5P_DEFAULT, headers[0]);

    status = H5Dclose(dset_id);
    status = H5Sclose(fspace_id);
    status = H5Pclose(plist_id);
    HeaderType_destroy(headertype, status);

    status = H5Fclose(file_id);

    return 0;
}

int createPointDataset(header_t* header, char* filename)
{
    /** TODO: create some type of name for the dataset that will 
      let it be easily referenced **/
    // A header ID could probably be well suited for this (something that will
    // reflect the array index of the data
    hid_t file_id, dset_id, dataspace_id, plist_id, pointtype;
    hsize_t dim = header->pnt_count;
    herr_t status;
    // points are kept in 1-D array
    int rank = 1;
    
    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    /* Open an existing HDF5 file, error if doesn't exist */
    file_id = H5Fopen(filename, H5F_ACC_RDWR | H5F_ACC_DEBUG, plist_id);

    pointtype = PointType_create(status);
    /* Create the space for the dataset */
    // Currently don't need this to be unlimited as we know how many
    // points are there
    dataspace_id = H5Screate_simple(rank, &dim);
    
    char* dataset_name[12] = sprintf("pt_%08d", header.id);

    plist_id = H5Pcreate(H5P_DATASET_CREATE);
    dataset_id = H5Dcreate(file_id, dataset_name, pointtype, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);

    /* Clean up */
    status = H5Pclose(plist_id);
    status = H5Dclose(dataset_id);
    status = H5Sclose(dataspace_id);
    PointType_destroy(pointtype, status);
    status = H5Fclose(file_id);
    return 0;
}

