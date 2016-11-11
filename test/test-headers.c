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
#include <time.h>
#include "util.h"
#include <argp.h>
#include <argz.h>
#include "header.h"
#include "file_util.h"


const char *argp_program_version = "version 0.1";
const char *argp_program_bug_address = "<ncasler@illinois.edu>";

struct arguments 
{
	char* argz;
	size_t argz_len;
	char *dirName;
	int n_files;
};
static char doc[] = "Attempts to read a series of LAS Files into the LME datastore";
static char args_doc[] = "[FILENAME]...";
static struct argp_option options[] =  {
	{ "directory", 'd', "DIRNAME", 0, "Directory holding LAS files to read"},
	{ "files", 'f', "NUM", 0, "Number of LAS files to read"},
	{0}
};


static int parse_opt(int key, char *arg, struct argp_state *state) {
	struct arguments *a = state->input;
	switch(key) {
		case 'd': 
			a->dirName = arg;
			break;
		case 'f': 
			a->n_files = atoi(arg);
			break;
		case ARGP_KEY_ARG:
			argz_add(&a->argz, &a->argz_len, arg);
			break;
		case ARGP_KEY_INIT: 
			a->argz = NULL;
			a->argz_len = 0;
			a->dirName = "";
			a->n_files = 0;
			break;
		case ARGP_KEY_END:
			{
			size_t count = argz_count(a->argz, a->argz_len);
			if (a->n_files == 0) {
				/* Not enough arguments */
				argp_failure(state, 1, 0, "Failed to specify number of files");
			} else if (strcmp(a->dirName, "") == 0) {
				argp_failure(state, 1, 0, "Failed to specify file path");
			}
			break;
		}
	}	
	return 0;
}

/* Our arg parser */
static struct argp argp = { options, parse_opt, args_doc, doc};



int main(int argc, char* argv[]) {
	struct arguments arguments;
	//struct argp argp = { options, parse_opt, 0, 0, doc };


	/* Default argument values */
	//arguments.dirName = "-";
	//arguments.n_files = 1;

	/* Parse arguments */
	argp_parse (&argp, argc, argv, 0, 0, &arguments);
	printf("Directory: %s\n",arguments.dirName);
	/*if (strcmp(arguments.dirName, "-")) {
		printf("No filepath supplied. exiting.\n");
		exit(0);
	} else {
		printf("Reading files from %s\n", arguments.dirName);
	}*/

	//char buf[PATH_MAX+1];
	//getDataStore(&buf);
	
	hid_t file_id, group_id, plist_id;
	herr_t status;
	clock_t start = clock(), diff;
	/** Fetch the file id if it exists **/
	openLME(&file_id);
	int i;
	/* Initialize variables used to read headers */
	LASReaderH lasReader = NULL;
	char* pathBuf = malloc(sizeof(char)* (PATH_SIZE));
	LMEheader* headers;
	char* filePaths =  malloc(sizeof(char) * ((size_t)arguments.n_files * PATH_SIZE));
	
	headers = malloc(sizeof(LMEheader) * arguments.n_files);
	/** Get the list of files **/
	buildArray(arguments.dirName, filePaths, arguments.n_files);
	printf("Files found\n");
	//NOTE:  Should create if not exists
	plist_id = H5Pcreate(H5P_GROUP_CREATE);
	/** Check existence of group **/
	//status = H5Eset_auto(H5E_DEFAULT, NULL, NULL);
	if (H5Lexists(file_id, "/headers", H5P_DEFAULT) > 0) {
		printf("Header group exists\n");
		group_id = H5Gopen(file_id, "/headers", H5P_DEFAULT);
	} else {
		printf("Creating header group\n");
		group_id = H5Gcreate(file_id, "/headers", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	}
	//status = H5Eset_auto(H5E_DEFAULT, NULL, NULL);
	//NOTE: Need to set up method to get the filenames here, could use
	//buildArray()
	printf("Reading headers\n");
	LMEheaderBlock_read(&filePaths[0], 0, arguments.n_files, headers);
	printf("Headers read successfully.\n");
	LMEheaderSet_createDataset(group_id, headers, (uint32_t) arguments.n_files);

	free(pathBuf);
	free(filePaths);
	free(headers);
	H5Pclose(plist_id);
	H5Gclose(group_id);
	H5Fclose(file_id);





	//free(path);
	//
	
	return 0;

}
