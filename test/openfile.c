/***************************************************************
 * Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  *
 * Information Laboratory (CIGI). All Rights Reserved.         *
 *                                                             *
 ***************************************************************/

/** 
 * @file fileopen.c
 * @author Nathan Casler
 * @date May 6 2016
 * @brief Test to check if file opening/closing is functional
 *
 */
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int parse_args(int argc, char* argv[], char* filename) {
	int i;
	opterr = 0;

	while ((i = getopt (argc, argv, "i:")) != -1)
		switch (i)
			{
				case 'i':
					printf("Checking file: %s\n", optarg);
					strcpy(filename, optarg);
					break;
				case '?':
					if (optopt == 'i')
						fprintf(stderr, "Option -%c requires an argument.\n", optopt);
					else if(isprint (optopt))
						fprintf(stderr, "Unknown option `-%c`.\n", optopt);
					else
						fprintf(stderr, "Unknown option character `\\x%x`.\n", optopt);
					return 1;
				default:
					abort();
			}
	return 0;
}

char*  resolvePath(char* path) {
	char actualPath[PATH_MAX];
	char *ptr = path;
	while(*ptr) {
		switch(*ptr) {
			case '\v': printf("\\v"); break;
			case '\n': printf("\\n"); break;
			case '\t': printf("\\t"); break;
			case '\0': printf("\\0"); break;
			default: putchar(*ptr); break;
		}
		ptr++;
	}
	path[strcspn(path, "\r\n")] = 0;
	printf("\n");
	ptr = realpath(path, actualPath);
	if (errno) {
		printf("Checking error: %s\n", strerror(errno));
		exit(1);
	} 
	printf("Stating %s\n", ptr);
	if (access (actualPath, F_OK) != -1) {
		printf("File stat successful\n");
	} else {
		printf("Failed to open file: %s\n", strerror(errno));
		
	}
	strcpy(path, actualPath);
	return ptr;

}

int main(int argc, char* argv[]) {
	char filename[PATH_MAX];
	char actualPath[PATH_MAX];
	char *ptr;
	int i;
	int fileCount = 0;
	char line[PATH_MAX];
	parse_args(argc, argv, &filename[0]);
	char *linePtr;
	ptr = resolvePath(&filename[0]);
	FILE* fp = fopen(ptr, "r");
	while(!feof(fp)) {
		char ch;
		ch = fgetc(fp);
		if (ch == '\n')
		{
			fileCount++;
		}
	}
	printf("Found %i files\n", fileCount);
	fseek(fp, 0, SEEK_SET);
	for (i = 0;i < fileCount; i++) {
		fgets(line, sizeof(line), fp);
		line[strcspn(line, "\r\n")] = 0;
		printf("Checking path for %s\n", line);
		linePtr = resolvePath(line);
		printf("File read %s\n", linePtr);
	}
	fclose(fp);
	printf("Finished execution\n");
	return 0;
}
