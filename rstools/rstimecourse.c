/*******************************************************************
 *
 * rstimecourse.c
 *
 * Extracts the time course from a 4D-Nifti by supplying either a voxel or a binary mask
 *
 * Usage: rstimecourse [-m <mask> [-a <algorithm>]] [-p <X> <Y> <Z>] -input <volume>
 *
 * 
 * André Hoffmann
 *******************************************************************/


#include <stdio.h>
#include <strings.h>

#include <nifti1.h>
#include <fslio.h>
#include "rsniftiutils.h"

int show_help( void )
{
   printf(
      "rstimecourse: Given a 4D-Nifti, this tool extracts the time course\n"
      "              for a single voxel or the meaned average of a region\n"
      "              specified by a binary mask\n"
      "\n"
   );
    
   printf(
      "basic usage:  rstimecourse [-m <mask> [-a <algorithm>] [-savemask <mask>]] [-p <X> <Y> <Z>] -input <volume>\n"
      "\n"
   );
    
   printf(
      "options:\n"
      "              -a <algorithm>   : the algorithm used to aggregate the data within\n"
      "                                 a ROI, e.g. mean\n"
   );

   printf(
      "              -help            : show this help\n"
   );
 
   printf(
      "              -input <volume>  : the volume from which the timecourse will be extracted\n"
   );
   
   printf(
      "              -mask <mask>     : a mask specifying the ROI\n"
   );
   
   printf(
      "              -p <X> <Y> <Z>   : speficies a voxel using nifti coordinates(0-based) from\n"
      "                                 which the timecourse is to be extracted\n"
   );
    
   printf(
      "              -savemask <mask> : optional path where the rescaled mask specified with -mask\n"
      "                                 will be saved. The saved file with have the same dimensions\n"
      "                                 as the input volume.\n"
   );
   
   printf(
      "              -v               : show debug information\n"
      "\n"
   );
    
   return 0;
}

