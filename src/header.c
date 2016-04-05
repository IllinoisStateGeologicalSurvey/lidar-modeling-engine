#include "mpi.h"
#include <hdf5.h>
#include "header.h"
#include "point.h"
#include "file_util.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <inttypes.h>
#include <liblas/capi/liblas.h>
#include <proj_api.h>

//size_t strlen(const char *str) {
//	const char *s;
//	for (s = str; *s; ++s);
//	return(s - str);
//}

int Proj_Set(LASHeaderH header, proj_t* proj) {
    //printf("Proj_Set called\n");
    LASSRSH srs = NULL;
	srs = LASHeader_GetSRS(header);
	char* proj4_text = NULL;
    if (!srs) {
    	printf("Failed to read Reference System\n");
    }
	size_t projLen = sizeof(LASSRS_GetProj4(srs)) + 1;
	
    proj4_text = (char *) malloc(projLen * sizeof(char));
    proj4_text = LASSRS_GetProj4(srs);
	//printf("Projection is %s \n", proj4_text);
    //printf("SRS has length %zd\n", projLen);
    //printf("Projection received\n");
	//strcpy(&proj->proj4[0], proj4_text);
	strncpy(&proj->proj4[0], proj4_text, PATH_LEN);
	
	//printf("Cleaning up\n");
	//LASString_Free(proj4_text);
	free(proj4_text);
	if (srs != NULL) {
		LASSRS_Destroy(srs);
	}
    return 0;
}

/*
int Bound_Set(LASHeaderH header, bound_t* bounds) {
    //coord_t *low, *high;
    //coord_t *low = malloc(sizeof(coord_t));
    //coord_t *high = malloc(sizeof(coord_t));
    coord_dbl_t* ur = malloc(sizeof(coord_dbl_t));
    coord_dbl_t* ll = malloc(sizeof(coord_dbl_t));

    Coord_Set(ll, LASHeader_GetMinX(header), LASHeader_GetMinY(header), LASHeader_GetMinZ(header));
    Coord_Set(ur, LASHeader_GetMaxX(header), LASHeader_GetMaxY(header), LASHeader_GetMaxZ(header));
    Coord_Encode(&bounds->low, ll);
    Coord_Encode(&bounds->high, ur);
    //printf("LL.x is %"PRIu32"\n", &bounds.low.x);
    //bounds->low = low;
    free(ur);
    free(ll);
    //bounds->high = high;
    return 0;
}

int Bound_intersects(bound_t* bound_1, bound_t* bound_2) {
	if ((bound_1->high.x < bound_2->low.x) ||(bound_1->high.y < bound_2->low.y)) {
		// Bound 2 is above/right of bound 1
		return 0;
	} 
	else if ((bound_2->high.x < bound_2->low.x) || (bound_2->high.y < bound_1->low.y))
	{
		// Bound 1 is above right of bound 2
		return 0;
	} else {
		//Bounds intersect!
		return 1;
	}
}
*/
int Header_Set(uint32_t pnt_count, bound_t bounds, char fname, proj_t proj_str) {
	return 0;
}

void Header_free(header_t* headers, int n) {
    int i;
    for (i = 0; i < n; i++) {
        free(&headers[i].id);
        free(&headers[i].pnt_count);
        free(&headers[i].bounds);
        free(&headers[i].path);
        free(&headers[i].proj);
    }
    free(headers);
}



hid_t ProjType_create(herr_t* status) {
    hid_t projtype;
    projtype = H5Tcopy(H5T_C_S1);
    *status = H5Tset_size(projtype, PATH_LEN);
    return projtype;
}

