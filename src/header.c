/***************************************************************
 * Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  *
 * Information Laboratory (CIGI). All Rights Reserved.         *
 *                                                             *
 ***************************************************************/

/** 
 * @file header.c
 * @author Nathan Casler
 * @date May 6 2016
 * @brief File containing methods for reading header data
 *
 */


#include "mpi.h"
#include "hdf5.h"
#include "hdf5_hl.h"
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


/*int Proj_Set(LASHeaderH header, proj_t* proj) {
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
}*/

int Header_Set(uint32_t pnt_count, LMEboundCode bounds, char fname, proj_t proj_str) {
	return 0;
}

void Header_free(LMEheader* headers, int n) {
    int i;
    for (i = 0; i < n; i++) {
        free(&headers[i].id);
        free(&headers[i].pnt_count);
        free(&headers[i].bounds);
        free(&headers[i].path);
        free(&headers[i].crs);
    }
    free(headers);
}

hid_t HeaderType_create(herr_t* status) {
    hid_t coordtype, boundtype, projtype, headertype;
    coordtype = CoordType_create(status);
    boundtype = BoundType_create(status);
    projtype = ProjType_create(status);
    headertype = H5Tcreate(H5T_COMPOUND, sizeof(LMEheader));
    *status = H5Tinsert(headertype, "id", HOFFSET(LMEheader, id), H5T_NATIVE_UINT);
    *status = H5Tinsert(headertype, "bounds", HOFFSET(LMEheader, bounds), boundtype);
    *status = H5Tinsert(headertype, "name", HOFFSET(LMEheader, path), projtype);
    *status = H5Tinsert(headertype, "crs", HOFFSET(LMEheader, crs), projtype);
    *status = H5Tinsert(headertype, "pnt_count", HOFFSET(LMEheader, pnt_count), H5T_NATIVE_UINT);
    BoundType_destroy(boundtype, status);
    ProjType_destroy(projtype, status);
    CoordType_destroy(coordtype, status);
    return headertype;
}