Point3D* ReadMask(char *path, int newX, int newY, int newZ, int *nPoints, char *resampledMaskPath, FSLIO *maskPrototype)
{    
    FSLIO *fslio;
	void *buffer;
	unsigned long buffsize;
    
    short xDim, yDim, zDim, vDim;
	short pixtype;
	size_t dt;
    float inter = 0.0, slope = 1.0;
    
    int pointCount = 0;

    /* Open mask */
    fslio = FslOpen(path, "rb");
    if (fslio == NULL) {
        fprintf(stderr, "\nError, could not read header info for %s.\n",path);
        return NULL;
    }
    
    /* Read out dimensions */
    FslGetDim(fslio, &xDim, &yDim, &zDim, &vDim);

    if (fslio->niftiptr->scl_slope != 0) {
        slope = fslio->niftiptr->scl_slope;
        inter = fslio->niftiptr->scl_inter;
    }
    
    /* Determine datatype */
	dt = FslGetDataType(fslio, &pixtype);
    
    /* Init buffer */
    buffsize = (unsigned long)xDim * (unsigned long)yDim * (unsigned long)zDim * (unsigned long)(dt/8);
    buffer   = malloc(buffsize);
    
    /* Read in first volume */
    if (!FslReadVolumes(fslio, buffer, 1)) {
        free(buffer);
        fprintf(stderr, "\nError - reading data in %s\n", path);
        FslClose(fslio);
        return NULL;
    }
    
    double ***mask = FslGetVolumeAsScaledDouble(fslio, 0);
    
    /* Resample mask to have the same scaling as the input volume */
    double ***resampledMask = ResampleVolume(mask, xDim, yDim, zDim, newX, newY, newZ);
    
    free(mask[0][0]);
    free(mask[0]);
    free(mask);
    
    /* Count how many points we'll get */
    *nPoints = 0;
    for (unsigned int x=0; x<newX; x=x+1) {
        for (unsigned int y=0; y<newY; y=y+1) {
            for (unsigned int z=0; z<newZ; z=z+1) {
                if ( resampledMask[z][y][x] > 0.01 ) {
                    *nPoints = *nPoints + 1;
                }
            }
        }
    }

    /* Initialize result array */
    Point3D* points = malloc(*nPoints*sizeof(Point3D));
    
    /* Create array with all points that are in the mask */
    int i=0;
    for (unsigned int x=0; x<newX; x=x+1) {
        for (unsigned int y=0; y<newY; y=y+1) {
            for (unsigned int z=0; z<newZ; z=z+1) {
                if ( resampledMask[z][y][x] > 0.01 ) {
                    points[i] = MakePoint3D(x,y,z);
                    i = i+1;
                }
            }
        }
    }
    
    /* Save mask */
    FSLIO *fslioResampled = NULL;
    
    if ( resampledMaskPath != NULL ) {
        fslioResampled = FslOpen(resampledMaskPath, "wb");
    }
    if (fslioResampled == NULL) {
        if ( resampledMaskPath != NULL ) {
            fprintf(stderr, "\nWarning, could not open %s for writing.\n",resampledMaskPath);
        }
    } else {
        
        dt = FslGetDataType(maskPrototype, &pixtype);
        
        FslCloneHeader(fslioResampled, maskPrototype);
        FslSetDim(fslioResampled, newX,newY,newZ,1);
        FslSetDimensionality(fslioResampled, 3);
        FslSetDataType(fslioResampled, pixtype);
        FslWriteHeader(fslioResampled);
        
        void *maskBuffer = malloc((unsigned long)newX * (unsigned long)newY * (unsigned long)newZ * (unsigned long)(dt/8));
        
        switch ( maskPrototype->niftiptr->datatype ) {
            case NIFTI_TYPE_UINT8:
                convertScaledDoubleToBuffer_UINT8(maskBuffer, resampledMask, maskPrototype->niftiptr->scl_slope, maskPrototype->niftiptr->scl_inter, newX, newY, newZ);
                break;
            case NIFTI_TYPE_INT8:
                convertScaledDoubleToBuffer_INT8(maskBuffer, resampledMask, maskPrototype->niftiptr->scl_slope, maskPrototype->niftiptr->scl_inter, newX, newY, newZ);
                break;
            case NIFTI_TYPE_UINT16:
                convertScaledDoubleToBuffer_UINT16(maskBuffer, resampledMask, maskPrototype->niftiptr->scl_slope, maskPrototype->niftiptr->scl_inter, newX, newY, newZ);
                break;
            case NIFTI_TYPE_INT16:
                convertScaledDoubleToBuffer_INT16(maskBuffer, resampledMask, maskPrototype->niftiptr->scl_slope, maskPrototype->niftiptr->scl_inter, newX, newY, newZ);
                break;
            case NIFTI_TYPE_UINT64:
                convertScaledDoubleToBuffer_UINT64(maskBuffer, resampledMask, maskPrototype->niftiptr->scl_slope, maskPrototype->niftiptr->scl_inter, newX, newY, newZ);
                break;
            case NIFTI_TYPE_INT64:
                convertScaledDoubleToBuffer_INT64(maskBuffer, resampledMask, maskPrototype->niftiptr->scl_slope, maskPrototype->niftiptr->scl_inter, newX, newY, newZ);
                break;
            case NIFTI_TYPE_UINT32:
                convertScaledDoubleToBuffer_UINT32(maskBuffer, resampledMask, maskPrototype->niftiptr->scl_slope, maskPrototype->niftiptr->scl_inter, newX, newY, newZ);
                break;
            case NIFTI_TYPE_INT32:
                convertScaledDoubleToBuffer_INT32(maskBuffer, resampledMask, maskPrototype->niftiptr->scl_slope, maskPrototype->niftiptr->scl_inter, newX, newY, newZ);
                break;
            case NIFTI_TYPE_FLOAT32:
                convertScaledDoubleToBuffer_FLOAT32(maskBuffer, resampledMask, maskPrototype->niftiptr->scl_slope, maskPrototype->niftiptr->scl_inter, newX, newY, newZ);
                break;
            case NIFTI_TYPE_FLOAT64:
                convertScaledDoubleToBuffer_FLOAT64(maskBuffer, resampledMask, maskPrototype->niftiptr->scl_slope, maskPrototype->niftiptr->scl_inter, newX, newY, newZ);
                break;
                
            case NIFTI_TYPE_FLOAT128:
            case NIFTI_TYPE_COMPLEX128:
            case NIFTI_TYPE_COMPLEX256:
            case NIFTI_TYPE_COMPLEX64:
            default:
                fprintf(stderr, "\nWarning, %s not supported yet.\n",nifti_datatype_string(fslio->niftiptr->datatype));

        }
        
        FslWriteVolumes(fslioResampled, maskBuffer, 1);
        FslClose(fslioResampled);
    }
    
    FslClose(fslio);
    free(buffer);
    free(resampledMask[0][0]);
    free(resampledMask[0]);
    free(resampledMask);
    free(fslio);
    free(fslioResampled);
    
    return points;
}

