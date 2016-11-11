/***************************************************************
 * Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  *
 * Information Laboratory (CIGI). All Rights Reserved.         *
 *                                                             *
 ***************************************************************/

/** 
 * @file hilbert-test.c
 * @author Nathan Casler
 * @date May 6 2016
 * @brief Test to check if the hilbert indexing is functional
 *
 */
#include "hilbert.h"
#include "point.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
/** TODO FIND the reason that Y coordinates are sometimes offset by +/- 90 degrees. **/

/** Function to test whether the encoding is functioning properly **/
int checkCoding(LMEcoord * coordIn)
{
    LMEcoord  coordOut;
    LMEcoordCode ccIn, ccOut;
    Hpoint pt;
    Hcode idx;
    uint64_t idxFull;
    //coordOut = malloc(sizeof(LMEcoord));
    //ccIn = malloc(sizeof(LMEcoordCode));
    //ccOut = malloc(sizeof(LMEcoordCode));
    LMEcoord_encode(&ccIn, coordIn);
    //pt = malloc(sizeof(Hpoint));
    pt.hcode[0] = LMEcoordCode_getX(&ccIn);
    pt.hcode[1] = LMEcoordCode_getY(&ccIn);
	printf("Pre-encoding: %"PRIu32" %"PRIu32"\n", pt.hcode[0], pt.hcode[1]);
    idx = H_encode(pt);
	printf("Post-encoding: %"PRIu32" %"PRIu32"\n", idx.hcode[0], idx.hcode[1]);
    combineIndices(&idxFull, idx.hcode[0], idx.hcode[1]);
	printf("Full index: %" PRIu64 "\n", idxFull);
	//splitIndex(idxFull, &idx.hcode[0], &idx.hcode[1]);
	printf("Pre-decode: %"PRIu32" %"PRIu32"\n", idx.hcode[0], idx.hcode[1]);
    //NOTE THE y coordinate is occaisionally coming back 90degrees off 
	pt = H_decode(idx);
	printf("Post-decode: %"PRIu32" %"PRIu32"\n", pt.hcode[0], pt.hcode[1]);
    LMEcoordCode_setX(&ccOut, pt.hcode[0]);
    LMEcoordCode_setY(&ccOut, pt.hcode[1]);
    LMEcoordCode_setZ(&ccOut, LMEcoordCode_getZ(&ccIn));
    LMEcoord_decode(&coordOut, &ccOut);
    //printf("Output coordinates:[ %f, %f, %f ]\n", coordOut->x, coordOut->y, coordOut->z);
    double dist = LMEcoord_distance(coordIn, &coordOut);
	if (dist > 1) {
		printf("Distance is : %f\n", dist);
	//printf("YCodeDiff is %u\n", (LMEcoord_getY(coordIn) - LMEcoord_getY(&coordOut)));
		printf("In: ");
		LMEcoord_print(coordIn);
		printf("Out: ");
		LMEcoord_print(&coordOut);
		printf("YDiff is %f\n", (LMEcoordCode_getY(&ccIn) - LMEcoordCode_getY(&ccOut)));
	}

    //free(ccIn);
    //free(ccOut);
    //free(pt);
    return 0;
}


uint64_t getCode(LMEcoord * coordIn) {
    
    LMEcoordCode ccIn;
    Hpoint pt;
    Hcode idx;
    uint64_t idxFull;
    //ccIn = malloc(sizeof(LMEcoordCode));
    LMEcoord_encode(&ccIn, coordIn);
    //pt = malloc(sizeof(Hpoint));
    pt.hcode[0] = LMEcoordCode_getX(&ccIn);
    pt.hcode[1] = LMEcoordCode_getY(&ccIn);
    idx = H_encode(pt);
    //idxFull = malloc(sizeof(uint64_t));
    combineIndices(&idxFull, idx.hcode[0], idx.hcode[1]);
    //free(ccIn);

    return idxFull;
}

double randomDouble(double min, double max) {
	assert(max > min);
	// Create a random coefficient to use
	double random = ((double) rand()) / (double) RAND_MAX;
	// Generate a double within the given range
	double range = max - min;
	return (random * range) + min;
}

int randomCoord(LMEcoord * coord) {
	double mins[3] = {-180.0, -90.0, -12300.0};
	double maxs[3] = {180.0, 90.0, 12300.0};
	double args[3];
	int i = 0;
	for (i = 0; i < 3; i++) {
		args[i] = randomDouble(mins[i], maxs[i]);
	}

	LMEcoord_set(coord, args[0], args[1], args[2]);
	return 0;
}


int main(int argc, char** argv[])
{
    double coords[3];
    //coord_dbl_t *coord1, *coord2, *coord3;
    //coord1 = malloc(sizeof(coord_dbl_t));
    LMEcoord coordTest;
	LMEcoordCode code;
	uint64_t cache[100];
	int i =0;
	for (i = 0; i < 100; i++) {
		randomCoord(&coordTest);
		LMEcoord_encode(&code, &coordTest);
		cache[i] = encodeIndex(&code);
		printf("Testing point: %i\n", i);
		printf("Code: %"PRIu64 "\n", cache[i]);
		checkCoding(&coordTest);
	}
	//coords[0] = -180.0000;
    //coords[1] = -90.0000;
    //coords[2] = -12300.0;
    //checkCoding(coords[0], coords[1], coords[2]);

    //Coord_Set(coord1, coords[0], coords[1], coords[2]);
    //printf("%lf, %lf\n", coord1->x, coord1->y);

    //coord2 = malloc(sizeof(coord_dbl_t));
    //coords[0] = 180.0000;
    //coords[1] = 90.0000;
    //coords[2] = 12300.0;
    //checkCoding(coords[0], coords[1], coords[2]);
    ///Coord_Set(coord2, coords[0], coords[1], coords[2]);
    //printf("%lf, %lf\n", coord2->x, coord2->y);

    //coord3 = malloc(sizeof(coord_dbl_t));
    //coords[0] = 0.000000657;
    //coords[1] = 0.000000657;
    //coords[2] = 0.0;
    //checkCoding(coords[0], coords[1], coords[2]);

    //coords[0] = -50.0000;
    //coords[1] = -50.0000;
    //coords[2] = 1500;
    //checkCoding(coords[0], coords[1], coords[2]);

    //coords[0] = -1.0000;
    //coords[1] = -1.0000;
    //coords[2] = -1.0000;
    //checkCoding(coords[0], coords[1], coords[2]);

    //coords[0] = 1.0000;
    //coords[1] = 1.00000;
    //coords[2] = 1.0000;
    //checkCoding(coords[0], coords[1], coords[2]);

    //coords[0] = -89.0000;
    //coords[1] = -89.00000;
    //coords[2] = 1.0000;
    //checkCoding(coords[0], coords[1], coords[2]);

	printf("Finished Hilbert test\n");

    return 0;
}
