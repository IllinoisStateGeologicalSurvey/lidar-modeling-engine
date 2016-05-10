#ifndef LME_CRS_H
#define LME_CRS_H

#include <stdlib.h>
#include <stdio.h>
#include <proj_api.h>
#include <string.h>

typedef LMEcrs {
	int type;
	char* projStr;
} LMEcrs;

int LMEcrs_set(LMEcrs* crs, char* projStr);

int LMEcrs_check(char* projStr);

int LMEcrs_fromLAS(LMEcrs* crs, LASHeader* header);

char* LMEcrs_UTMfromPoint(LAScoord* coord);

hid_t ProjType_create(herr_t* status);

void ProjType_destroy(hid_t projtype, herr_t status);





