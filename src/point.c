#include <assert.h>
#include <hdf5.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <hilbert.h>
#include <math.h>
#include <proj_api.h>
#include <liblas/capi/liblas.h>
#include "point.h"
#include "common.h"

const double offsets[3] = {180.0, 90.0, 12300.0};
const double max = (double)UINT_MAX;


double Scale_init(double offset, double max)
{
    double range = 2.0 * offset;
    double scale = max / range;
    return scale;
}

void Point_SetIndex(Point *p, idx_t* idx) {
    p->idx = *idx;
}

void Point_IndexCoords(Point *p) {
    //uint64_t x, y;
    Hpoint *pt;
    Hcode idx;
    idx_t idxFull;
    //uint64_t *idxFull;
    //x = p->coords.x;
    //y = p->coords.y;

    pt = malloc(sizeof(Hpoint));
    pt->hcode[0] = p->coords.x;
    pt->hcode[1] = p->coords.y;
    //printf("Computing index\n");
    idx = H_encode(*pt);
    //printf("Combining indexes\n");
    //printf("Point index has size: %i, and index pieces are %i, %i", sizeof(p->idx), sizeof(idx.hcode[0]), sizeof(idx.hcode[1]));
    combineIndices(&idxFull, idx.hcode[0], idx.hcode[1]);
    //printf("Adding index\n");
    Point_SetIndex(p, &idxFull);
    free(pt);

}


void Point_SetCoords(Point* p, coord_t* coords)
{
    p->coords.x = coords->x;
    p->coords.y = coords->y;
    p->coords.z = coords->z;
}

void Coord_Set(coord_dbl_t* coords, double x, double y, double z)
{
    coords->x = x;
    coords->y = y;
    coords->z = z;
}

void Coord_Get(double coord_arr[3], coord_dbl_t* coords) {
    coord_arr[0] = coords->x;
    coord_arr[1] = coords->y;
    coord_arr[2] = coords->z;
    
}

void Coord_SetCode(coord_t* coords, uint32_t x, uint32_t y, uint32_t z)
{
    coords->x = x;
    coords->y = y;
    coords->z = z;
}

/** Coord_Encode: Applies scale and offset to a given coordinate set
    
    @param coord_raw: Structure holding 3 double precision coordinates
    @param coord_code: Structure holding 3 unsigned 32bit integer coordinate codes
**/
void Coord_Encode(coord_t* coord_code, coord_dbl_t* coord_raw)
{
    double scales[3];
    int i;
    double rawCoords[3];
    double max = (double)UINT_MAX;
    // Get the coordinate values as an array for iterating
    Coord_Get(rawCoords, coord_raw);
    
//    printf("CE:Raw coords: %f, %f, %f \n", rawCoords[0], rawCoords[1], rawCoords[2]); 
    for (i=0;i<3;i++) {
        // Encoding scheme code = (coordinate + offset) * scalx
        scales[i] = Scale_init(offsets[i], max);
        //printf("Offset: %f, Scale: %f\n", offsets[i], scales[i]);
        //printf("Coord with offset: %f\n", (rawCoords[i] + offsets[i]));
        rawCoords[i] = (rawCoords[i] + offsets[i]) * scales[i];
        //printf("Code %i is %f\n", i, rawCoords[i]);
        //printf("%f becomes %u\n", rawCoords[i], (uint32_t)rawCoords[i]);   
    }
    Coord_SetCode(coord_code, (uint32_t)rawCoords[0], (uint32_t)rawCoords[1], (uint32_t)rawCoords[2]);
    
}

