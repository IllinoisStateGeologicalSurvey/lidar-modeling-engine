#ifndef LME_IDX_H
#define LME_IDX_H

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "coord.h"

typedef struct LMEidx {
	uint64_t val;
} LMEidx;

void LMEidx_set(LMEidx* idx, LMEcoordCode * const coord);

#endif