void ProjType_destroy(hid_t projtype, herr_t* status) {
    *status = H5Tclose(projtype);
}
/*
hid_t BoundType_create(herr_t *status) {
    hid_t boundtype;
    hid_t coordtype;
    coordtype = CoordType_create(status);
    boundtype = H5Tcreate(H5T_COMPOUND, sizeof(bound_t));
    *status = H5Tinsert(boundtype, "low", HOFFSET(bound_t, low), coordtype);
    *status = H5Tinsert(boundtype, "high", HOFFSET(bound_t, high), coordtype);
    *status = H5Tclose(coordtype);
	
    return boundtype;
}

void BoundType_destroy(hid_t boundtype, herr_t* status) {
    *status = H5Tclose(boundtype);
    
}
*/
hid_t HeaderType_create(herr_t* status) {
    hid_t coordtype, boundtype, projtype, headertype;
    coordtype = CoordType_create(status);
    boundtype = BoundType_create(status);
    projtype = ProjType_create(status);
    headertype = H5Tcreate(H5T_COMPOUND, sizeof(header_t));
    *status = H5Tinsert(headertype, "id", HOFFSET(header_t, id), H5T_NATIVE_UINT);
    *status = H5Tinsert(headertype, "bounds", HOFFSET(header_t, bounds), boundtype);
    *status = H5Tinsert(headertype, "name", HOFFSET(header_t, path), projtype);
    *status = H5Tinsert(headertype, "proj", HOFFSET(header_t, proj), projtype);
    *status = H5Tinsert(headertype, "pnt_count", HOFFSET(header_t, pnt_count), H5T_NATIVE_UINT);
    BoundType_destroy(boundtype, status);
    ProjType_destroy(projtype, status);
    CoordType_destroy(coordtype, status);
    return headertype;
}

void HeaderType_destroy(hid_t headertype, herr_t* status) {
    *status = H5Tclose(headertype);
}
/*
int MPI_BoundType_create(MPI_Datatype *mpi_boundtype) {
    int nitems=2;
    int blocklengths[2] = {1,1};
    MPI_Datatype types[2];
    // Set the member types
    MPI_CoordType_create(&types[0]);
    MPI_CoordType_create(&types[1]);
    MPI_Aint offsets[2];
    offsets[0] = offsetof(bound_t, low);
    offsets[1] = offsetof(bound_t, high);
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, mpi_boundtype);
    MPI_Type_commit(mpi_boundtype);
    printf("Bound type created\n");
    
    return 0;
}
*/
int MPI_ProjType_create(MPI_Datatype *mpi_projtype) {
    int nitems=1;
    int blocklength = PATH_LEN;
    int mpi_err, mpi_err_class, resultlen;
    char err_buffer[MPI_MAX_ERROR_STRING];
    MPI_Datatype type = MPI_CHAR;
    MPI_Aint offset = offsetof(proj_t, proj4);
    mpi_err = MPI_Type_create_struct(nitems, &blocklength, &offset, &type, mpi_projtype);
    if (mpi_err != MPI_SUCCESS) {
        MPI_Error_class(mpi_err, &mpi_err_class);
        MPI_Error_string(mpi_err, err_buffer, &resultlen);
        fprintf(stderr, err_buffer);
        MPI_Finalize();
    }
    MPI_Type_commit(mpi_projtype);
    printf("Proj type created\n");

    return 0;
}

