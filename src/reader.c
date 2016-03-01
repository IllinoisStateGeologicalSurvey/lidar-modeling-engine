#include "mpi.h"
#include "reader.h"
#include "header.h"
#include "point.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <hdf5.h>
#include <proj_api.h>
#include <liblas/capi/liblas.h>

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
hsize_t Headers_count(hid_t file_id) {
	hid_t dset_id, fspace_id, plist_id, headertype;
	herr_t status;
	hsize_t nVals;
	/* Create the headertype for reading */
	headertype = HeaderType_create(&status);

	plist_id = H5Pcreate(H5P_DATASET_ACCESS);
	char dataset_name[10] = "/headers";
	dset_id = H5Dopen(file_id, &dataset_name[0], plist_id);
	/* Get the data space */
	fspace_id = H5Dget_space(dset_id);
	
	nVals = H5Sget_simple_extent_npoints(fspace_id);

	printf("Dataspace has %zd headers to read\n", (size_t)nVals);

	HeaderType_destroy(headertype, &status);
	return nVals;

}
/** This function will read all of the headers from a given HDF file **/
int Headers_read(header_t* headers, hid_t file_id) {
	hid_t dset_id, fspace_id, plist_id, headertype;
	herr_t status;
	
	/* create the headertype for reading */
	headertype = HeaderType_create(&status);

	plist_id = H5Pcreate(H5P_DATASET_ACCESS);
	char dataset_name[10] = "/headers";
	dset_id = H5Dopen(file_id, &dataset_name[0], plist_id);
	fspace_id = H5Dget_space(dset_id);

	status = H5Sselect_all(fspace_id);
	/* Read the headers */
	status = H5Dread(dset_id, headertype, H5S_ALL, fspace_id, H5P_DEFAULT, &headers[0]);
	
	status = H5Dclose(dset_id);
	status = H5Sclose(fspace_id);
	status = H5Pclose(plist_id);
	HeaderType_destroy(headertype, &status);
	
	return 0;
}
/** This will read a hyperslab of headers from an HDF file **/
int HeaderSet_read(int start, int numHeaders, header_t* headers, char* filename)
{
    hid_t file_id, dset_id, fspace_id, plist_id, headertype;
    herr_t status;
    hsize_t offset, block, stride, count;
    //header_t* headers;
    /* Allocate memory to hold the headers */
    //headers=malloc(sizeof(header_t)*numHeaders);
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

    headertype = HeaderType_create(&status);
    

    /* Open the dataset */
    plist_id =  H5Pcreate(H5P_DATASET_ACCESS);
    char* dataset_name = "/headers";
    dset_id = H5Dopen(file_id, dataset_name, plist_id);

    /** Get the data space **/
    fspace_id = H5Dget_space(dset_id);

    /* Get the headers */
    status = H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, &offset, &stride, &count, &block);

    /* Read the headers */
    status = H5Dread(dset_id, headertype, H5S_ALL, fspace_id, H5P_DEFAULT, &headers[0]);

    status = H5Dclose(dset_id);
    status = H5Sclose(fspace_id);
    status = H5Pclose(plist_id);
    HeaderType_destroy(headertype, &status);

    status = H5Fclose(file_id);

    return 0;
}

int HeaderPoint_get(header_t* header, char* dset_name) {
    printf("Header id: %i, path: %s", header->id, header->path);
    sprintf(dset_name, "/pt_%08d", header->id);
    printf(" Dataset name: %s\n", dset_name); 

    return 0;
}
/*  PointSet_prepare:
 *  This function will get the designated dataset name from the index
 *  of the Header in the H5 file.This will also retrieve the LAS filepath 
 *  and the number of points to read from the file.
 *  @param header: a header type describing the file to read
 *  @param filename: the filename for the HDF5 file where the data will be written
 *  @param LASpath: a character array to hold the LAS file path
 *  @param pointBlock: an integer to hold the count of points to read
 */
int PointSet_prepare(header_t* header, char* LASpath, hsize_t* pointBlock) {
    //char* dset_name = malloc(sizeof(char) * (PATH_MAX+1));
    // Get the name for the h5 dataset(based on header index)
    //printf("HEADER_POINT_GET called on : %s", header->path);
    //HeaderPoint_get(header, dset_name);
    // Get the name and point count for the file (used for reading with libLAS)
    strcpy(LASpath, header->path);
    *pointBlock = (hsize_t)header->pnt_count;

    return 0;
}
    

