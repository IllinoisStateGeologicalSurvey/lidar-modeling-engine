#ifndef HILBERT_H
#define HILBERT_H


typedef struct {
    uint32_t  hcode[3];
} Hcode;

typedef Hcode Hpoint;

const uint32_t g_mask[3] = {4, 2, 1};

uint32_t calc_P(int i, Hcode h);

uint32_t calc_P2(uint32_t S);

uint32_t calc_T(uint32_t P);

uint32_t calc_tS_tT(uint32_t xJ, uint32_t val);

Hpoint H_decode (Hcode H);

Hcode H_encode(Hpoint pt);

void printBits(size_t const size, void const * const ptr);


#endif