int MPI_HeaderType_create(MPI_Datatype *mpi_headertype) {
    int nitems=5;
    int strLen = PATH_LEN;
    header_t header;
    //int blocklengths[5] = { 1, 1, 1, strLen, 1};
    int blocklengths[5] = {1, 1, 6, strLen, strLen};
    MPI_Datatype types[5];
    int mpi_err, mpi_err_class, resultlen;
    char err_buffer[MPI_MAX_ERROR_STRING];
    types[0] = MPI_UNSIGNED;
    types[1] = MPI_UNSIGNED;
    //MPI_BoundType_create(&types[2]);
    types[2] = MPI_UNSIGNED;
    types[3] = MPI_CHAR;
    //MPI_ProjType_create(&types[4]);
    types[4] = MPI_CHAR;
    MPI_Aint offsets[5];
    MPI_Aint addresses[6];
    MPI_Get_address(&header, &addresses[0]);
    MPI_Get_address(&header.id, &addresses[1]);
    MPI_Get_address(&header.pnt_count, &addresses[2]);
    MPI_Get_address(&header.bounds, &addresses[3]);
    MPI_Get_address(&header.path, &addresses[4]);
    MPI_Get_address(&header.proj, &addresses[5]);

    offsets[0] = addresses[1] - addresses[0];
    offsets[1] = addresses[2] - addresses[0];
    offsets[2] = addresses[3] - addresses[0];
    offsets[3] = addresses[4] - addresses[0];
    offsets[4] = addresses[5] - addresses[0];
    fprintf(stderr, "Nitems: %i\n", nitems);
    fprintf(stderr, "Blocklengths: [ %i, %i, %i, %i, %i]\n", blocklengths[0], blocklengths[1], blocklengths[2], blocklengths[3], blocklengths[4]);
    fprintf(stderr, "Offsets: [ %i, %i, %i, %i, %i]\n", (int)offsets[0], (int)offsets[1], (int)offsets[2], (int)offsets[3], (int)offsets[4]);
    fprintf(stderr, "Types: [%i, %i, %i, %i, %i]\n", types[0], types[1], types[2], types[3], types[4]);
    //offsets[2] = offsetof(header_t, bounds);
    //offsets[3] = offsetof(header_t, path);
    //offsets[4] = offsetof(header_t, proj);
    printf("Creating mpi_header_struct\n");
    /** TODO: Fix this struct creation, application quits here **/
    mpi_err = MPI_Type_create_struct(nitems, blocklengths, offsets, types, mpi_headertype);
    printf("MPI_ERR %i\n", mpi_err);
    if (mpi_err != MPI_SUCCESS) {
        MPI_Error_class(mpi_err, &mpi_err_class);
        MPI_Error_string(mpi_err, err_buffer, &resultlen);
        fprintf(stderr, err_buffer);
        MPI_Finalize();
    }
    printf("header struct created\n");
    mpi_err = MPI_Type_commit(mpi_headertype);
    if (mpi_err != MPI_SUCCESS) {
        MPI_Error_class(mpi_err, &mpi_err_class);
        MPI_Error_string(mpi_err, err_buffer, &resultlen);
        fprintf(stderr, err_buffer);
        MPI_Finalize();
    }
    printf("Header type created\n");
    //MPI_Type_free(&types[2]);
    //MPI_Type_free(&types[4]);

    return 0;
}


int createHeaderDataset(char* file, char* dataset, hsize_t* dims)
{
    hid_t file_id, dataset_id, dataspace_id, plist_id, headertype; /* H5 identifiers */
    hsize_t max_dims;
    hsize_t chunk_dims;
    herr_t status;

    int rank = 1;
    // TODO: Implement the header writing functionality 

    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    /* Open an existing H5 file, if it doesn't exist, return an error */
    printf("Attempting to open %s\n", file);
    file_id = H5Fcreate(file, H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);

    /* Create HDF5 headertype definition for dataset */
    headertype = HeaderType_create(&status);

    /** Set the maximum size, should be an unlimited length 1-d array **/
    max_dims = H5S_UNLIMITED;
    //max_dims[1] = 1;

    /** Set the chunking dimensions, should be tested for perfomance **/
    chunk_dims = 10000;
    //chunk_dims[1] = 1;
    /* Create the dataspace for the dataset */
    dataspace_id = H5Screate_simple(rank, dims, &max_dims);

    /* Set the chunking definitions for the dataset */
    plist_id = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(plist_id, rank, &chunk_dims);
    
    /**For TESTING PURPOSES ONLY */
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
    HeaderType_destroy(headertype, &status);
    /* Close the file */
    status = H5Fclose(file_id);

    return EXIT_SUCCESS;
}

