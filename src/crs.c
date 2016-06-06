/** 
 * @file crs.c
 * @author Nathan Casler
 * @date May 10 2016
 * @brief Methods for validating and transforming coordinate systems
 */

#include <stdlib.h>
#include <stdio.h>
#include <proj_api.h>
#include <liblas/capi/liblas.h>
#include <string.h>
#include "crs.h"

#define PROJ_LEN  4096

int LMEcrs_set(LMEcrs* crs, char* projStr) {
	strcpy(crs->projStr, projStr);
	return 0;
}

int LMEcrs_init(LMEcrs* crs, projPJ* pj) {
	pj = pj_init_plus(crs->projStr);
	int out;
	if (!pj) {
		fprintf(stderr, "Failed to intialize projection from CRS\n");
		out = 0; // failure
	} else {
		out = 1;
	}
	return out;
}
/** 
 * @brief LMEcrs_check: Validates that a projStr will return a projPJ instance
 */
int LMEcrs_check(char* projStr) {
	projPJ proj;
	proj = pj_init_plus(projStr);
	if (!proj) {
		fprintf(stderr, "Failed to initialize projection with string %s\n", projStr);
		return 0;
	} else {
		pj_free(proj);
		return 1;
	}
}

int LMEcrs_setWGS(LMEcrs* crs) {
	char * projStr = (char *)malloc(sizeof(char) * PROJ_LEN);
	sprintf(projStr, "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
	int out;
	if (LMEcrs_check(projStr)) {
		LMEcrs_set(crs, projStr);
		out = 1;
	} else {
		fprintf(stderr, "Failed to read WGS84 projection\n");
		out = 0;
	}
	free(projStr);
	return out;
}

int LMEcrs_fromLAS(LMEcrs* crs, LASHeaderH* header) {
	LASSRSH srs = LASHeader_GetSRS(*header);
	int out;
	char* projStr = LASSRS_GetProj4(srs);
	if (LMEcrs_check(projStr)) {
		LMEcrs_set(crs, projStr);
		out = 1;
	} else {
		fprintf(stdout, "Failed to read projection from header\n");
		out = 0; 
		// Find the closest UTM projection and assign it
	}
	//TODO: Check the validity of this string so nulls don't crash the program
	return out;	
}
//
/**
 * @brief LMEcrs_UTMfromLAS: Should calculate the proj4 definition of the
 * closest UTM projection to a given point.a
 */
int LMEcrs_UTMfromPoint(LMEcoord* coord, char* projStr) {
	int zone = ((int) floor((coord->dims[0] + 180) / 6) % 60) + 1;
	char hemisphere[6];
	//char projStr[PATH_MAX];
	if (coord->dims[1] > 0) {
		strcpy(&hemisphere[0], "north"); 
	} else {
		strcpy(&hemisphere[0], "south");
	}
	sprintf(projStr, "+proj=utm +zone=%i +%s +ellps=WGS84 +datum=WGS84 +units=m +no_defs", zone, hemisphere);
	
	fprintf(stdout, "Generated following proj string %s\n", projStr);
	return 0;
}


hid_t ProjType_create(herr_t* status) {
	hid_t projtype;
	projtype = H5Tcopy(H5T_C_S1);
	*status = H5Tset_size(projtype, PATH_MAX);
	return projtype;
}

void ProjType_destroy(hid_t projtype, herr_t* status) {
	*status = H5Tclose(projtype);
}