void Coord_Decode(coord_dbl_t* coords, coord_t* coord_code) {
    double scales[3];
    int i;
    double outCoords[3];
    outCoords[0] = (double)coord_code->x;
    outCoords[1] = (double)coord_code->y;
    outCoords[2] = (double)coord_code->z;

    for (i=0;i<3;i++) {
        // Decoding scheme: coord = (code / scale) - offset
        //printf("Code is %f\n", outCoords[i]);
        scales[i] = Scale_init(offsets[i], max);
        //printf("Offset: %f, Scale: %f\n", offsets[i], scales[i]);
        //printf("Unscaled coord = %f\n", (outCoords[i] / scales[i]));
        outCoords[i] = (outCoords[i] / scales[i]) - offsets[i];
    }
    Coord_Set(coords, outCoords[0], outCoords[1], outCoords[2]);
}

void Point_SetReturns(Point* p, int returnNum, int returnCnt) {
    p->retns.rNum = returnNum;
    p->retns.rTot = returnCnt;
}

void Point_SetColor(Point* p, int r, int g, int b) {
    p->color.r = r;
    p->color.g = g;
    p->color.b = b;
}

void Point_SetIntensity(Point* p, int intens) {
    p->i = intens;
}

void Point_SetClassification(Point* p, unsigned char clss) {
    p->clss = clss;
}
Point* Point_new(idx_t *idx, double coords[3], int i, short retns[2], unsigned char clss, short rgb[3])
{
    Point* p = malloc(sizeof(Point));
    assert(p != NULL);
    Point_SetIndex(p, idx);
    p->coords.x = coords[0];
    p->coords.y = coords[1];
    p->coords.z = coords[2];
    p->i = i;
    p->retns.rNum = retns[0];
    p->retns.rTot = retns[1];
    p->clss = clss;
    p->color.r = rgb[0];
    p->color.g = rgb[1];
    p->color.b = rgb[2];

    return p;
}

void Point_destroy(struct Point *p)
{
    assert(p != NULL);
    free(p);
}

hid_t CoordType_create(herr_t* status) {
    /** Create the coordinate data type **/
    hid_t coordtype;
    coordtype = H5Tcreate(H5T_COMPOUND, sizeof(coord_t));
    *status = H5Tinsert(coordtype, "x", HOFFSET (coord_t, x), H5T_NATIVE_UINT32);
    *status = H5Tinsert(coordtype, "y", HOFFSET (coord_t, y), H5T_NATIVE_UINT32);
    *status = H5Tinsert(coordtype, "z", HOFFSET (coord_t, z), H5T_NATIVE_UINT32);
    return coordtype;
}



void CoordType_destroy(hid_t coordtype, herr_t* status) {
    *status = H5Tclose(coordtype);

}


hid_t ReturnType_create(herr_t* status) { 
    /** Create the return data type **/
    hid_t returntype;
    returntype = H5Tcreate(H5T_COMPOUND, sizeof(return_t));
    *status = H5Tinsert(returntype, "rNum", HOFFSET (return_t, rNum), H5T_NATIVE_SHORT);
    *status = H5Tinsert(returntype, "rTot", HOFFSET (return_t, rTot), H5T_NATIVE_SHORT);
    return returntype;
}

void ReturnType_destroy(hid_t returntype, herr_t* status) {
    *status = H5Tclose(returntype);
}

hid_t ColorType_create(herr_t* status) {
    hid_t colortype;
    colortype = H5Tcreate(H5T_COMPOUND, sizeof(color_t));
    *status = H5Tinsert(colortype, "red", HOFFSET (color_t, r), H5T_NATIVE_SHORT);
    *status = H5Tinsert(colortype, "green", HOFFSET (color_t, g), H5T_NATIVE_SHORT);
    *status = H5Tinsert(colortype, "blue", HOFFSET (color_t, b), H5T_NATIVE_SHORT);
    return colortype;
}

void ColorType_destroy(hid_t colortype, herr_t* status) {
    *status = H5Tclose(colortype);
}