/** This function will read the necessary header data from a given LAS file **/
int Header_read(char* path, header_t* header, uint32_t id) {
	LASReaderH LASreader = NULL;
	LASHeaderH LASheader = NULL;
	//zero-fill the path buffer to get rid of any remaining paths
	// Copy the path 
	strncpy(header->path, path, PATH_LEN);
	header->id = id;
        
	//printf("[%i] Filepath set for file id %i\n", mpi_rank, id);
	printf("Opening LASReader for file %s\n", header->path);
	LASreader = LASReader_Create(path);
	if (!LASreader) {
		fprintf(stderr,"Could not open file: %s\n", path);
		// TODO: NEED A WAY TO HANDLE IO ERRORS
		return 0;
	}
	printf("Opening LASHeader\n");
	LASheader = LASReader_GetHeader(LASreader);
	if (!LASheader) {
		fprintf(stderr, "Could not fetch header for file %s\n", path);
		// TODO: NEED A WAY TO HANDLE IO ERRORS
		return 0;
	}
	header->pnt_count = LASHeader_GetPointRecordsCount(LASheader);
	if (!LASBound_Get(&LASheader, &header->bounds)) {
		fprintf(stderr, "Bound Error: Failed to read Bound from source\n");
		if (LASheader != NULL) {
			LASHeader_Destroy(LASheader);
			LASheader = NULL;
		}
		if (LASreader != NULL) {
			LASReader_Destroy(LASreader);
			LASreader = NULL;
		}
		return 0;
	}
	printf("Bounds set for idx: %i \n", id);
	Proj_Set(LASheader, &header->proj);
	printf("Projection set for id: %i \n", id);
	if (LASheader != NULL) {
		LASHeader_Destroy(LASheader);
		LASheader = NULL;
	}
	if (LASreader != NULL) {
		LASReader_Destroy(LASreader);
		LASreader = NULL;
	}
	return 1;
}

int readHeaderBlock(char paths[], int offset, int block, header_t* headers)
{
    int i; // counter
    int strLen = PATH_LEN;
    char fpath[strLen];

    int blockInt = block;
    uint32_t headerId;
    fprintf(stderr, "Reading %i files\n", blockInt);
    for (i = 0; i < (blockInt); i++) {
        /* Set reader and header null to allow for error checking */
        //printf("[%i]Last file is %s\n", mpi_rank, &paths[(blockInt - 1) * PATH_LEN]);
        memset(&fpath[0], 0, PATH_LEN);
        strncpy(&fpath[0], &paths[PATH_LEN * i], PATH_LEN);
	//snprintf(&fpath[0], PATH_LEN, &paths[PATH_LEN * i]);
        printf("Reading file %i/%i: %s \n", i, blockInt, &fpath[0]);
        
        headerId = offset + i;
        
        if (!Header_read(&fpath[0], &headers[i], headerId)) {
			fprintf(stderr,"Header Error: Failed to read header for %s\n", fpath);
			continue;
		}
        
    }
    return 1;
}
/*int readHeaderBlock(char paths[], int offset, int block, header_t* headers, MPI_Comm comm, int mpi_rank)
{
    int i; // counter
    int strLen = PATH_LEN;
    char fpath[strLen];

    int blockInt = block;
    uint32_t headerId;
    fprintf(stderr, "[%i] Reading %i files\n", mpi_rank, blockInt);
    for (i = 0; i < (blockInt); i++) {
        // Set reader and header null to allow for error checking 
        //printf("[%i]Last file is %s\n", mpi_rank, &paths[(blockInt - 1) * PATH_LEN]);
        memset(&fpath[0], 0, PATH_LEN);
        strncpy(&fpath[0], &paths[PATH_LEN * i], PATH_LEN);
	//snprintf(&fpath[0], PATH_LEN, &paths[PATH_LEN * i]);
        printf("[%i] Reading file %i/%i: %s \n", mpi_rank, i, blockInt, &fpath[0]);
        
        headerId = offset + i;
        
        Header_read(&fpath[0], &headers[i], headerId, mpi_rank);
        
    }
    return 0;
}
*/