int PointSet_create(header_t* header, hid_t group_id)
{
    /** TODO: create some type of name for the dataset that will 
      let it be easily referenced **/
    // A header ID could probably be well suited for this (something that will
    // reflect the array index of the data
    hid_t dset_id, dataspace_id, plist_id, pointtype;
    hsize_t *dim, *cdim, *max_dim;
    dim = malloc(sizeof(hsize_t));
    cdim = malloc(sizeof(hsize_t));
    max_dim = malloc(sizeof(hsize_t));
    herr_t status;
    // points are kept in 1-D array
    int rank = 1; 
    *dim = header->pnt_count;
	char dataset_name[12];
	sprintf(dataset_name, "%i", (int)header->id);

    /* Open an existing HDF5 file, error if doesn't exist */
    //plist_id = H5Pcreate(H5P_FILE_ACCESS);
    //file_id = H5Fopen(filename, H5F_ACC_RDWR | H5F_ACC_DEBUG, plist_id);
	//printf("SUCCESSFULLY OPENED %s\n", filename); 
    // Create the HDF point data type
    pointtype = PointType_create(&status);
    /* Create the space for the dataset */
    // Currently don't need this to be unlimited as we know how many
    // points are there
    dataspace_id = H5Screate_simple(rank, dim, dim);
    
    //Extract the dataset name to use for the points
    //HeaderPoint_get(header, dataset_name);
    fprintf(stderr, "Header[%i]: DATASET NAME: %s\n", header->id, dataset_name);
    //sprintf(dataset_name, "pt_%08d", header->id);

    //TESTING: set deflate compression for points, may need to modifty read/write for compression
    plist_id = H5Pcreate(H5P_DATASET_CREATE);
    *cdim = 100;
    status = H5Pset_chunk(plist_id, 1, cdim);
    //status = H5Pset_deflate(plist_id, 6);

    dset_id = H5Dcreate(group_id, dataset_name, pointtype, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);
    fprintf(stderr, "[SUCCESS]: Created dataset: %s\n", dataset_name);
    /* Clean up */
    // Close the propery lise
    status = H5Pclose(plist_id);
    // Close the dataset
    status = H5Dclose(dset_id);
    // Close the dataspace
    status = H5Sclose(dataspace_id);
    // CLose data type references
    PointType_destroy(pointtype, &status);
    // CLose the file
    //status = H5Fclose(file_id);
    free(dim);
    free(cdim);
    free(max_dim);
 //   free(dataset_name);
    printf("Dataset cleanup finished\n");
    return 0;
}

int PointSet_write(hid_t file_id, char* dataset, hsize_t* offset, hsize_t* block, Point* points, MPI_Comm comm, MPI_Info info) {
    hid_t dset_id, fspace_id, pointtype, memspace_id, plist_id;
    herr_t status;
    int rank = 1;
    hsize_t stride = 1;
    hsize_t count = 1;
    int mpi_rank;
    MPI_Comm_rank(comm, &mpi_rank);
    
    // Open the file
    printf("[%i] PointSet_write called\n", mpi_rank);
    //plist_id = H5Pcreate(H5P_FILE_ACCESS);
    //printf("Creating parallel file access flag\n");
    //H5Pset_fapl_mpio(plist_id, comm, info); // TODO: Find out why this is erroring

    //printf("[%i] Opening file %s\n", mpi_rank, file);
    //file_id = H5Fopen(file, H5F_ACC_RDWR | H5F_ACC_DEBUG, plist_id);
    //Access the dataset
    //printf("[%i] H5 File opened: %s \n", mpi_rank, file);
    /** Open the dataset **/
    plist_id = H5Pcreate(H5P_DATASET_ACCESS);
    dset_id = H5Dopen(file_id, dataset, plist_id);
    printf("[%i] Dataset opened: %s \n", mpi_rank, dataset);
    // Get the data space
    /** Get the point data space **/
    fspace_id = H5Dget_space(dset_id);
    memspace_id = H5Screate_simple(rank, block, NULL);
    printf("[%i] FileSpace Created\n", mpi_rank);
    
    // Create the HDF Point datatype
    pointtype = PointType_create(&status);
    // Get the hyperslab to write the data into
    status = H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, offset, &stride, &count, block);
    printf("[%i] Hyperslab selected\n", mpi_rank);
    // Set up the dataset for parallel writing
    plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE);
    //H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_INDEPENDENT);
    // Write the data
    status = H5Dwrite(dset_id, pointtype, memspace_id, fspace_id, plist_id, points);
    printf("[%i] Dataset written\n", mpi_rank);
    //Clean up
    // Free the point data type
    //printf("[%i] Closing up file %s\n", mpi_rank, file);
    //Close out the file
    
    status = H5Dclose(dset_id);
    status = H5Sclose(fspace_id);
    status = H5Sclose(memspace_id);
    status = H5Pclose(plist_id);
    PointType_destroy(pointtype, &status);
    //checkOrphans(file_id, mpi_rank);
	MPI_Barrier(comm);
 	//status = H5Fclose(file_id);
    return 0;
}

