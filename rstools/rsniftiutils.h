#include <stdio.h>
#include <strings.h>
#include <ctype.h>

#include <nifti1.h>
#include <fslio.h>

#if !defined(__NIFTIUTILS_H)
#define __NIFTIUTILS_H

#ifdef __cplusplus
extern "C" {
#endif
    
    
#define BOOL    int
#define FALSE   0
#define TRUE    1
    
#define RSIOERR(x) { fprintf(stderr,"Error:: %s\n",(x)); fflush(stderr); exit(EXIT_FAILURE); }
    
typedef struct {
    unsigned int x, y, z;
} Point3D;
    
typedef struct {
    float x, y, z;
} FloatPoint3D;

Point3D      MakePoint3D(unsigned int x, unsigned int y, unsigned int z);
FloatPoint3D MakeFloatPoint3D(float x, float y, float z);
Point3D*     ReadMask(char *path, unsigned short newX, unsigned short newY, unsigned short newZ, unsigned long *nPoints, char *resampledMaskPath, FSLIO *maskPrototype, double ***resampledMaskReturn);
double***    ResampleVolume(double ***oldVolume, int oldX, int oldY, int oldZ, int newX, int newY, int newZ);
size_t       rsWriteTimeSeries(FSLIO *fslio, const void *buffer, short xVox, short yVox, short zVox, int nvols);
BOOL         rsExtractTimecourseFromBuffer(const FSLIO *fslio, double *timecourse, const void *buffer, const float slope, const float inter, const Point3D p, const int xh, const int yh, const int zh, const int th);
BOOL         rsExtractPointsFromBuffer(const FSLIO *fslio, double *data, const void *buffer, const float slope, const float inter, const Point3D* points, const unsigned long nPoints, const int t, const int xh, const int yh, const int zh, const int th);
BOOL         rsWriteTimecourseToBuffer(const FSLIO *fslio, const double *timecourse, void *buffer, const float slope, const float inter, const Point3D p, const int xh, const int yh, const int zh, const int th);
BOOL         rsResetBufferToValue(const int datatype, void *buffer, const float slope, const float inter, const int xh, const int yh, const int zh, const int th, const int threads, const double value);
    
BOOL convertScaledDoubleToBuffer(int datatype, void *outbuf,   double *inbuf, float slope, float inter, int xh, int yh, int zh, BOOL multidim);
void convertScaledDoubleToBuffer_UINT8(  THIS_UINT8 *outbuf,   double *inbuf, float slope, float inter, int xh, int yh, int zh, BOOL multidim);
void convertScaledDoubleToBuffer_INT8(   THIS_INT8 *outbuf,    double *inbuf, float slope, float inter, int xh, int yh, int zh, BOOL multidim);
void convertScaledDoubleToBuffer_UINT16( THIS_UINT16 *outbuf,  double *inbuf, float slope, float inter, int xh, int yh, int zh, BOOL multidim);
void convertScaledDoubleToBuffer_INT16(  THIS_INT16 *outbuf,   double *inbuf, float slope, float inter, int xh, int yh, int zh, BOOL multidim);
void convertScaledDoubleToBuffer_UINT64( THIS_UINT64 *outbuf,  double *inbuf, float slope, float inter, int xh, int yh, int zh, BOOL multidim);
void convertScaledDoubleToBuffer_INT64(  THIS_INT64 *outbuf,   double *inbuf, float slope, float inter, int xh, int yh, int zh, BOOL multidim);
void convertScaledDoubleToBuffer_UINT32( THIS_UINT32 *outbuf,  double *inbuf, float slope, float inter, int xh, int yh, int zh, BOOL multidim);
void convertScaledDoubleToBuffer_INT32(  THIS_INT32 *outbuf,   double *inbuf, float slope, float inter, int xh, int yh, int zh, BOOL multidim);
void convertScaledDoubleToBuffer_FLOAT32(THIS_FLOAT32 *outbuf, double *inbuf, float slope, float inter, int xh, int yh, int zh, BOOL multidim);
void convertScaledDoubleToBuffer_FLOAT64(THIS_FLOAT64 *outbuf, double *inbuf, float slope, float inter, int xh, int yh, int zh, BOOL multidim);

char *rsLeftTrimString(char *s);
char *rsRightTrimString(char *s);
char *rsTrimString(char *s);
    
#ifdef __cplusplus
}
#endif
    
#endif