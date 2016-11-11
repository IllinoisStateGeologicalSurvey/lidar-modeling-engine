/**********************************************8
 *  Copyright (c) 2015-2017 Cyberinfrastructure and Geospatial Information
 *  Laboratory (CIGI). All Rights Reserved.
 *
 ***********************************************/

/**
 * @file LMEUtils.c
 * @author Nathan Casler
 * @date June 20 2016
 * @brief This will test the capabilities of the LME datastore. With regards to
 * dataset creation, group addition and symlinking.
 */

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <hdf5.h>
#include <mpi.h>
#include <unistd.h>
#include <string.h>
#include "util.h"

#define PATH_SIZE 1024

int main(int argc, char* argv[]) {
	char buf[PATH_SIZE];
	getDataStore(&buf[0]);
	
	hid_t file_id;
	/** Fetch the file id if it exists **/
	openLME(&file_id);
	

	
	H5Fclose(file_id);




	//free(path);
	//
	
	return 0;

}