hid_t PointType_create(herr_t* status) {
    hid_t coordtype, returntype, colortype, pointtype;
    coordtype = CoordType_create(status);
    returntype = ReturnType_create(status);
    colortype = ColorType_create(status);
    printf("Coord has size %lu\n", sizeof(coord_t));
    printf("Intensity has size %lu\n", sizeof(int));
    printf("Returns have size %lu\n", sizeof(return_t));

    printf("Point has size %lu\n", sizeof(Point));
    pointtype = H5Tcreate(H5T_COMPOUND, sizeof(Point));
    *status = H5Tinsert(pointtype, "idx", HOFFSET(Point, idx), H5T_STD_U64BE);
    *status = H5Tinsert(pointtype, "coords", HOFFSET(Point, coords), coordtype);
    *status = H5Tinsert(pointtype, "intensity", HOFFSET(Point, i), H5T_NATIVE_INT);
    *status = H5Tinsert(pointtype, "returns", HOFFSET(Point, retns), returntype);
    *status = H5Tinsert(pointtype, "class", HOFFSET(Point, clss), H5T_NATIVE_UCHAR);
    *status = H5Tinsert(pointtype, "color", HOFFSET(Point, color), colortype);
    //printf("Point Type created with size, %d\n", H5T_get_size(pointtype)); 
    *status = H5Tclose(coordtype);
    *status = H5Tclose(returntype);
    *status = H5Tclose(colortype);
    return pointtype;
}

    
void PointType_destroy(hid_t pointtype, herr_t* status) {
    *status = H5Tclose(pointtype);
}

int MPI_CoordType_create(MPI_Datatype *mpi_coordtype) {
    /* Create an MPI type for Coordinates */
    int nitems=3;
    int blocklengths[3] = {1,1,1}; //TODO: the number of each element
    MPI_Datatype types[3] = {MPI_UNSIGNED, MPI_UNSIGNED, MPI_UNSIGNED};
    MPI_Aint offsets[3];
    offsets[0] = offsetof(coord_t, x);
    offsets[1] = offsetof(coord_t, y);
    offsets[2] = offsetof(coord_t, z);
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, mpi_coordtype);
    MPI_Type_commit(mpi_coordtype);

    return 0;
}

int MPI_ReturnType_create(MPI_Datatype *mpi_returntype) {
    /* Create an MPI Type for returns */
    int nitems=2;
    int blocklengths[2] = {1,1};
    MPI_Datatype types[2] = {MPI_SHORT, MPI_SHORT};
    MPI_Aint offsets[2];
    offsets[0] = offsetof(return_t, rNum);
    offsets[1] = offsetof(return_t, rTot);
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, mpi_returntype);
    MPI_Type_commit(mpi_returntype);

    return 0;
}

int MPI_ColorType_create(MPI_Datatype *mpi_colortype) {
    /* Create an MPU Type for color data */
    int nitems=3;
    int blocklengths[3] = {1, 1, 1};
    MPI_Datatype types[3] = {MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR};
    MPI_Aint offsets[3];
    offsets[0] = offsetof(color_t, r);
    offsets[1] = offsetof(color_t, g);
    offsets[2] = offsetof(color_t, b);
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, mpi_colortype);
    MPI_Type_commit(mpi_colortype);
    
    return 0;
}

int MPI_PointType_create(MPI_Datatype *mpi_pointtype) {
    /* Create an MPI type for points */
    int nitems=6;
    int blocklengths[6] = {1,1,1,1,1,1};
    MPI_Datatype types[6];
    // Set the member types
    types[0] = MPI_UNSIGNED_LONG_LONG;
    MPI_CoordType_create(&types[1]);
    types[2] = MPI_SHORT; // The data comes in as a 16 bit integer, may need to check
    MPI_ReturnType_create(&types[3]);
    types[4] = MPI_UNSIGNED_CHAR;
    MPI_ColorType_create(&types[5]);
    MPI_Aint offsets[6];
    offsets[0] = offsetof(Point, idx);
    offsets[1] = offsetof(Point, coords);
    offsets[2] = offsetof(Point, i);
    offsets[3] = offsetof(Point, retns);
    offsets[4] = offsetof(Point, clss);
    offsets[5] = offsetof(Point, color);
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, mpi_pointtype);
    MPI_Type_commit(mpi_pointtype);

    return 0;
}



