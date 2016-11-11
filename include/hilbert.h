#ifndef HILBERT_H
#define HILBERT_H

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <coord.h>
#define DIM 2
#define ORDER 32

typedef struct {
    uint32_t  hcode[DIM];
} Hcode;

typedef Hcode Hpoint;


extern const uint32_t g_mask[];
//[3] = {4, 2, 1};

uint32_t calc_P(int i, Hcode h);

uint32_t calc_P2(uint32_t S);

uint32_t calc_T(uint32_t P);

uint32_t calc_tS_tT(uint32_t xJ, uint32_t val);

Hpoint H_decode (Hcode H);

Hcode H_encode(Hpoint pt);

uint32_t swapEndian(uint32_t num);

int combineIndices(uint64_t* idxfull, uint32_t high, uint32_t low);

int splitIndex(uint64_t fullIndex, uint32_t * high, uint32_t * low);

uint64_t encodeIndex(LMEcoordCode * code);

void decodeIndex(uint64_t fullidx, LMEcoordCode * code);

unsigned createMask(unsigned start, unsigned stop);

int less_than(uint64_t a, uint64_t b);


void printBits(size_t const size, void const * const ptr);


#endif
