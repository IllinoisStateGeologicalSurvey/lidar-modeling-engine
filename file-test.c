//This is a test to check that the file reading utilities work

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include "file_util.h"


int main(int argc, char* argv[])
{
    char dirname[PATH_MAX];
    size_t size;
    int i;
    if (argc < 2)
    {
        fprintf(stderr,  "Usage: fileTest <directory> <# of files>\n");
        exit(1);
    }

    strcpy(dirname, argv[1]);
    printf("Dirname: %s\n", dirname);
    size = atoi(argv[2]);
    printf("PathMax is: %llu\n", PATH_MAX);
    //char* outPaths[size][PATH_MAX + 1];
    char** outPaths = calloc(size, sizeof(char*));
    for (i = 0; i < size; ++i)
        outPaths[i] = malloc(PATH_MAX + 1);
    fprintf(stdout, "Reading %i files from directory %s\n", size, dirname);
    buildArray(dirname, outPaths, size);
    for (i = 0; i < size; i++){
        printf("Path %i: %s\n", i, outPaths[i]);
    }
    for (i = 0; i < size; ++i) 
        free(outPaths[i]);
    free(outPaths);
    return 0;
}
    