void Point_print(struct Point *p)
{
    printf("ID: %lu\n",  p->idx);
    printf("X:%i, Y:%i, Z: %i\n", p->coords.x, p->coords.y, p->coords.z);
    printf("Intensity: %d\n", p->i);
    printf("Return: %d of %d\n", p->retns.rNum, p->retns.rTot);
    printf("Class: %hhu\n", p->clss);
};


int LASPoint_read(LASPointH* lasPnt, Point* pnt, double* x, double* y, double* z)  {
    // Check that the point is a last return
    LASColorH color;
    //if (LASPoint_GetReturnNumber(*lasPnt) == LASPoint_GetNumberOfReturns(*lasPnt)){
    *x = LASPoint_GetX(*lasPnt);
    *y = LASPoint_GetY(*lasPnt);
    *z = LASPoint_GetZ(*lasPnt);
    Point_SetIntensity(pnt, LASPoint_GetIntensity(*lasPnt));
    Point_SetReturns(pnt, LASPoint_GetReturnNumber(*lasPnt), LASPoint_GetNumberOfReturns(*lasPnt));
    Point_SetClassification(pnt, LASPoint_GetClassification(*lasPnt));
    color = LASPoint_GetColor(*lasPnt);
    Point_SetColor(pnt, LASColor_GetRed(color), LASColor_GetGreen(color), LASColor_GetBlue(color));
    //}

    return 0;
}



int LASPoint_project(LASHeaderH* header, hsize_t* count, double* x, double* y, double* z, Point* pnts, int mpi_rank) {
    fprintf(stdout, "[%d] LASpoint_project called\n", mpi_rank);
    projPJ pj_src, pj_wgs;
    int i;
    if (!LASProj_get(header, &pj_src)) {
		fprintf(stderr, "Error loading projection from source\n");
		return 1;
	}
	if (!Proj_load("+proj=longlat +ellps=WGS84 +datum=WGS84 +vunits=m +no_defs", &pj_wgs)) {
		printf("Error loading WGS84 projection\n");
		pj_free(pj_src);
		return 1;
	}
	//Allocate space for coordinates
    coord_dbl_t* rawCoords = malloc(sizeof(coord_dbl_t));
    coord_t* cc = malloc(sizeof(coord_dbl_t));

   // printf("[%d] Sample coordinates 0 of %i: %f, %f, %f\n", mpi_rank,(int)*count, x[0], y[0], z[0]);
    pj_transform(&pj_src, &pj_wgs, *count, 1, x, y, z);

    //rawCoords = malloc(sizeof(coord_dbl_t));
    //cc =  malloc(sizeof(coord_t));
    //Scale/offset coordinates
    int prjCount = (int)(*count);
    fprintf(stdout, "[%d] Starting reprojection\n", mpi_rank);
    for (i = 0; i <  prjCount-1; i++) {
        if (i % 1000 == 0) {
			//printf("[%d], Projecting point %i\n", mpi_rank, (int)i);
		}
        Coord_Set(rawCoords, (x[i] * RAD_TO_DEG), (y[i] * RAD_TO_DEG), (z[i] * RAD_TO_DEG));
		        
        Coord_Encode(cc, rawCoords);
        Point_SetCoords(&pnts[i], cc);
        Point_IndexCoords(&pnts[i]);
    }
    
   // printf("[%d] Project finished!\n", mpi_rank);
    pj_free(pj_src);
    //free(pj_src);
    pj_free(pj_wgs);
    //free(pj_wgs);
 //   printf("[%d] Freeing data\n", mpi_rank);
    free(rawCoords);
    free(cc);

    return 0;
}
