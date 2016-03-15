#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <liblas/capi/liblas.h>

#define PATH_LEN 4096

typedef struct task_t{
    size_t offset;
    size_t size;
    char fname[PATH_LEN];
} task_t;


int taskType_Create(task_t *task, char fname[], size_t start, size_t size);

void taskType_Print(task_t *task);

void taskType_Destroy(task_t *task);

void dump_entry (struct dirent *entry);

int listFiles(char dirPath[], int depth);

int countLAS(char dirPath[]);

int buildArray(char dirPath[], char outPaths[], int file_count);

int getWorkingDir(char* pathBuf);

int fileExists(char* filename);

char* resolvePath(char* path, int verbose);

#endif
