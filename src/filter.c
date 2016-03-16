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


filter_t* Filter_Create() {
	filter_t* filter = (filter_t*)malloc(sizeof(filter_t));
	filter->retn = 0;
	filter->intens =  0;
	filter->clas = (char)0;
	Bound_dbl_Set(&filter->range, -180.0, -90.0, 0.0, 180.0, 90.0, 0.0);
	
	return filter;
}

int Filter_Set(filter_t* filter, bound_dbl_t* range, int retn, int intensity, char classification) {
	filter->range = *range;
	filter->retn = retn;
	filter->intens = intensity;
	filter->clas = classification;
	return 0;
}

int Filter_SetRange(filter_t* filter, bound_dbl_t* range) {
	filter->range.low = range->low;
	filter->range.high = range->high;
	return 0;
}

int Filter_SetReturn(filter_t* filter, int retnFlag) {
	filter->retn = retnFlag;
	return 0;
}

void Filter_Destroy(filter_t* filter) {
	free(filter);
}

int Filter_RangeCheck(filter_t* filter, LASPointH* lasPnt) {
	double x,y;
	x = LASPoint_GetX(*lasPnt);
	y = LASPoint_GetY(*lasPnt);
	// Not implemented yet
	//z = LASPoint_GetZ(*lasPnt);
	//printf("Is (%f,%f) in (%f,%f)(%f,%f)\n", x, y, filter->range.low.x, filter->range.low.y, filter->range.high.x, filter->range.high.y);	
	//printf("Is %f < %f or %f < %f\n", x, filter->range.low.x, y, filter->range.low.y);
	if (x < filter->range.low.x) {
		//printf("Left\n");
		return 0;
	}
	else if (y < filter->range.low.y) {
		//Point is below/left of box
		//printf("Point Y: %f < %f Bound Low Y\n", y, filter->range.low.y);
	//	printf("Below\n");
		return 0;
	} else if (x > filter->range.high.x) {
		//printf("Right\n");
		return 0;
	} else if (y > filter->range.high.y) {
		//Point is above/right of box
		//printf("Above\n");
		return 0;
	} else {
		//printf("WITHIN\n");
		return 1;
	}
}

int Filter_ReturnCheck(filter_t* filter, LASPointH* lasPnt) {
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
