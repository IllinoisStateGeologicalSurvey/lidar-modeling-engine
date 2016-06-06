/***************************************************************
 * Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  *
 * Information Laboratory (CIGI). All Rights Reserved.         *
 *                                                             *
 ***************************************************************/

/** 
 * @file filter.c
 * @author Nathan Casler
 * @date May 6 2016
 * @brief File containing point filtration methods
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <hdf5.h>
#include <math.h>
#include <inttypes.h>
#include <limits.h>
#include <proj_api.h>
#include <liblas/capi/liblas.h>
#include "point.h"
#include "bound.h"
#include "filter.h"

/** 
 * @brief Filter_Create: Create a point filter object
 *
 * @return filter: #LMEfilter = Filter object used to parse a LAS file
 */

int LMEfilter_create(LMEfilter* filter) {
	filter->retn = 0;
	filter->intens =  0;
	filter->clas = (char)0;
	LMEcoord ll, ur;
	LMEcoord_set(&ll, -180.0, -90.0, 0.0);
	LMEcoord_set(&ur, 180.0, 90.0, 0.0);
	LMEbound_set(&filter->range, &ll, &ur);
	
	return 0;
}

/**
 * @brief Filter_Set: Setter for filter attributes
 *
 * This will set the values for various filter classes
 * @param filter: #LMEfilter = Filter object to hold settings
 * @param range: #bound_dbl_t = The bounding box to use when filtering points
 * @param retn: int = The return number to match points against
 * @param intensity: int = The intensity values to match points against
 * @param classification: char = The point classification to read from the
 * points
 * @return 0
 * @note Should update the return to account for errors(no point or read
 * failure)
 * @note Should update return, intensity and classification to account for
 * ranges of values
 */
int LMEfilter_set(LMEfilter* filter, LMEbound* range, int retn, int intensity, char classification) {
	filter->range = *range;
	filter->retn = retn;
	filter->intens = intensity;
	filter->clas = classification;
	return 0;
}
/**
 * @brief Filter_SetRange: Set the bounding box for the filter object
 *
 * @param filter: #LMEfilter = The filter object to update
 * @param range: #bound_dbl_t = The bounding box to apply to the filter
 * @return 0
 * @note The range is set in EPSG:4326 decimal degrees
 */
int LMEfilter_setRange(LMEfilter* filter, LMEbound* range) {
	filter->range.low = range->low;
	filter->range.high = range->high;
	return 0;
}

/** 
 * @brief Filter_SetReturn: Set the return value for the filter object
 *
 * @param filter: #LMEfilter = The filter object to update
 * @param retnFlag: int = The return value to set
 * @return 0
 */
int LMEfilter_setReturn(LMEfilter* filter, int retnFlag) {
	filter->retn = retnFlag;
	return 0;
}
/** 
 * @brief Filter_Destroy: Frees memory occupied by a filter object
 *
 * @param filter: #LMEfilter = The filter object to free
 */
void LMEfilter_destroy(LMEfilter* filter) {
	free(filter);
}

/**
 * @brief Filter_RangeCheck: Check if a point falls within bounding filter
 *
 * @param filter: #LMEfilter = The filter holding a range filter
 * @param lasPnt: LASPointH = Pointer to a point in a LAS file
 * @return 1 if point is contained by the filter, else 0
 * @note: There is probably a more efficient/accurate method to do this
 */
int LMEfilter_rangeCheck(LMEfilter* filter, LASPointH* lasPnt) {
	double x,y;
	x = LASPoint_GetX(*lasPnt);
	y = LASPoint_GetY(*lasPnt);
	// Not implemented yet
	//z = LASPoint_GetZ(*lasPnt);
	//printf("Is (%f,%f) in (%f,%f)(%f,%f)\n", x, y, filter->range.low.x, filter->range.low.y, filter->range.high.x, filter->range.high.y);	
	//printf("Is %f < %f or %f < %f\n", x, filter->range.low.x, y, filter->range.low.y);
	if (x < LMEcoord_getX(&filter->range.low)) {
		//printf("Left\n");
		return 0;
	}
	else if (y < LMEcoord_getY(&filter->range.low)) {
		//Point is below/left of box
		//printf("Point Y: %f < %f Bound Low Y\n", y, filter->range.low.y);
	//	printf("Below\n");
		return 0;
	} else if (x > LMEcoord_getX(&filter->range.high)) {
		//printf("Right\n");
		return 0;
	} else if (y > LMEcoord_getY(&filter->range.high)) {
		//Point is above/right of box
		//printf("Above\n");
		return 0;
	} else {
		//printf("WITHIN\n");
		return 1;
	}
}

/**
 * @brief Filter_ReturnCheck: Check if a point matches the return filter value
 *
 * @param filter: #LMEfilter = Filter object with a return filter to apply
 * @param lasPnt: LASPointH = Pointer to a point in an LAS file to filter
 * @return 1 if point matches the return filter
 * @note Currently only matches first or last returns. First if the filter
 * = 1, last if the filter = 2, no matching if filter = 0, else print error
 */
int LMEfilter_returnCheck(LMEfilter* filter, LASPointH* lasPnt) {
	// TODO: Make this check more robust
	// FOR NOW: 0 => Don't filter returns
	// 1 => First Returns
	// 2 => Last Returns
	int flag = 0;
	switch(filter->retn) {
		case 0:
			flag = 1;
			break;
		case 1:
			if (LASPoint_GetReturnNumber(*lasPnt) == 1) {
				flag = 1;
			}
			break;
		case 2:
			if (LASPoint_GetReturnNumber(*lasPnt) == LASPoint_GetNumberOfReturns(*lasPnt)) {
				//printf("Last Return\n");
				flag = 1;
			}
			break;
		default: 
			printf("Error: Return filter %i, not supported.\n", filter->retn);
	}
	return flag;
}
