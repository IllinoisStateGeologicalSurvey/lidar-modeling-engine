/** Copyright HDF Group **/

/** 
  * This example illustrates how to create a dataset that is a 
  *4x6 array. It is used in the HDF5 Tutorial.
  */

#include "hdf5.h"
#define FILE "dset.h5"

int main() {

    hid_t       file_id, dataset_id, dataspace_id; /* identifiers */
    hsize_t     dims[2];
    herr_t      status;

    /* Create a new file using default properties */
    file_id = H5Fcreate(FILE, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    /* Create the data space for the dataset */
    dims[0] = 4;
    dims[1] = 6;
    dataspace_id = H5Screate_simple(2, dims, NULL);
    
    dataset_id = H5Dcreate2(file_id, "/dset", H5T_STD_I32BE, dataspace_id,
                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    /* End access to the dataset and release resources used by it */
    status = H5Dclose(dataset_id);

    /* Terminate acces to the data space */
    status = H5Sclose(dataspace_id);

    /* Close the file */
    status = H5Fclose(file_id);
}
