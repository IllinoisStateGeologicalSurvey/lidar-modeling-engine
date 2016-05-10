/** 
 * @file crs.c
 * @author Nathan Casler
 * @date May 10 2016
 * @brief Methods for validating and transforming coordinate systems
 */

#include <stdlib.h>
#include <stdio.h>
#include <proj_api.h>
#include <string.h>
#include "crs.h"

int LMEcrs_set(LMEcrs* crs, char* projStr) {
	strcpy(crs->projStr, projStr);
	return 0;
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

int LMEcrs_fromLAS(LMEcrs* crs, LASHeader* header) {
	LASSRSH srs = LASHeader_GetSRS(*header);
	char* projStr = LASSRS_GetProj4(srs);
	if LMEcrs_check(projStr) {
		LMEcrs_set(crs, projStr);
	} else {
		fprintf("Failed to read projection from header\n");
		return 1; 
		// Find the closest UTM projection and assign it
	}
	//TODO: Check the validity of this string so nulls don't crash the program
	return 0;	
}
//
/**
 * @brief LMEcrs_UTMfromLAS: Should calculate the proj4 definition of the
 * closest UTM projection to a given point.
 */
char* LMEcrs_UTMfromPoint(LAScoord* coord) {
	int zone = (int) (floor((coord[0] + 180) / 6) % 60) + 1;
	char hemisphere[6];
	char projStr[PATH_MAX];
	if (coord[1] > 0) {
		strcpy(&hemisphere[0], "north") {
	} else {
		strcpy(&hemisphere[0], "south")
	}
	sprintf(projStr, "+proj=utm +zone=%i +%s +ellps=WGS84 +datum=WGS84 +units=m +no_defs", zone, hemisphere);
	}
	fprintf("Generated following proj string %s\n", projStr);
	return projStr;
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
