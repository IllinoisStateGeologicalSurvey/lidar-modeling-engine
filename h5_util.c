#include "hdf5.h"

int createDataset(char* file, char* dset) {
    hid_t       file_id, dataset_id, dataspace_id; /*identifiers */
    hsize_t     dims[2], max_dims[2];
    herr_t      status;
    hsize_t     RANK;

    /* Create a new file using default parameters */
    file_id = H5Fcreate(file, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    /* Create the dataspace needed for the dataset */
    dims[0] = 1000;
    dims[1] = 8;
    max_dims[0] = H5S_UNLIMITED;
    max_dims[1] = 8;

    dataspace_id = H5Screate_simple(RANK, dims, max_dims);
    
    dataset_id = H5D_create(file_id, dset, H5T_STD_I32BE, dataspace_id,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    /* End access to the dataset and release resources */
    status = H5Dclose(dataset_id);
    
    /* terminate acces to the dataspace */
    status = H5Sclose(dataspace_id);

    /* Close the file */
    status = H5Fclose(file_id);
    
    return 0;
}
    
