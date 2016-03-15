#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <math.h>
#include <errno.h>
#include "file_util.h"
#include "common.h"
#include "header.h"
#include "reader.h"

void usage()
{
    fprintf(stderr, "-------------------------------------------------------\n");
    fprintf(stderr, "                    registerLAS usage:\n");
    fprintf(stderr, "\n");
    fprintf(stderr, " registerLAS -i <path-to-register> -f <hdf-file>\n");
    fprintf(stderr, "-------------------------------------------------------\n");
}

void parseArgs(int argc, char* argv[], char *lasPath, char* h5_file, int* verbose)
{
    int i;
    /* Check the arguments */
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 ||
                strcmp(argv[i], "--help") == 0
            )
        {
            usage();
            exit(0);
        }
        else if (strcmp(argv[i], "-v") == 0 ||
                strcmp(argv[i], "--verbose") == 0
                )
        {
            *verbose = 1;
        }
        else if (strcmp(argv[i], "-i") == 0 ||
                    strcmp(argv[i], "-in") == 0 ||
                    strcmp(argv[i], "--input") == 0
                )
        {
            i++;
            strcpy(lasPath, argv[i]);
        }
        else if (strcmp(argv[i], "-f") == 0 ||
                    strcmp(argv[i], "--h5-file") == 0)
        {
            i++;
            strcpy(h5_file, argv[i]);
        } else
        {
            fprintf(stderr, "Error: unknown argument '%s'\n", argv[i]);
            usage();
            exit(1);
        }
    }
}

int main(int argc, char* argv[])
{
    char dirname[PATH_LEN];
    char h5_file[PATH_LEN];
    hid_t file_id, plist_id;
    int i;
    int verbose;
    size_t totPoints = 0;
    header_t* headers;

    parseArgs(argc, argv, &dirname[0], &h5_file[0], &verbose);
    printf("Registering directory: %s\n", dirname);
    int file_count = countLAS(dirname);
    
    //Open the h5 file
    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    file_id = H5Fopen(h5_file, H5F_ACC_RDWR | H5F_ACC_DEBUG, plist_id);


    // Should get the count of existing headers here
    //offset = header_count(file_id);
    //
    char *out_paths = malloc(sizeof(char)* (size_t)(file_count * PATH_LEN));
    headers = malloc(sizeof(header_t) * file_count);
    // Get the paths for the LAS files 
    buildArray(dirname, out_paths, file_count);
    // Read the necessary metadata from the LAS files
    readHeaderBlock(&out_paths[0], 0, file_count, headers);
    
    // Write the header data to the HDF dataset
    hsize_t hBlock = file_count;
    hsize_t hOffset = 0;

    writeHeaderBlock(file_id, "/headers", &hOffset, &hBlock, headers);

    H5Pclose(plist_id);
    H5Fclose(file_id);
    free(out_paths);
    free(headers);
    printf("Process complete\n");
    return 0;
}
