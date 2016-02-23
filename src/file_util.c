#ifndef FILE_UTIL_C
#define FILE_UTIL_C

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>
#include <liblas/capi/liblas.h>
#include "file_util.h"

void dump_entry (struct dirent *entry)
{
    char type[200];
    //char filename[40];
    switch(entry->d_type) {
        case DT_BLK:
            strcpy(type, "block device");
            break;

        case DT_CHR:
            strcpy(type, "character device");
            break;
        case DT_DIR:
            strcpy(type, "directory");
            break;
        case DT_FIFO:
            strcpy(type, "named pipe");
            break;
        case DT_LNK:
            strcpy(type, "symbolic link");
            break;
        case DT_REG:
            strcpy(type, "regular file");
            break;
        case DT_SOCK:
            strcpy(type, "unix domain socket");
            break;
        case DT_UNKNOWN:
            strcpy(type, "unknown");
            break;

    }
    fprintf(stdout, "%s is %s\n" , entry->d_name, type);
}

int buildArray( char  dirPath[], char outPaths[], size_t size) {
    // make sure size >= 2
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char * ext;
    size_t counter = 0;
    //char buf[PATH_MAX + 1];
    //int i = 0;
    //char *ptr;
    char fullPath[PATH_MAX + 1];
    char fname[PATH_MAX + 1];
    printf("Reading files from %s\n", dirPath);
    //char *ptr = realpath(dirPath, fullPath); 
    printf("Checking files from %s\n", fullPath);
    if ((dir = opendir(dirPath)) == NULL)
    {
        fprintf(stderr, "Error: Failed to open input directory: %s, ERROR: %s\n", dirPath, strerror(errno));
        exit(1);
    }
    chdir(dirPath);
    printf("Looking for %zu files in %s\n", size, dirPath);
    while (((entry = readdir(dir)) != NULL) && (counter < size))
    {
        lstat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode)) {
            /* Found a directory, but ignore links to parent directory */
            if (strcmp(".", entry->d_name) == 0 ||
                strcmp("..", entry->d_name) == 0)
                continue;

            /** Recurse at a new level */
            //TODO: Make the number of files or depth controllable
            //listFiles(entry->d_name, depth+4);
        }
        else
        {
            if (S_ISREG(statbuf.st_mode)) {
                //printf("Regular file found at %s\n", entry->d_name);
                {
                    /* We have a regular file, check the extension */
                    if ((ext = strrchr(entry->d_name, '.')) != NULL)
                    {
                        //printf("Extension is %s\n", ext);
                        if (strcmp(ext, ".las") == 0)
                        {
                            // Append the path to the filename
                            memset(fname, 0, sizeof(fname));
                            strcat(fname, dirPath);
                            strcat(fname, "/");
                            strcat(fname, entry->d_name);
                            //printf("Fname: %s\n", fname);
                            /* TODO: Figure out why real path spits null strings here
                            char *res = realpath(fname, buf);
                            if (res) {
                                printf("Buffer is: %s\n", counter, buf);
                            } else {
                                perror("realpath");
                                exit(1);
                            }*/
                            //** TODO: Figure out how to write to string array **/
                            strncpy(&outPaths[counter * (PATH_MAX + 1)], fname, PATH_MAX + 1);
                            //printf("Copied to path list.\n");
                            counter++;
                        }
                        else
                        { 
                            continue;
                        }
                    }
                }

            }
        }
    }
    chdir("..");
        //dump_entry(entry);
    closedir(dir);
    return 0;
}
int listFiles(char dirPath[], int depth)
{
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char  * ext;
    
    char buf[PATH_MAX + 1];
    //int i = 0;
    if ((dir = opendir(dirPath)) == NULL)
    {
        fprintf(stderr, "Error: Failed to open input directory\n");
        exit(1);;
    }
    chdir(dirPath);
    while ((entry = readdir(dir)) !=NULL)
    {
        lstat(entry->d_name,&statbuf);
        if (S_ISDIR(statbuf.st_mode)) {

            /* Found a directory, but ignore parent directory */
            if (strcmp(".", entry->d_name) == 0 ||
                strcmp("..", entry->d_name) == 0)
                continue;
            
            //printf("%*s%s\n",depth,"",entry->d_name);
            /** Recurse at a new level */
            listFiles(entry->d_name, depth+4);
        }
        else 
        {
            if ((ext = strrchr(entry->d_name, '.')) != NULL) 
            {
                if (strcmp(ext,".las") == 0)
                {
                    realpath(entry->d_name,buf);
                    //*pathList + i = buf;
                    printf("%s\n",buf);
                    //i++;
                }
            }
        }
    }
          
    chdir("..");
        //dump_entry(entry);
    closedir(dir);
    return 0;
}

int countLAS(char dirPath[])
{
    DIR *dir;
    struct dirent *entry;
    char * ext;

    //char buf[PATH_MAX+1];
    int i = 0;
    if ((dir = opendir(dirPath)) ==  NULL)
    {
        fprintf(stderr, "Error: Failed to open input directory\n");
        exit(1);
    }
    chdir(dirPath);
    while ((entry = readdir(dir)) != NULL)
    {
        if ((ext = strrchr(entry->d_name, '.')) != NULL)
        {
            if (strcmp(ext, ".las") == 0)
            {
                i++;
                //realpath(entry->d_name, buf);
                // Append this to list
            }
        }
    }
    return i;
}


/** Task Type **/
int taskType_Create(task_t* task, char fname[], size_t offset, size_t size)
{
    strcpy(task->fname, fname);
    task->offset = offset;
    LASReaderH reader =  LASReader_Create(fname);
    LASHeaderH header = LASReader_GetHeader(reader);

    task->size = LASHeader_GetPointRecordsCount(header);
    free(reader);
    free(header);
	return 0;    
}

void taskType_Print(task_t *task)
{
    fprintf(stdout, "File: %s, Start: %zu, Count: %zu\n", task->fname, task->offset, task->size);
}

void taskType_Destroy(task_t *task) {
    free(task);
}

int getWorkingDir(char* pathBuf)
{
    char szTmp[32];
    size_t len=PATH_MAX+1;
    sprintf(szTmp, "/proc/%d/exe", getpid());
    char pathTmp[len];
    MIN(readlink(szTmp,pathTmp,len),len -1);
    //int bytes = MIN(readlink(szTmp, pathTmp, len), len - 1);
    //if (bytes >= 0)
    //    pathTmp[bytes] = "\0";
    strcpy(pathBuf, dirname(pathTmp));
    //pathBuf = dirname(pathTmp);
    fprintf(stderr, "WORKING PATH: %s\n", pathBuf);
    //pathBuf = dirname(pathBuf);
    len = strlen(pathBuf);
    return len;
}

int fileExists(char* filename) {
    if (access(filename, R_OK) != -1) {
        // File exists
        return 0;
    } else if (access(filename, F_OK) != -1) {
        fprintf(stderr, "PERMISSION ERROR: Cannot read file %s\n", filename);
        return 1;
    } else {
        fprintf(stderr, "FILE ERROR: File not found: %s\n", filename);
        return 1;
    }
}


#endif
