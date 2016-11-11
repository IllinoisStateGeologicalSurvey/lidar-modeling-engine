#ifndef LME_CRS_H
#define LME_CRS_H

#include <stdlib.h>
#include <stdio.h>
#include <proj_api.h>
#include <string.h>
#include <liblas/capi/liblas.h>
#include "coord.h"

#define PATH_SIZE 1024

typedef struct LMEcrs {
	int type;
	char projStr[1024];
} LMEcrs;

int LMEcrs_set(LMEcrs* crs, char* projStr);

/** @brief LMEcrs_init: This will initiliaze the projPJ object with the given CRS
 * definition. There should probably be a better way to manage the projPJ
 * instance within the LMEcrs class.
 * @param crs (LMEcrs*) Coordinate system class holding the projection
 * definition to use
 * @param pj (projPJ*) Proj4 projection object to initialize
 * @return 1 if successful, else 0
 */
int LMEcrs_init(LMEcrs* crs, projPJ* pj);
int LMEcrs_check(char* projStr);

int LMEcrs_fromLAS(LMEcrs* crs, LASHeaderH * const header);
/** @brief LMEcrs_setWGS: Set the Coordinate System object to WGS84
 * @param crs (LMEcrs*) The coordinate system object to set
 * @return 1 if coordinate system set successfully, else 0
 */
int LMEcrs_setWGS(LMEcrs* crs);

int LMEcrs_UTMfromPoint(LMEcoord* coord, char* projStr);

hid_t ProjType_create(herr_t* status);

void ProjType_destroy(hid_t projtype, herr_t* status);

#endif
