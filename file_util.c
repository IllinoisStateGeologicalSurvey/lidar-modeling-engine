#ifndef FILE_UTIL_C
#define FILE_UTIL_C

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

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

#endif