int PointSet_copy(char* LASpath, char* dataset_name, hsize_t* pointBlock, hid_t group_id,  MPI_Comm comm, MPI_Info info) {
    Point* points;
    hsize_t pointCnt = *pointBlock;
    hsize_t pointOffset=0;
    printf("Allocating space for %i points\n", (int)pointCnt);
    printf("Allocating %zu bytes\n", (size_t)(sizeof(Point) * pointCnt));
    points = malloc(sizeof(Point) * pointCnt);
    printf("Finished allocating points\n");
    LASReaderH lasReader;
    int mpi_rank;
    MPI_Comm_rank(comm, &mpi_rank);
    lasReader = LASReader_Create(LASpath);
    printf("[%d]Beginning copy of points from %s\n", mpi_rank, LASpath);
    // CHeck that the file was properly opened
    if (!lasReader) {
        LASError_Print("Could not open the file for reading\n");
        exit(1);
    }
    MPI_Barrier(comm);
    printf("[%d] Reading file %s\n", mpi_rank, LASpath);
    LASFile_read(lasReader, &pointOffset, &pointCnt, points, mpi_rank);
    printf("[%d] Successfully read file: %s\n", mpi_rank, LASpath);
    MPI_Barrier(comm);
    //printf("[%d] POINT_SET_WRITE called on: %s\n", mpi_rank, h5_dset);
    PointSet_write(group_id, dataset_name, &pointOffset, pointBlock, points, comm, info);
    free(points);
    return 0;
}


int LASFile_read(LASReaderH reader, hsize_t* offset, hsize_t* count, Point* points, int mpi_rank)
{
    hsize_t i; // counter
    LASPointH p = NULL;
    LASHeaderH header = NULL;
    printf("[%i] Beginning LAS File reading\n", mpi_rank);
    header = LASReader_GetHeader(reader);
    printf("[%i] Header opened\n",mpi_rank);
    /* Check that the point count is not larger than file */
//int end = count;
    printf("[%i] Sanity check: header offset: %i, pntcount: %i\n",mpi_rank,(int)*offset,  (int)*count);
    printf("[%i]Number of points to read is %i\n", mpi_rank,LASHeader_GetPointRecordsCount(header));
    int pntCnt = *count;
    int off = *offset;
    int end = (int)(*offset + pntCnt);
    printf("[%i] End:%i = %i", mpi_rank, end, LASHeader_GetPointRecordsCount(header));
    if (end > LASHeader_GetPointRecordsCount(header)) {
        fprintf(stdout, "ERROR: Point list out of file bounds\n");
        exit(1);
    }
    double *x, *y, *z;
    printf("[%d] Allocating space for %i doubles\n", mpi_rank, pntCnt);
    x = malloc(sizeof(double) * pntCnt);
    y = malloc(sizeof(double) * pntCnt);
    z = malloc(sizeof(double) * pntCnt);
    

    // Run GetPointAt outside loop as it is considerably slower than getNextPoint
    /* TODO: Update flow control so that i doesnt increment when the return is < returnTotal,
       This will prevent empty rows from being written */
    printf("[%d] Reading points\n",mpi_rank);
    for (i = 0; i < pntCnt; i++)  {
        //printf("Point:%d\n", i);
        if (i == 0) {
            p = LASReader_GetPointAt(reader, off);
            if (!p) {
                LASError_Print("Could not read point\n");
            }
        
        } else {
            p = LASReader_GetNextPoint(reader);
        }
        // For now read only last returna
        LASPoint_read(&p, &points[i], &x[i], &y[i], &z[i]);
    }

    //printf("%9.6f, %9.6f, %9.6f\n", x[0], y[0], z[0]);
    printf("[%d] Projecting Points\n", mpi_rank);
    // Projecting points is faster if they are done at the same time, using an array and count
    /* Initialize projection parameters */
    LASPoint_project(&header, count, &x[0], &y[0], &z[0], &points[0], mpi_rank);
    printf("[%d] Cleaning up\n", mpi_rank); 
    //Point_print(&points[0]);

    free(x);
    free(y);
    free(z);
    
    LASHeader_Destroy(header);
    header = NULL;

    return 0;
}
/** HDF5 Utility to check for any unclosed property lists */
int checkOrphans(hid_t file_id, int mpi_rank) {
	size_t norphans = H5Fget_obj_count(file_id, H5F_OBJ_ALL);
	fprintf(stderr,"Number of open objects is %zd\n", norphans);
	if (norphans > 1) { /* expect 1 for the file we haven't closed */
		int i;
		H5O_info_t info;
		char name[64];
		hid_t * objects = calloc(norphans, sizeof(hid_t));
		H5Fget_obj_ids(file_id, H5F_OBJ_ALL, -1, objects);
		for (i=0; i<norphans;i++) {
			H5Oget_info(objects[i], &info);
			H5Iget_name(objects[i], name, 64);
			fprintf(stderr,"[%i] %d of %zd things still open: %d with name %s of type %d\n", mpi_rank, i, norphans, objects[i], name, info.type);
		}
	}
	return 0;
}