int main(int argc, char * argv[])
{
	FSLIO *fslio;
	void *buffer;
	size_t buffsize;
	
	char *inputpath = NULL;
	char *maskpath = NULL;
    char *savemaskpath = NULL;
	
	int x=-1, y=-1, z=-1, t=0;
	short xDim, yDim, zDim, vDim;
	short pixtype;
	size_t dt;
    float inter = 0.0, slope = 1.0;
    
    BOOL verbose = FALSE;
	
	int ac;

	if( argc < 2 ) return show_help();   /* typing '-help' is sooo much work */

	/* parse parameters */
	for( ac = 1; ac < argc; ac++ ) {
		if( ! strncmp(argv[ac], "-h", 2) ) {
			return show_help();
		} else if ( ! strcmp(argv[ac], "-input") ) {
			if( ++ac >= argc ) {
				fprintf(stderr, "** missing argument for -input\n");
				return 1;
			}
			inputpath = argv[ac];  /* no string copy, just pointer assignment */
		} else if ( ! strncmp(argv[ac], "-m", 2) ) {
			if( ++ac >= argc ) {
				fprintf(stderr, "** missing argument for -m\n");
				return 1;
			}
			maskpath = argv[ac];  /* no string copy, just pointer assignment */
		} else if ( ! strncmp(argv[ac], "-s", 2) ) {
			if( ++ac >= argc ) {
				fprintf(stderr, "** missing argument for -savemask\n");
				return 1;
			}
			savemaskpath = argv[ac];  /* no string copy, just pointer assignment */
		} else if ( ! strncmp(argv[ac], "-v", 2) ) {
			verbose = TRUE;
		} else if ( ! strncmp(argv[ac], "-p", 2) ) {
			if( ac+3 >= argc ) {
				fprintf(stderr, "** missing argument for -p, 3 coordinates must be supplied!\n");
				return 1;
			}
			ac++;
			x = atoi(argv[ac]);
			ac++;
			y = atoi(argv[ac]);
			ac++;
			z = atoi(argv[ac]);
		} else {
			fprintf(stderr, "\nError, unrecognized command %s\n",argv[ac]);
		}
	}
	
	if ( inputpath == NULL ) {
		fprintf(stderr, "No input volume specified!\n");
		return 1;
	}
	
	if ( maskpath == NULL && (x<0||y<0||z<0) ) {
		fprintf(stderr, "Either a binary mask or a voxel coordinate must be specified!\n");
		return 1;
	}
	
    if ( verbose ) {
        fprintf(stdout, "Input file: %s\n", inputpath);
        fprintf(stdout, "Mask file: %s\n", maskpath);
        fprintf(stdout, "Voxel: %d %d %d\n", x, y, z);
    }
	
    fslio = FslOpen(inputpath, "rb");
    if (fslio == NULL) {
        fprintf(stderr, "\nError, could not read header info for %s.\n",inputpath);
        return 1;
    }
    
	/* open nifti dataset header */
	/*fslio = FslReadHeader(inputpath);
	if (fslio == NULL) {
		fprintf(stderr, "\nError, could not read header info for %s.\n",inputpath);
		return 1;
	}*/

	/* determine dimensions */
	FslGetDim(fslio, &xDim, &yDim, &zDim, &vDim);
	
    if ( verbose ) {
        fprintf(stdout, "Dim: %d %d %d (%d Volumes)\n", xDim, yDim, zDim, vDim);
    }
    
    if (fslio->niftiptr->scl_slope != 0) {
        slope = fslio->niftiptr->scl_slope;
        inter = fslio->niftiptr->scl_inter;
    }
	
	/* determine datatype and initalize buffer */
	dt = FslGetDataType(fslio, &pixtype);
	
    if ( verbose ) {
        fprintf(stdout, "Dt: %ld Pixtype: %d\n", dt, pixtype);
    }
	
    if ( x >= 0 ) {
        /*** Extract timecourse for a single point ***/
        
        /* check inputs */
        if ( (x<0) || (x>=fslio->niftiptr->nx) ) {
            fprintf(stderr, "\nError: x index (%d) out of range [0..%d]\n",x,fslio->niftiptr->nx-1);
            FslClose(fslio);
            return 1;
        }
        if ( (y<0) || (y>=fslio->niftiptr->ny) ) {
            fprintf(stderr, "\nError: y index (%d) out of range [0..%d]\n",y,fslio->niftiptr->ny-1);
            FslClose(fslio);
            return 1;
        }
        if ( (z<0) || (z>=fslio->niftiptr->nz) ) {
            fprintf(stderr, "\nError: z index (%d) out of range [0..%d]\n",z,fslio->niftiptr->nz-1);
            FslClose(fslio);
            return 1;
        }
        
        buffsize = vDim*dt/8;
        
        if ( verbose ) {
            fprintf(stdout, "Buffsize: %ld\n", buffsize);
        }
        
        buffer = malloc(buffsize);
        
        /* read out timecourse */
        FslReadTimeSeries(fslio, buffer, x, y, z, vDim);
        
        for (t=0; t<vDim; t++) {
            void *newV = (char*)buffer + (t*dt/8);
            
            double v = 0.0;
            int ret = convertBufferToScaledDouble(&v, newV, 1, slope, inter, fslio->niftiptr->datatype);
         
            if ( verbose ) {
                fprintf(stdout, "%d: %.2f(%d)\n", t, v, ret);
            } else {
                fprintf(stdout, "%.2f\n", v);
            }
        }
        
        /* clear buffer */
        free(buffer);
    } else {
        /*** Extract timecourse for a mask ***/
        int nPoints;
        Point3D *maskPoints = ReadMask(maskpath, xDim, yDim, zDim, &nPoints, savemaskpath, fslio);
        if ( maskPoints == NULL) {
            fprintf(stderr, "\nError: Mask invalid.\n");
            return 1;
        }

        /* Iterate over all timepoints */
        for ( unsigned int t = 0; t<vDim; t=t+1 ) {
            
            /* Read in volume */
            double ***volume = FslGetVolumeAsScaledDouble(fslio, t);
            double sum = 0;
            
            /* Iterate over all points in the mask */
            for ( int i=0; i<nPoints; i=i+1) {
                Point3D p = maskPoints[i];                
                sum = sum + volume[p.z][p.y][p.x];
            }
            
            /* Create average */
            double avg = sum / nPoints;
            fprintf(stdout, "%.10f\n", avg);
            
            free(volume[0][0]);
            free(volume[0]);
            free(volume);
        }
        
        free(maskPoints);
    }
    
    FslClose(fslio);
    free(fslio);
    
	return 0;
}
