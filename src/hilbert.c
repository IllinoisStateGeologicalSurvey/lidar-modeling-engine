/****************************************************
 *                                                  *
 * This file contains methods for the filter object *
 *                                                  *
 ***************************************************/

/** 
 * @file hilbert.c
 * @author Nathan Casler
 * @date May 6 2016
 * @brief File containing point filtration methods
 *
 * Code copied from 
 *   J.K. Lawder, Calculation of Mappings between One and n-dimensional Values 
 *   Using the Hilbert Space-filling Curve, Research Report JL1/00, 
 *   School of Computer Science and Information Systems, Birkbeck College, University of London, 2000
 *
 *   This code assumes the following
 *
 *  The macro ORDER corresponds to the order of curve and is 32,
 *  thus coordinates are 32bit values.
 *
 *  A uint32_t should be a 32bit unsigned integer.
 *
 *  The macro DIM corresponds tot he number of dimensions in a space.
 *
 *  The derived-key of a Hpoint is stored in an Hcode which is an
 *  array of uint32_t. The bottom bit of a derived-key is held in the 
 *  bottom bit of the hcode[0] element of an Hcode and the top bit
 *  of a derived-key is held in the top bit of the hcode[DIM-1]
 *  element of and Hcode.

 *  g_mask is a global array of masks which helps simplify some
 *  calculations - it has DIM elements. In each element, only 
 *  one bit is zeo valued - the top bit in element no. 0 and the
 *  bottom bit in element no. (DIM - 1). eg.
 *  #if DIM == 5 const uint32_t g_mask[] = {16, 8, 4, 2, 1}; #endif
 *  #if DIM == 6 const uint32_t g_mask[] = {32, 16, 8, 4, 2, 1}; #endif
 *  etc...
 *
 */
#ifndef HILBERT_C
#define HILBERT_C
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <hilbert.h>
#include <coord.h>
#define DIM 2

//#if DIM == 4 const uint32_t g_mask[] = {8, 4, 2, 1}; #endif
//#if DIM == 5 const uint32_t g_mask[] = {16, 8, 4, 2, 1}; #endif
//#if DIM == 6 const uint32_t g_mask[] = {32, 16, 8, 4, 2, 1}; #endif


//typedef uint32_t    uint32_t;

//#define ORDER   32

//typedef struct {
//    uint32_t   hcode[DIM];
//}Hcode;

//typedef Hcode Hpoint;

//const uint32_t g_mask[] = {4, 2, 1};
//const uint32_t g_mask[3] = {4, 2, 1};
const uint32_t g_mask[] = {2, 1};
/*==============================================================*/
/*                          calc_P                              */
/*==============================================================*/
uint32_t calc_P(int i, Hcode H)
{
    int element;
    uint32_t P, temp1; //TODO: CHECK why temp2 isn't used
    //uint32_t P, temp1, temp2;

    element = i / ORDER;
    P = H.hcode[element];
    if (i % ORDER > ORDER - DIM)
    {
        temp1 = H.hcode[element + 1];
        P >>= i % ORDER;
        temp1 <<= ORDER - i % ORDER;
        P |= temp1;
    }
    else
        P >>= i % ORDER; /* P is a DIM bit hcode */

    /* the & masks out spurious highbit values */
    #if DIM < ORDER
        P &= (1 << DIM) -1;
    #endif

    return P;
}

/*==============================================================*/
/*                         calc_P2                              */
/*==============================================================*/
uint32_t calc_P2(uint32_t S)
{
    int i;
    uint32_t P;

    P = S & g_mask[0];
    for (i = 1; i < DIM; i++)
        if ((S & g_mask[i]) ^ ((P >> 1) & g_mask[i]))
            P |= g_mask[i];

    return P;
}


/*===============================================================*/
/*                        calc_J                                 */
/*===============================================================*/
uint32_t calc_J (uint32_t P)
{
    int i;
    uint32_t J;

    J = DIM;
    for (i = 1; i < DIM; i++) {
        if ((P >> i & 1) == (P & 1))
            continue;
        else
            break;
    }
    if (i != DIM)
        J -= i;

    return J;
}


