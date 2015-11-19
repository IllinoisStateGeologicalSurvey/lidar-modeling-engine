/* 
 * This example writes dataset using chunking. Each process writes exactly
 * one chunk
 *
 * Number of processes is assumed to be 4
 */

#include "mpi.h"
#include "hdf5.h"
#include "stdlib.h"

#define H5FILE_NAME     "SDS_chunk.h5"
#define DATASETNAME     "PointArray"
#define NX              1000        /* Dataset dimensions */
#define NY              3          
#define CH_NX           10           /* Chunk dimensions */
#define CH_NY           3
#define RANK            2

int main(int argc, char **argv)
{
    /*
     * HDF5 APIs definitions
     */
    hid_t       file_id, dset_id;        /* file and dataset identifiers */
    hid_t       filespace, memspace;     /* file and memory dataspace identifiers */
    hsize_t     dimsf[2];                /* dataset dimensions */
    hsize_t     chunk_dims[2];          /* Chunk dimensions */
    int         *data;                   /* pointer to data buffer to write */
    hsize_t     count[2];               /* hyperslab selection parameters */
    hsize_t     stride[2];
    hsize_t     block[2];
    hsize_t     offset[2];
    hid_t       plist_id;               /* property list identifier */
    int         i;
    herr_t      status;

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
    /*
     * Exit if number of processes is not 4
     */
    /*if (mpi_size != 4) {
        printf("Number of processes is %d and must be equal to 4\n", mpi_size);
        printf("Quitting...\n");
        return 0;
    }*/
    hsize_t chunk_X = NX/mpi_size;
    int leftOver = NX % mpi_size;
    if (leftOver != 0) {
        printf("Process has %d values leftover\n", leftOver);
    } else {
        printf("Datasets chunks evenly");
    }

    printf("Points per chunk is %d\n", chunk_X);

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
    chunk_dims[0] = chunk_X;
    chunk_dims[1] = CH_NY;
    filespace = H5Screate_simple(RANK, dimsf, NULL);
    memspace = H5Screate_simple(RANK, chunk_dims, NULL);

    /*
     * Create chunked dataset
     */
    plist_id = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(plist_id, RANK, chunk_dims);
    dset_id = H5Dcreate(file_id, DATASETNAME, H5T_NATIVE_INT, filespace,
                        H5P_DEFAULT, plist_id, H5P_DEFAULT);
    H5Pclose(plist_id);
    H5Sclose(filespace);

    /*
     * Each process defines dataset in memory and writes it to the
     * hyperslab in the file
     */

    count[0] = 1;
    count[1] = 1;
    stride[0] = 1;
    stride[1] = 1;
    block[0] = chunk_dims[0];
    block[1] = chunk_dims[1];
    
    offset[0] = mpi_rank*chunk_X;
    offset[1] = 0; 
    /*if (mpi_rank == 0) {
        offset[0] = 0;
        offset[1] = 0;
    }
    if (mpi_rank == 1) {
        offset[0] = 0;
        offset[1] = chunk_dims[1];
    }
    if (mpi_rank == 2) {
        offset[0] = chunk_dims[0];
        offset[1] = 0;
    }
    if (mpi_rank == 3) {
        offset[0] = chunk_dims[0];
        offset[1] = chunk_dims[1];
    }*/

    /*
     * Select hyperslab in the file.
     */
    filespace = H5Dget_space(dset_id);
    status = H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, stride, count, block);

    /*
     * Initialize data buffer
     */
    data = (int *) malloc(sizeof(int)*chunk_dims[0]*chunk_dims[1]);
    for (i=0; i < (int)chunk_dims[0]*chunk_dims[1]; i++) {
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
