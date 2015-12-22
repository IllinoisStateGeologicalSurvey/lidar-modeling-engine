#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>

void dump_entry (struct dirent *entry);

int listFiles(char dirPath[], int depth);

int countLAS(char dirPath[]);

int buildArray(char dirPath[], char* outPaths[], size_t size);

#endif