/*================================================================*/
/*                       calc_T                                   */
/*================================================================*/
uint32_t calc_T (uint32_t P)
{
    if (P < 3)
        return 0;
    if (P % 2)
        return (P - 1) ^ (P - 1) / 2;
    return (P - 2) ^ (P - 2) / 2;
}

/*================================================================*/
/*                      calc_tS_tT                                */
/*================================================================*/
uint32_t calc_tS_tT(uint32_t xJ, uint32_t val)
{
    uint32_t retval, temp1, temp2;

    retval = val;

    if (xJ % DIM != 0)
    {
        temp1 = val >> xJ % DIM;
        temp2 = val << (DIM - xJ % DIM);
        retval = temp1 | temp2;
        retval &= ((uint32_t)1 << DIM) - 1;
    }

    return retval;
}


/*=================================================================*/
/*                      H_decode                                   */
/*=================================================================*/
/* For mapping from one dimension to DIM dimensions */

Hpoint H_decode (Hcode H)
{
    uint32_t mask = (uint32_t) 1 << (ORDER - 1), 
          A, W = 0, S, tS, T, tT, J, P = 0, xJ;
    Hpoint pt = {{0}};
    int i = ORDER * DIM - DIM, j;

    P = calc_P(i, H);
    J = calc_J(P);
    xJ = J - 1;
    A = S = tS = P ^ P / 2;
    T = calc_T(P);
    tT = T;
    
    /*------- distrib bits to coords -------*/
    for (j = DIM - 1; P > 0; P >>= 1, j--)
        if (P & 1)
            pt.hcode[j] |= mask;

    for (i -= DIM, mask >>= 1; i >= 0; i -= DIM, mask >>= 1)
    { 
        P = calc_P(i, H);
        S = P ^ P / 2;
        tS = calc_tS_tT(xJ, S);
        W ^= tT;
        A = W ^ tS;

        /*----- distrib bits to coords -----------*/
        for (j = DIM - 1; A > 0; A >>= 1, j--)
            if (A & 1)
                pt.hcode[j] |= mask;

        if (i > 0)
        {
            T = calc_T(P);
            tT = calc_tS_tT(xJ, T);
            J = calc_J(P);
            xJ += J - 1;
        }
    }
    return pt;
}



/*=================================================================*/
/*                         H_encode                                */
/*=================================================================*/
/* For mapping from DIM dimensions to one dimension */

Hcode H_encode(Hpoint pt)
{
    uint32_t  mask = (uint32_t)1 << (ORDER - 1), element,
           A, W = 0, S, tS, T, tT, J, P = 0, xJ;

    Hcode    h = {{0}};
    int i = ORDER * DIM - DIM, j;

    for (j = A = 0; j < DIM; j++)
        if (pt.hcode[j] & mask)
            A |= g_mask[j];

    S = tS = A;

    P = calc_P2(S);

    /* add in DIM bits to hcode */
    element = i / ORDER;
    if (i % ORDER > ORDER - DIM)
    {
        h.hcode[element] |= P << i % ORDER;
        h.hcode[element + 1] |= P >> (ORDER - i % ORDER);
    }
    else
        h.hcode[element] |= P << (i - element * ORDER);

    J = calc_J(P);
    xJ = J - 1;
    T = calc_T(P);
    tT = T;

    for (i -= DIM, mask >>= 1; i >= 0; i -= DIM, mask >>= 1)
    {
        for (j = A = 0; j < DIM; j++)
            if (pt.hcode[j] & mask)
                A |= g_mask[j];

        W ^= tT;
        tS = A ^ W;
        S = calc_tS_tT(xJ, tS);
        P = calc_P2(S);

        /* add in DIM bits to hcode */
        element = i / ORDER;
        if (i % ORDER > ORDER - DIM)
        {
            h.hcode[element] |= P << i % ORDER;
            h.hcode[element + 1] |= P >> (ORDER - i % ORDER);
        }
        else
            h.hcode[element] |= P << (i - element * ORDER);

        if (i > 0)
        {
            T = calc_T(P);
            tT = calc_tS_tT(xJ, T);
            J = calc_J(P);
            xJ += J - 1;
        }

    }
    return h;
}