int writeHeaderBlock_ser(hid_t file_id, char* dataset, hsize_t* offset, hsize_t* block, header_t* headers)
{
    hid_t dset_id, fspace_id, headertype, memspace_id, plist_id;
    herr_t status;
    int rank = 1;
    hsize_t stride = 1;
    hsize_t count = 1;
    // SANITY TEST ON HEADER: MAKE SURE IT HAS A POINTCOUNT VALUE and a PaTH 
    printf("Beginning write: Path: %s, Point count:%d\n", headers[0].path, headers[0].pnt_count);
    //plist_id = H5Pcreate(H5P_FILE_ACCESS);
    // Set parallel reading property flag 
    //H5Pset_fapl_mpio(plist_id, comm, info);
    printf("Opening file for reading\n");
    // Open file for reading 
    //file_id = H5Fopen(file, H5F_ACC_RDWR | H5F_ACC_DEBUG, plist_id);
    printf("File opened, opening Dataset.\n");
    // Open Dataset 
    plist_id = H5Pcreate(H5P_DATASET_ACCESS);
    
    // Open the header dataset 
    dset_id = H5Dopen(file_id, dataset, plist_id);
    // Get the id for the header data space 
    fspace_id = H5Dget_space(dset_id);
    memspace_id = H5Screate_simple(rank, block, NULL);

    headertype = HeaderType_create(&status);
    status = H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, offset, &stride, &count, block);
    
    plist_id = H5Pcreate(H5P_DATASET_XFER);
    // CHECKING THE DATA 
    printf("Writing dataset\n");
    status = H5Dwrite(dset_id, headertype, memspace_id, fspace_id, plist_id, headers);
    printf("Cleaning up\n");
    status = H5Dclose(dset_id);
    status = H5Sclose(fspace_id);
    status = H5Sclose(memspace_id);
    status = H5Pclose(plist_id);
    HeaderType_destroy(headertype, &status);
    return EXIT_SUCCESS;
}
int writeHeaderBlock(hid_t file_id, char* dataset, hsize_t* offset, hsize_t* block, header_t* headers, MPI_Comm comm, MPI_Info info)
{
    hid_t dset_id, fspace_id, headertype, memspace_id, plist_id;
    herr_t status;
    int rank = 1;
    hsize_t stride = 1;
    hsize_t count = 1;
    int mpi_rank;
    MPI_Comm_rank(comm, &mpi_rank);
    // SANITY TEST ON HEADER: MAKE SURE IT HAS A POINTCOUNT VALUE and a PaTH 
    printf("[%i] Beginning write: Path: %s, Point count:%d\n", mpi_rank, headers[0].path, headers[0].pnt_count);
    //plist_id = H5Pcreate(H5P_FILE_ACCESS);
    // Set parallel reading property flag 
    //H5Pset_fapl_mpio(plist_id, comm, info);
    printf("[%i]Opening file for reading\n", mpi_rank);
    // Open file for reading
    //file_id = H5Fopen(file, H5F_ACC_RDWR | H5F_ACC_DEBUG, plist_id);
    printf("[%i] File opened, opening Dataset.\n", mpi_rank);
    // Open Dataset
    plist_id = H5Pcreate(H5P_DATASET_ACCESS);
    
    // Open the header dataset 
    dset_id = H5Dopen(file_id, dataset, plist_id);
    // Get the id for the header data space 
    fspace_id = H5Dget_space(dset_id);
    memspace_id = H5Screate_simple(rank, block, NULL);

    headertype = HeaderType_create(&status);
    printf("[%i] Selecting offset: %i, stride: %i, count: %i, block: %i\n", mpi_rank, (int)*offset, (int)stride, (int)count, (int)*block);
    status = H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, offset, &stride, &count, block);
    
    // Set flag for parallel writing
    plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE);
    // CHECKING THE DATA 
    //int i;
    printf("[%i] Writing dataset\n", mpi_rank);
    status = H5Dwrite(dset_id, headertype, memspace_id, fspace_id, plist_id, headers);
	printf("[%i] Cleaning up\n", mpi_rank);
    status = H5Dclose(dset_id);
    status = H5Sclose(fspace_id);
    status = H5Sclose(memspace_id);
    status = H5Pclose(plist_id);
    HeaderType_destroy(headertype, &status);
    //status = H5Fclose(file_id);
    return EXIT_SUCCESS;
}
