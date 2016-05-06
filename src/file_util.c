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

//#define PATH_LEN=4096

/** 
 * @brief dump_entry: Check the name and type of a filesystem entry
 *
 * @param entry: #dirent = Pointer to a directory entry metadata object
 */
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

/**
 * @brief buildArray: Create an array of LAS filepaths found in a directory
 *
 * @param dirPath: char[] = Character array holding the directory path to be
 * searched
 * @param outPaths: char[] = Character array that will hold the LAS file paths
 * @param file_count: int = Number of files to find before returning
 * @return 0 if successful, else 1 (TODO: Should probably reverse these return
 * values for consistency
 */
int buildArray( char  dirPath[], char outPaths[], int file_count) {
    // make sure size >= 2
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char * ext;
    size_t counter = 0;
    //char buf[PATH_LEN + 1];
    //int i = 0;
    //char *fptr;
    //char fullPath[PATH_LEN];
    char fname[PATH_LEN];
    //fptr = fname;
    printf("Reading files from %s\n", dirPath);
    //char *ptr = realpath(dirPath, fullPath); 
    printf("Checking files from %s\n", dirPath);
    if ((dir = opendir(dirPath)) == NULL)
    {
        fprintf(stderr, "Error: Failed to open input directory: %s, ERROR: %s\n", dirPath, strerror(errno));
        exit(1);
    }
    chdir(dirPath);
    printf("Looking for %i files in %s\n", file_count, dirPath);
    while (((entry = readdir(dir)) != NULL) && (counter < file_count))
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
                            strcat(fname, "\0");
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
                            //snprintf(&outPaths[counter * PATH_LEN], PATH_LEN, fname);
                            strncpy(&outPaths[counter * (PATH_LEN)], fname, PATH_LEN);
                            
                            //printf("Copied to path list %s.\n", &outPaths[counter * PATH_LEN]);
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
/**
 * @brief listFiles: Recursively print LAS filenames under a directory path
 *
 * @param dirPath: char[] = Directory path to search
 * @param depth: int = Number of subdirectory levels to search
 * @return 0 if successful else, 1
 * @note This function is deprecated since it calls itself recursively and
 * doesn't return anything
 */
int listFiles(char dirPath[], int depth)
{
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char  * ext;
    
    char buf[PATH_LEN];
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

/**
 * @brief countLAS: Count the number of LAS files under a directory path
 *
 * @param dirPath: char[] = The directory path to search
 * @return int representing the number of LAS files found in the directory
 */
int countLAS(char dirPath[])
{
    DIR *dir;
    struct dirent *entry;
    char * ext;

    //char buf[PATH_LEN+1];
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


/** 
 * @brief taskType_Create: Create a task data struct to hold information
 * necessary to run a subprocess
 *
 * @note As of now, this data type is deprecated, creating the processes using
 * a block definitions seemed to be a more generally applicable approach since
 * it could apply to different processes
 * @param task: #task_t = Task object to hold the process metadata
 * @param fname: char[] = Name of the filename to process
 * @param offset: size_t = Offset of point array to begin reading
 * @param size: size_t = Number of points to read
 */
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

/** 
 * @brief taskType_Print: Print a task object to standard output
 *
 * @param task: #task_t = Task object to print
 */
void taskType_Print(task_t *task)
{
    fprintf(stdout, "File: %s, Start: %zu, Count: %zu\n", task->fname, task->offset, task->size);
}

/** 
 * @brief taskType_Destroy: Free the memory used by a task object
 *
 * @param task: #task_t = Task object to free
 */
void taskType_Destroy(task_t *task) {
    free(task);
}

/**
 * @brief getWorkingDir: Get the path to the current working directory
 *
 * @param pathBuf: char* = The String to hold the filepath
 * @return Int representing the number of characters in the filepath
 */
int getWorkingDir(char* pathBuf)
{
    int len;
	getcwd(pathBuf, PATH_LEN);
	//MIN(readlink(szTmp,pathTmp,len),len -1);
    //int bytes = MIN(readlink(szTmp, pathTmp, len), len - 1);
    //if (bytes >= 0)
    //    pathTmp[bytes] = "\0";
	printf("Path: %s\n", pathBuf);
	
    //pathBuf = dirname(pathTmp);
    //pathBuf = dirname(pathBuf);
    len = strlen(pathBuf);
    return len;
}

/** 
 * @brief resolvePath: Clean a filepath
 *
 * This will escape any unprintable special characters in a filepath and 
 * remove them. This allows for 'generally safe' processing of filenames
 * @param path: char* = The file path to be cleaned
 * @param verbose: int = Whether or not verbose output should be printed
 * @return Pointer to resolve filepath
 */
char* resolvePath(char* path, int verbose) {
	char actualPath[PATH_LEN];
	char *ptr;
	if (verbose) {
		printf("Checking for non-printable characters\n");
		ptr = path;
		while (*ptr) {
			switch(*ptr) {
				case '\v': printf("\\v"); break;
				case '\n': printf("\\n"); break;
				case '\t': printf("\\t"); break;
				case '\r': printf("\\r"); break;
				case '\0': printf("\\0"); break;
				default: putchar(*ptr); break;
			}
			ptr++;
		}
		printf("\n");
	}
	// Remove newlines and carriage returns
	path[strcspn(path, "\r\n")] = 0;
	ptr = realpath(path, actualPath);
	if (errno) {
		printf("Error stating file: %s\n", strerror(errno));
		//TODO: How should this be handled?
		exit(1);
	}
	printf("Checking if %s exists\n", actualPath);
	if (access (actualPath, F_OK) == -1)
		printf("File Stat failed: %s\n", strerror(errno));
	// Zero-fill path
	memset(path, 0, PATH_LEN);
	// Copy resolved path into original buffer
	strcpy(path, actualPath);
	return ptr;
}

	

/**
 * @brief fileExists: Checks whether a file exists at a filepath
 *
 * @param filename: char* = The filename to check
 * @return 0 if file exists, else 1
 */
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