void printBits(size_t const size, void const * const  ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i,j;

    for (i=size-1; i>=0; i--)
    {
        for (j=7;j>=0; j--)
        {
            byte = b[i] & (i<<j);
            byte >>= j;
            printf("%u", byte);
        }

    }
    puts("");
}

int combineIndices(uint64_t *outIdx, uint32_t high, uint32_t low) {
    uint32_t low_le, high_le;
	//Create little endian versions of inputs
	low_le = swapEndian(low);
	high_le = swapEndian(high);
	uint64_t idx = (uint64_t) high_le << 32;
    //printBits(sizeof(uint64_t), &idx);
    *outIdx = idx | low_le;
    //printf("%"PRIu64"\n", outIdx);
    //printBits(sizeof(uint64_t), outIdx);
    return 0;
}

int splitIndex(uint64_t fullIndex, uint32_t * high, uint32_t * low) {
	uint32_t tmp;
	tmp = (uint32_t) fullIndex & 0x0000ffff;
	// Change to big endian for decryption 
	*low = swapEndian(tmp);
	tmp =  (fullIndex  & 0xffff0000) >> 32;
	*high = swapEndian(tmp);
	return 0;
}

uint32_t swapEndian(uint32_t num) {
	// Swap big to little endian
	uint32_t swap;
	swap = ((num>>24)&0xff) | // move byte 3 to byte 0
		((num<<8)&0xff0000) | // move byte 1 to byte 2
		((num>>8)&0xff00)   | // move byte 2 to byte 1
		((num<<24)&0xff000000); // move byte 0 to byte 3
	return swap;
}
/**
// TODO: MAKE THE SCALE CONVERSION WORK FOR FLOAT ARRAY
void scaleCoords(double *coords, Hpoint* pt) {
    //uint32_t latOff = 90.0f, lonOff = 180.0f, altOff = 12300.0f;
    double offsets[3] = {180.0, 90.0, 12300.0};
    printf("UINTMAX: %lf\n", UINT_MAX);
    printf("Input coordinates: %lf, %lf, %lf\n", coords[0], coords[1], coords[2]);
    int i;
    uint32_t scales[3];

    for (i = 0; i < 3; i++) {
        
        printf("Scaling coord %d\n", i);
        double range = 2.0 * offsets[i];
        double max = (double)UINT_MAX;
        double scale = max / range;
        double outCoord = scale * coords[i];
        printf("%lf / %lf = %lf\n", max, range, scale);
        printf("Out: %lf\n", outCoord);
        scales[i] = floor(max / range);
        //printf("Scale %d is: %u, offset: %f\n", i, scales[i], offsets[i]); 


        printf("Offset coord[%d] is %f\n", i, (coords[i] + offsets[i]));
        pt->hcode[i] = (uint32_t)((coords[i] + offsets[i]) * scales[i]);
        printf("Coord %d has value %lu\n", i, pt->hcode[i]);
    }    

}**/


int less_than(uint64_t a, uint64_t b) {
    uint64_t tmp = a ^ b;
    tmp &= -tmp;
    return tmp > a || tmp & a;
}

unsigned createMask(unsigned start, unsigned stop)
{
    unsigned mask;
    mask = ((1 << stop) - 1) << start;
    return mask;
}

uint64_t encodeIndex(LMEcoordCode * code) {
	Hpoint pt;
	Hcode idx;
	uint64_t fullIndex;
	pt.hcode[0] = LMEcoordCode_getX(code);
	pt.hcode[1] = LMEcoordCode_getY(code);
	// Encode the indexes
	idx = H_encode(pt);
	// Swap encoding and join into 64 bit uint
	combineIndices(&fullIndex, idx.hcode[0], idx.hcode[1]);
	return fullIndex;

}

void decodeIndex(uint64_t fullIdx, LMEcoordCode * code)  {
	Hpoint pt;
	Hcode idx;
	// Break up 64 bit uint and swap encoding 
	splitIndex(fullIdx, &idx.hcode[0], &idx.hcode[1]);
	// Decode index
	pt = H_decode(idx);
	// Apply to encoded coord
	LMEcoordCode_setX(code, pt.hcode[0]);
	LMEcoordCode_setY(code, pt.hcode[1]);
}

#endif
