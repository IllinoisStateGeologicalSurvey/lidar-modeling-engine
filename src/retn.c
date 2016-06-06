#include <stdlib.h>
#include <stdio.h>
#include <hdf5.h>
#include <liblas/capi/liblas.h>
#include "retn.h"

void LMEreturn_set(LMEreturn* retn, int returnNum, int returnTot) {
	retn->rNum = (short) returnNum;
	retn->rTot = (short) returnTot;
}

int LMEreturn_isLast(LMEreturn* retn) {
	if (retn->rNum == retn->rTot) {
		return 1;
	} else {
		return 0;
	}
}

int LMEreturn_isFirst(LMEreturn* retn) {
	if (retn->rNum == 0) {
		return 1;
	} else {
		return 0;
	}
}

int LMEreturn_fromLAS(LMEreturn* retn, LASPointH * const lasPnt) {
	LMEreturn_set(retn, LASPoint_GetReturnNumber(*lasPnt), LASPoint_GetNumberOfReturns(*lasPnt));	
	return 1;
}

hid_t ReturnType_create(herr_t* status) {
	hid_t returntype;
	returntype = H5Tcreate(H5T_COMPOUND, sizeof(LMEreturn));
	*status = H5Tinsert(returntype, "rNum", HOFFSET (LMEreturn, rNum), H5T_NATIVE_SHORT);
	*status = H5Tinsert(returntype, "rTot", HOFFSET (LMEreturn, rTot), H5T_NATIVE_SHORT);
	return returntype;
}

void ReturnType_destroy(hid_t returntype, herr_t* status) {
	*status = H5Tclose(returntype);
}
