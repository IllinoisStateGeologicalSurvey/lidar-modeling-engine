/* 
 * This example writes dataset using chunking. Each process writes exactly
 * one chunk
 *
 * Number of processes is assumed to be 4
 */

#include "mpi.h"
#include "hdf5.h"
#include "stdlib.h"
#include "math.h"

#define H5FILE_NAME     "SDS_chunk.h5"
#define DATASETNAME     "PointArray"
#define NX              5746324        /* Dataset dimensions */
#define NY              3          
#define RANK            2



/** Note: offset is being assigned bad values if there is only 1 process, need to
  fix the mpi segmentation **/
int main(int argc, char **argv)
{
    /*
     * HDF5 APIs definitions
     */
    hid_t       file_id, dset_id;        /* file and dataset identifiers */
    hid_t       filespace, memspace;     /* file and memory dataspace identifiers */
    hsize_t     dimsf[2];                /* dataset dimensions */
    hsize_t     max_dims[2];            /* Maximum dimensions */
    hsize_t     chunk_dims[2];          /* Chunk dimensions */
    int         *data;                   /* pointer to data buffer to write */
    hsize_t     count[2];               /* hyperslab selection parameters */
    hsize_t     stride[2];
    hsize_t     block[2];
    hsize_t     offset[2];
    hid_t       plist_id;               /* property list identifier */
    int         i;
    herr_t      status;
    int remainder;
    hsize_t chunk_X;
    /*
     * MPI variables
     */
    int mpi_size, mpi_rank;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;

    /* Initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &mpi_size);
    MPI_Comm_rank(comm, &mpi_rank);

    /* Check if this is being called in parallel */
    if (mpi_size > 1) {
        printf("Running in parallel, %d processes called.\n", mpi_size);
        chunk_dims[0] = floor(NX/mpi_size);
        remainder = NX % mpi_size;
        if (remainder) {
            chunk_dims[0]++;
        }
    } else {
        chunk_dims[0];
        offset[0]=0;
    }
    // Flag to signal processes with larger dimensions
    int isBig;
    if (remainder != 0) {
        if ((mpi_rank + 1) <= remainder) {
            printf("Process %d has %d values leftover\n", mpi_rank, remainder);
            isBig = 1;
            offset[0] = (mpi_rank * chunk_dims[0]);

            printf("offset:%d = (%d * (%d + 1))", offset[0], mpi_rank, chunk_dims[0]);

        } else {
            isBig = 0;
            offset[0] = (mpi_rank * chunk_dims[0]) - ((mpi_rank + 1) - remainder);
        }
            

    } else {
        printf("Datasets chunks evenly\n");
    }


    /*
     * Set up file access property list with parallel I/O access
     */
    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_mpio(plist_id, comm, info);

    /*
     * Create a new file collectively and release property list identifier
     */
    file_id = H5Fcreate(H5FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);
    H5Pclose(plist_id);

    /*
     * Create a dataspace for the dataset
     */
    dimsf[0] = NX;
    dimsf[1] = NY;
    max_dims[0] = H5S_UNLIMITED;
    max_dims[1] = H5S_UNLIMITED;
    /* Set the dataspace to take the max chunk size */
    chunk_dims[1] = 3;
    filespace = H5Screate_simple(RANK, dimsf, max_dims);
    /*
     * Create chunked dataset
     */
    plist_id = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(plist_id, RANK, chunk_dims);
   
    /* Alloate the necessary memory space 
    if ((mpi_size > 1) && isBig) {
        chunk_dims[0] = chunk_X;
    }*/

    dset_id = H5Dcreate(file_id, DATASETNAME, H5T_NATIVE_INT, filespace,
                        H5P_DEFAULT, plist_id, H5P_DEFAULT);
    H5Pclose(plist_id);
    H5Sclose(filespace);

    /*
     * Each process defines dataset in memory and writes it to the
     * hyperslab in the file
     */
    // remainder
    fprintf(stderr, "Process %d: writing %d points with %d offset\n", mpi_rank, chunk_dims[0], offset[0]);

    count[0] = 1;
    count[1] = 1;
    stride[0] = 1;
    stride[1] = 1;
    if (remainder && !isBig) {
        printf("Process %d is small chunk\n", mpi_rank);
        block[0] = chunk_dims[0] - 1;
    } else {
        block[0] = chunk_dims[0];
    }
    block[1] = chunk_dims[1];
    
    offset[1] = 0; 
   // offset[0] = mpi_rank*chunk_X;

    memspace = H5Screate_simple(RANK, block, NULL);

    printf("Process %d has %d points and %d offset\n", mpi_rank, block[0], offset[0]);
    /*
     * Select hyperslab in the file.
     */
    filespace = H5Dget_space(dset_id);
    status = H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, stride, count, block);

    /*
     * Initialize data buffer
     */
    data = (int *) malloc(sizeof(int)*block[0]*block[1]);
    for (i=0; i < (int)block[0]*block[1]; i++) {
        data[i] = mpi_rank + 1;
    }

    /*
     * Create property list for collective dataset write
     */
    plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE);

    status = H5Dwrite(dset_id, H5T_NATIVE_INT, memspace, filespace,
                        plist_id, data);

    free(data);

    /*
     * Close resources
     */
    H5Dclose(dset_id);
    H5Sclose(filespace);
    H5Sclose(memspace);
    H5Pclose(plist_id);
    H5Fclose(file_id);

    MPI_Finalize();

    return 0;
}
