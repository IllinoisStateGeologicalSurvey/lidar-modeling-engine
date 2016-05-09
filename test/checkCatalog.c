/***************************************************************
 * Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  *
 * Information Laboratory (CIGI). All Rights Reserved.         *
 *                                                             *
 ***************************************************************/

/** 
 * @file checkCatalog.c
 * @author Nathan Casler
 * @date May 6 2016
 * @brief Function which will read metadata from the LME datastore
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <hdf5.h>
#include <math.h>
#include <mpi.h>
#include <errno.h>
#include <libgen.h>
#include <unistd.h>
#include "file_util.h"
#include "header.h"
#include "reader.h"
#include "util.h"

/* Struct for holding link operator information */
struct opdata {
	unsigned	recurs;   /* Recursion level. 0=root */
	struct opdata *prev;  /* Pointer to previous opdata */
	haddr_t		addr;	  /* Group address */
}opdata;

/* Operator function to be called by H5Literate
 */
herr_t op_func(hid_t loc_id, const char *name, const H5L_info_t *info, void *operator_data);

/* Function to check for duplicate groups in a path
 */
int group_check (struct opdata *od, haddr_t target_addr);


int createCatalog(hid_t* file_id, char* path) {

	// Check if the catalog already exists
	
	if (H5Lexists(*file_id, path, H5P_DEFAULT)) {
		return 1;
	} else {
		return 0;
	}

	}

void usage()
{
    fprintf(stderr, "-------------------------------------------------------\n");
    fprintf(stderr, "                  DSMinit usage:\n");
    fprintf(stderr, "\n");
    fprintf(stderr, " DSMinit \n");
    fprintf(stderr, "-------------------------------------------------------\n");
}

int main(int argc, char* argv[])
{
    
    hid_t			file_id, fapl_id, fcpl_id;
	herr_t			h5status;
	H5O_info_t		infobuf;
	struct opdata	od;

	/* Open file and initialize operator data structure */
	char* h5_file = (char *)malloc(sizeof(char)* PATH_LEN);
	getDataStore(h5_file);
	
   
    double start, current;

    
    int mpi_size, mpi_rank, mpi_err, mpi_err_class, resultlen;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;
    MPI_Status status;
    char err_buffer[MPI_MAX_ERROR_STRING];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &mpi_size);
    MPI_Comm_rank(comm, &mpi_rank);
    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
    start = MPI_Wtime();

    
    fcpl_id = H5Pcreate(H5P_FILE_CREATE);
    fapl_id = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_mpio(fapl_id, comm, info);
	printf("Opening LME\n");
    file_id = H5Fopen(h5_file, H5F_ACC_RDWR | H5F_ACC_DEBUG, fapl_id);
	h5status = H5Oget_info (file_id, &infobuf);
	od.recurs = 0;
	od.prev = NULL;
	od.addr = infobuf.addr;

	/* Print the root group and formatting, begin iteration */
	printf("/ {\n");
	h5status = H5Literate (file_id, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, op_func, (void *) &od);
	printf ("}\n");


    //if ( createCatalog(&file_id, "/regions") != 0 ) {
	//	printf("Regions meta group exists\n");
	//} else {
	//	printf("Failed to find Regions group\n");
	//}
	//group_id = H5Gcreate(file_id, "/regions", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    
    H5Pclose(fcpl_id);
    H5Pclose(fapl_id);
    H5Fclose(file_id);
	free(h5_file);    
	MPI_Finalize();
    return 0;
};

/** TODO Create function to add attribute to region with point total, min, max
 * lat/long, elev, intens, projection */


/**********************************************
 * Operator function. This function prints the name and type
 * of the object passed to it. If the object is a group, it is
 * first checked against other groups in its path using the
 * group_check function, then if it is not a duplicate, 
 * H5Literate is called for that group. THis guarantees that
 * the program will not enter an infinite recursion due to a 
 * circular path in the file.
 **********************************************/
herr_t op_func(hid_t loc_id, const char *name, const H5L_info_t *info, void *operator_data)
{
	herr_t		h5status, return_val = 0;
	H5O_info_t	infobuf;
	struct opdata	*od = (struct opdata *) operator_data;
					/* Type conversion */
	unsigned	spaces = 2*(od->recurs+1);
					/* Number of whitespaces to prepend to output*/

	/*
	 * Get type of the object and display its name and type.
	 * The name of the object is passed to this function by
	 * the Library.
	 */
	h5status = H5Oget_info_by_name (loc_id, name, &infobuf, H5P_DEFAULT);
	printf ("%*s", spaces, "");  /* Format output */
	switch (infobuf.type) {
		case H5O_TYPE_GROUP:
				printf("Group: %s {\n", name);

				/*
				* Check group address against linked list of operator 
				* data structures. We will always run the check, as the
				* reference count cannot  be relied upon if there are 
				* symbolic links, and H5Oget_info_by_name always follows
				* symbolic links. Alternatively we could use H5Lget_info
				* and never recurse on groups discovered by symbolic
				* links, however it could still fail if an object's
				* reference count was manually manipulated with 
				* H5Odecr_refcount.
				*/
			if (group_check(od, infobuf.addr) ) {
				printf("%*s Warning: Loop detected!\n", spaces, "");
			}
			else {
				/* Initialize new operator data structure and
				* begin recursive iteration on the discovered 
				* group. The new opdata structure is given a 
				* pointer to the current one.
				*/
				struct opdata nextod;
				nextod.recurs = od->recurs+1;
				nextod.prev = od;
				nextod.addr = infobuf.addr;
				return_val = H5Literate_by_name(loc_id, name, H5_INDEX_NAME,
							H5_ITER_NATIVE, NULL, op_func, (void *) &nextod,
							H5P_DEFAULT);
			}
			printf("%*s}\n", spaces, "");
			break;
		case H5O_TYPE_DATASET:
			printf("Dataset: %s\n", name);
			if (strcmp(name, "headers\0") == 0) {
				printf("Found header!\n");
			}
			break;
		case H5O_TYPE_NAMED_DATATYPE:
			printf("Datatype: %s\n", name);
			break;
		default:
			printf("Unknown: %s\n", name);
	}
	return return_val;
}


/**********************************************
 * This function recursively searches a linked list of
 * opdata structures for one whose address matches
 * target_addr. Returns 1 if a match is found, and 0 otherwise.
 * ********************************************/

int group_check (struct opdata *od, haddr_t target_addr)
{
	if (od->addr == target_addr)
		return 1; /* Addresses match */
	else if (!od->recurs)
		return 0; /* Root group reached with no matches */
	else
		return group_check (od->prev, target_addr);
			/* Recursively examine next node */
}
