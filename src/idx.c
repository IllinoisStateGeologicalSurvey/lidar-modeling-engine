#include <stdlib.h>
#include <stdio.h>
#include "hilbert.h"
#include "idx.h"
#include "coord.h"

void LMEidx_set(LMEidx* idx, const * LMEcoordCode coord) {
	Hpoint *pt = (Hpoint *)malloc(sizeof(Hpoint));
	Hcode hdx;
	pt->hcode[0] = LMEcoord_getX(coord);
	pt->hcode[1] = LMEcoord_getY(coord);
	hdx = H_encode(*pt);
	combineIndices(idx, hdx.hcode[0], hdx.hcode[1]);
	free(pt);
}
