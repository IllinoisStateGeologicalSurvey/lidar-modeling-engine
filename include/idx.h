#ifndef LME_IDX_H
#define LME_IDX_H

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>


typedef struct LMEidx {
	uint64_t val;
} LMEidx;

void LMEidx_set(LMEidx* idx, const * LMEcoordCode coord);

#endif
