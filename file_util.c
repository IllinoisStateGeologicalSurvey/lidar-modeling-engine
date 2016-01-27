#ifndef FILE_UTIL_C
#define FILE_UTIL_C

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <liblas/capi/liblas.h>
#include "file_util.h"

void dump_entry (struct dirent *entry)
{
    char *type;
    char filename[40];
    switch(entry->d_type) {
        case DT_BLK:
            type = "block device";
            break;

        case DT_CHR:
            type = "character device";
            break;
        case DT_DIR:
            type = "directory";
            break;
        case DT_FIFO:
            type = "named pipe";
            break;
        case DT_LNK:
            type = "symbolic link";
            break;
        case DT_REG:
            type = "regular file";
            break;
        case DT_SOCK:
            type = "unix domain socket";
            break;
        case DT_UNKNOWN:
            type = "unknown";
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
    char buf[PATH_MAX + 1];
    int i = 0;
    char *ptr;
    char fullPath[PATH_MAX + 1];
    char fname[PATH_MAX + 1];
    printf("Reading files from %s\n", dirPath);
    ptr = realpath(dirPath, fullPath); 
    printf("CHecking files from %s\n", fullPath);
    if ((dir = opendir(fullPath)) == NULL)
    {
        fprintf(stderr, "Error: Failed to open input directory: %s\n", dirPath);
        exit(1);
    }
    chdir(fullPath);
    printf("Looking for %i files in %s\n", size, fullPath);
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
                printf("Regular file found at %s\n", entry->d_name);
                {
                    /* We have a regular file, check the extension */
                    if ((ext = strrchr(entry->d_name, '.')) != NULL)
                    {
                        printf("Extension is %s\n", ext);
                        if (strcmp(ext, ".las") == 0)
                        {
                            // Append the path to the filename
                            memset(fname, 0, sizeof(fname));
                            strcat(fname, fullPath);
                            strcat(fname, "/");
                            strcat(fname, entry->d_name);
                            printf("Fname: %s\n", fname);
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
                            printf("Copied to path list.\n");
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
    int i = 0;
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

    char buf[PATH_MAX+1];
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
    
}

void taskType_Print(task_t *task)
{
    fprintf(stdout, "File: %s, Start: %i, Count: %i\n", task->fname, task->offset, task->size);
}

void taskType_Destroy(task_t *task) {
    free(task);
}

int getWorkingDir(char pathBuf[])
{
    char szTmp[32];
    size_t len=PATH_MAX+1;
    sprintf(szTmp, "/proc/%d/exe", getpid());
    int bytes = MIN(readlink(szTmp, pathBuf, len), len - 1);
    if (bytes >= 0)
        pathBuf[bytes] = "\0";
    pathBuf = dirname(pathBuf);
    len = strlen(pathBuf);
    return len;
}
/*
int main(int argc, char* argv[]) 
{
    char dirname[30];
    if (argc < 1) 
    {
        fprintf(stderr, "Usage: file_util <directory>\n");
        exit(1);
    }

    strcpy(dirname, argv[1]);
    fprintf(stdout, "Reading directory %s\n", dirname);
    listFiles(dirname, 2);
    return 0;
}
*/
#endif