void HeaderType_destroy(hid_t headertype, herr_t* status) {
    *status = H5Tclose(headertype);
}
/*
int MPI_HeaderType_create(MPI_Datatype *mpi_headertype) {
    int nitems=5;
    int strLen = PATH_LEN;
    LMEheader header;
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
    //offsets[2] = offsetof(LMEheader, bounds);
    //offsets[3] = offsetof(LMEheader, path);
    //offsets[4] = offsetof(LMEheader, proj);
    printf("Creating mpi_header_struct\n");
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
*/
int LMEheaderSet_createDataset(hid_t group_id, LMEheader* headers, uint32_t n_headers) 
{
	hid_t plist_id, id_type, count_type, code_type, bound_type, crs_type, path_type;	
	hsize_t n_records = (hsize_t) n_headers;
	herr_t status;

	const hsize_t n_fields = 5;
	hid_t field_types[5];
	int i;
	/* Calculate the size and offsets for the header struct;*/
	size_t dst_size = sizeof(LMEheader);
	printf("Header size %zu\n", dst_size);
	size_t dst_offsets[5] = { HOFFSET(LMEheader, id),
							HOFFSET(LMEheader, pnt_count),
							HOFFSET(LMEheader, bounds),
							HOFFSET(LMEheader, crs),
							HOFFSET(LMEheader, path)};

	const char *field_names[5] = {"id", "pointCount", "bounds", "projection", "filepath"};

	id_type = H5Tcopy( H5T_STD_U64LE);
	count_type = H5Tcopy( H5T_STD_U32LE);
	code_type = H5Tcopy(H5T_STD_U32LE);
	H5Tset_size(code_type, 3);
	bound_type = H5Tcopy(code_type);
	H5Tset_size(bound_type, 2);
	path_type = H5Tcopy(H5T_NATIVE_CHAR);
	H5Tset_size(path_type, 4096);
	printf("CRS size: %zu\n", sizeof(LMEcrs));
	crs_type = H5Tcreate(H5T_COMPOUND, sizeof(LMEcrs));
	status = H5Tinsert(crs_type, "type", HOFFSET(LMEcrs, type), H5T_NATIVE_INT);
	status = H5Tinsert(crs_type, "projStr", HOFFSET(LMEcrs, projStr), path_type);
	// Set the field types
	field_types[0] = id_type;
	field_types[1] = count_type;
	field_types[2] = bound_type;
	field_types[3] = crs_type;
	field_types[4] = path_type;
	
	for (i =0; i < 5; i++) {
		printf("Type %i: size %zu\n", i, H5Tget_size(field_types[i]));
	}
	/** Table specific properties **/
	hsize_t chunk_size = 1000;
	int *fill_data = NULL;
	int compress = 0;
	plist_id = H5Pcreate(H5P_DATASET_ACCESS);
	char* dataset_name = malloc(sizeof(char) * 32);
	rand_string(dataset_name, 32);
	printf("Dataset name: %s\n", dataset_name);

	/** Generate table **/
	status = H5TBmake_table("Test Headers", group_id, dataset_name, n_fields, n_records, dst_size, field_names, dst_offsets, field_types, chunk_size, fill_data, compress, headers);
	H5Tclose(path_type);
	H5Tclose(crs_type);
	H5Tclose(bound_type);
	H5Tclose(code_type);
	H5Tclose(count_type);
	H5Tclose(id_type);
	H5Pclose(plist_id);
	return 1;

	//ROGER uses little endian encoding
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
int Header_read(char* path, LMEheader* header, uint32_t id) {
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
	if (!LMEboundCode_fromLAS(&header->bounds, &LASheader)) {
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
	LMEcrs_fromLAS(&header->crs, &LASheader);
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

int LMEheaderBlock_read(char paths[], int offset, int block, LMEheader* headers)
{
    int i; // counter
    int strLen = PATH_LEN;
    char fpath[strLen];

    int blockInt = block;
    uint32_t headerId;
    //fprintf(stderr, "Reading %i files\n", blockInt);
    for (i = 0; i < (blockInt); i++) {
        /* Set reader and header null to allow for error checking */
        //printf("[%i]Last file is %s\n", mpi_rank, &paths[(blockInt - 1) * PATH_LEN]);
        memset(&fpath[0], 0, PATH_LEN);
        strncpy(&fpath[0], &paths[PATH_LEN * i], PATH_LEN);
	//snprintf(&fpath[0], PATH_LEN, &paths[PATH_LEN * i]);
    //    printf("Reading file %i/%i: %s \n", i, blockInt, &fpath[0]);
        
        headerId = offset + i;
        
        if (!Header_read(&fpath[0], &headers[i], headerId)) {
			fprintf(stderr,"Header Error: Failed to read header for %s\n", fpath);
			continue;
		}
        
    }
    return 1;
}

int LMEheaderBlock_writeSer(hid_t file_id, char* dataset, hsize_t* offset, hsize_t* block, LMEheader* headers)
{
    hid_t dset_id, fspace_id, headertype, memspace_id, plist_id;
    herr_t status;
    int rank = 1;
    hsize_t stride = 1;
    hsize_t count = 1;
    // SANITY TEST ON HEADER: MAKE SURE IT HAS A POINTCOUNT VALUE and a PaTH 
   // printf("Beginning write: Path: %s, Point count:%d\n", headers[0].path, headers[0].pnt_count);
    //plist_id = H5Pcreate(H5P_FILE_ACCESS);
    // Set parallel reading property flag 
    //H5Pset_fapl_mpio(plist_id, comm, info);
 //   printf("Opening file for reading\n");
    // Open file for reading 
    //file_id = H5Fopen(file, H5F_ACC_RDWR | H5F_ACC_DEBUG, plist_id);
   // printf("File opened, opening Dataset.\n");
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
int LMEheaderBlock_write(hid_t file_id, char* dataset, hsize_t* offset, hsize_t* block, LMEheader* headers, MPI_Comm comm, MPI_Info info)
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
  //  printf("[%i]Opening file for reading\n", mpi_rank);
    // Open file for reading
    //file_id = H5Fopen(file, H5F_ACC_RDWR | H5F_ACC_DEBUG, plist_id);
   // printf("[%i] File opened, opening Dataset.\n", mpi_rank);
    // Open Dataset
    plist_id = H5Pcreate(H5P_DATASET_ACCESS);
    
    // Open the header dataset 
    dset_id = H5Dopen(file_id, dataset, plist_id);
    
	// Get the id for the header data space 
    fspace_id = H5Dget_space(dset_id);
    memspace_id = H5Screate_simple(rank, block, NULL);

    headertype = HeaderType_create(&status);
    //printf("[%i] Selecting offset: %i, stride: %i, count: %i, block: %i\n", mpi_rank, (int)*offset, (int)stride, (int)count, (int)*block);
    status = H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, offset, &stride, &count, block);
    
    // Set flag for parallel writing
    plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE);
    // CHECKING THE DATA 
    //int i;
    //printf("[%i] Writing dataset\n", mpi_rank);
    status = H5Dwrite(dset_id, headertype, memspace_id, fspace_id, plist_id, headers);
	//printf("[%i] Cleaning up\n", mpi_rank);
    status = H5Dclose(dset_id);
    status = H5Sclose(fspace_id);
    status = H5Sclose(memspace_id);
    status = H5Pclose(plist_id);
    HeaderType_destroy(headertype, &status);
    //status = H5Fclose(file_id);
    return EXIT_SUCCESS;
}
