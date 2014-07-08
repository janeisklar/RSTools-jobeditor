#include "rsmotionscrubbing_common.h"

extern double deg2mm(const double dist, const double deg);
extern int rsMax(const int a, const int b);
extern int rsMin(const int a, const int b);


void rsMotionScrubbingInit(rsMotionScrubbingParameters *p)
{
    p->parametersValid = FALSE;
    
    /* check if the required arguments have been provided */
    if ( p->inputpath == NULL ) {
        fprintf(stderr, "No input volume specified!\n");
        return;
    }
    
    if ( p->outputpath == NULL ) {
        fprintf(stderr, "No output volume specified!\n");
        return;
    }
    
    if ( p->maskpath == NULL ) {
        fprintf(stderr, "A binary mask must be specified!\n");
        return;
    }
    
    if ( p->realignmentpath == NULL ) {
        fprintf(stderr, "A realignment parameter file must be specified!\n");
        return;
    }
    
    rsSetThreadsNum(p->threads);
    
    /* open input file */
    p->input = rsOpenNiftiFile(p->inputpath, RSNIFTI_OPEN_READ);

    if ( ! p->input->readable ) {
        fprintf(stderr, "\nError: The nifti file that was supplied as an input (%s) could not be read.\n", p->inputpath);
        return;
    }
    
    /* output the most important parameters to the user */
    if ( p->verbose ) {
        fprintf(stdout, "Input file:  %s\n", p->inputpath);
        fprintf(stdout, "Mask file:   %s\n", p->maskpath);
        fprintf(stdout, "Output file: %s\n", p->outputpath);
        fprintf(stdout, "DVARS threshold: %.4f\n", p->dvarsthreshold);
        fprintf(stdout, "Framewise displacement threshold: %.4f\n", p->fdthreshold);
        fprintf(stdout, "Realignment parameters file: %s\n", p->realignmentpath);
        
        if ( p->dvarspath != NULL ) 
            fprintf(stdout, "DVARS file: %s\n", p->dvarspath);
        
        if ( p->fdpath != NULL ) 
            fprintf(stdout, "Framewise displacement file: %s\n", p->fdpath);
        
        if ( p->flaggedpath != NULL ) 
            fprintf(stdout, "Flagged frames file: %s\n", p->flaggedpath);
            
        if ( p->verbose )
            fprintf(stdout, "Dim: %d %d %d (%d Volumes)\n", p->input->xDim, p->input->yDim, p->input->zDim, p->input->vDim);
    }
    
    /* load realignment parameters */
    p->rp = rsLoadRegressors(p->realignmentpath, &p->rpColumns, &p->rpEntries, 1.0);
    
    /* load mask */
    p->nMaskPoints = 0L;
    double ***mask = d3matrix(p->input->zDim-1, p->input->yDim-1, p->input->xDim-1);
    p->maskPoints = rsReadMask(p->maskpath, p->input->xDim, p->input->yDim, p->input->zDim, &p->nMaskPoints, NULL, p->input->fslio, p->mask);
    if ( p->maskPoints == NULL) {
        fprintf(stderr, "\nError: Mask invalid.\n");
        return;
    }
    free(mask[0][0]); free(mask[0]); free(mask);

    p->parametersValid = TRUE;
}

void rsMotionScrubbingRun(rsMotionScrubbingParameters *p)
{
    p->parametersValid = FALSE;
    
    // compute value range
    double min,max;
    rsComputeValueRange(p->input, p->maskPoints, p->nMaskPoints, &min, &max);
    
    if ( p->verbose ) {
        fprintf(stdout, "Value range [%.2f;%.2f]\n", min, max);
    }
    
    double fd[p->rpEntries];
    
    // compute framewise displacement
    rsComputeFramewiseDisplacement(&fd[0], &p->rp[0], p->rpEntries);
    
    // compute dvars
    double dvars[p->input->vDim];
    rsComputeDVARs(&dvars[0], min, max, p->input, p->maskPoints, p->nMaskPoints);
    
    // save dvars if requested
    if ( p->dvarspath != NULL ) {
        rsSaveDoubleVector(p->dvarspath, &dvars[0], p->input->vDim);
    }
    
    // save framewise displacement if requested
    if ( p->fdpath != NULL ) {
        rsSaveDoubleVector(p->fdpath, &fd[0], p->input->vDim);
    }
    
    // mark all frames as unflagged
    BOOL flaggedFrames[p->input->vDim];
    
    for ( int t=0; t<p->input->vDim; t=t+1 ) {
        flaggedFrames[t] = FALSE;
    }
    
    // determine which frames need to be flagged
    for ( int t=1; t<p->input->vDim; t=t+1 ) {
        if ( fd[t] > p->fdthreshold || dvars[t] > p->dvarsthreshold ) {
            flaggedFrames[rsMax(t-1,0)]                = TRUE;
            flaggedFrames[t]                           = TRUE;
            flaggedFrames[rsMin(t+1,p->input->vDim-1)] = TRUE;
        }
    }
    
    if ( p->flaggedpath != NULL ) {
        rsSaveIndexVector(p->flaggedpath, &flaggedFrames[0], p->input->vDim);
    }
    
    // count remaining frames
    int remainingFrames = 0;
    int framemap[p->input->vDim];

    for ( int t=0; t<p->input->vDim; t=t+1 ) {
        if ( flaggedFrames[t] == FALSE ) {
            framemap[remainingFrames] = t;
            remainingFrames = remainingFrames + 1;
        }
    }

    // create output file
    p->output = rsCloneNiftiFile(p->outputpath, p->input, RSNIFTI_OPEN_ALLOC, remainingFrames);
    
    if ( ! p->output->readable ) {
        fprintf(stderr, "\nError: The nifti file containing the scrubbed output (%s) could not be created.\n", p->outputpath);
        return;
    }
    
    rsWriteNiftiHeader(p->output->fslio, p->callString);

    // prepare the output file's content
    int t;
    #pragma omp parallel num_threads(rsGetThreadsNum()) private(t) shared(flaggedFrames)
    {
        #pragma omp for schedule(guided, 1)
        for ( t=0; t<remainingFrames; t++ ) {
                    
            // extract a single volume for timepoint t from the buffer
            double ***data = d3matrix(p->input->zDim-1, p->input->yDim-1, p->input->xDim-1);
            rsExtractVolumeFromRSNiftiFileBuffer(p->input, data[0][0], framemap[t]);

            // write back to the buffer
            rsWriteVolumeToBuffer(p->output->dt, data[0][0], p->output->data, p->output->slope, p->output->inter, t, p->output->xDim, p->output->yDim, p->output->zDim);

            // free up memory
            free(data[0][0]); free(data[0]); free(data);
        }
    }

    // write scrubbed file
    FslWriteVolumes(p->output->fslio, p->output->data, remainingFrames);

    p->parametersValid = TRUE;
}

void rsMotionScrubbingDestroy(rsMotionScrubbingParameters *p)
{
    if ( p->input != NULL ) {
        rsCloseNiftiFileAndFree(p->input);
        p->input = NULL;
    }
    
    if ( p->output != NULL ) {
        p->output->data = NULL;
        rsCloseNiftiFileAndFree(p->output);
        p->output = NULL;
    }
    
    if ( p->maskpath != NULL ) {
        free(p->mask);
        p->mask = NULL;
    }

    rsMotionScrubbingFreeParams(p);
}

/*
 * Iterates over a 4D nifti and determines the highest and lowest intensity values
 */
void rsComputeValueRange(rsNiftiFile *file, Point3D *maskPoints, unsigned long nMaskPoints, double *min, double *max)
{
    *min=9999999999999999;
    *max=-9999999999999999;
    
    double ***data = d3matrix(file->zDim-1,  file->yDim-1, file->xDim-1);
    
    for (int t=rsMin(5, file->vDim-1); t<file->vDim; t=t+1) {
        
        rsExtractVolumeFromRSNiftiFileBuffer(file, data[0][0], t);
        
        for (long p = 0L; p<nMaskPoints; p=p+1L) {
            const Point3D* point  = &maskPoints[p];
            const double intensity = data[point->z][point->y][point->x];
            
            if ( intensity != intensity ) {
                continue;
            }
            
            if ( *min > intensity ) {
                *min = intensity;
            }
            
            if ( *max < intensity ) {
                *max = intensity;
            }
        }
    }
    
    free(data[0][0]); 
    free(data[0]);
    free(data);
}

/*
 * Compute framewise displacement values as defined in:
 * Power, Jonathan D., et al. "Spurious but systematic correlations in functional connectivity MRI networks arise from subject motion." Neuroimage 59.3 (2012): 2142-2154. APA
 */
void rsComputeFramewiseDisplacement(double *fd, double **rp, int length)
{
    fd[0] = 0;
    for ( int i=1; i<length; i=i+1 ) {
        const double dx     = fabs(rp[1][i]-rp[1][i-1]);
        const double dy     = fabs(rp[2][i]-rp[2][i-1]);
        const double dz     = fabs(rp[3][i]-rp[3][i-1]);
        const double dpitch = fabs(deg2mm(50, rp[4][i])-deg2mm(50, rp[4][i-1]));
        const double droll  = fabs(deg2mm(50, rp[5][i])-deg2mm(50, rp[5][i-1]));
        const double dyaw   = fabs(deg2mm(50, rp[6][i])-deg2mm(50, rp[6][i-1]));
        fd[i] = dx + dy + dz + dpitch + droll + dyaw;
    }
}

/*
 * Compute DVARs as defined in:
 * Power, Jonathan D., et al. "Spurious but systematic correlations in functional connectivity MRI networks arise from subject motion." Neuroimage 59.3 (2012): 2142-2154. APA
 */
void rsComputeDVARs(double *dvars, const double min, const double max, const rsNiftiFile *file, Point3D *maskPoints, unsigned long nMaskPoints)
{
    dvars[0]=0;
    int t;
    #pragma omp parallel num_threads(rsGetThreadsNum()) private(t) shared(dvars)
    {
        #pragma omp for schedule(guided, 1)
        for (t=1; t<file->vDim; t++) {

            /* Extract a single volume for timepoint t from the buffer */
            double ***dataNow    = d3matrix(file->zDim-1,  file->yDim-1, file->xDim-1);
            double ***dataBefore = d3matrix(file->zDim-1,  file->yDim-1, file->xDim-1);

            rsExtractVolumeFromBuffer(file->dt,    dataNow[0][0], file->data, file->slope, file->inter, t,   file->xDim, file->yDim, file->zDim);
            rsExtractVolumeFromBuffer(file->dt, dataBefore[0][0], file->data, file->slope, file->inter, t-1, file->xDim, file->yDim, file->zDim);
            
            dvars[t]=0;
            
            for (long p = 0L; p<nMaskPoints; p=p+1L) {
                const Point3D* point  = &maskPoints[p];
                const double INow    = (dataNow[point->z][point->y][point->x]    - min)/fabs(max-min);
                const double IBefore = (dataBefore[point->z][point->y][point->x] - min)/fabs(max-min);
                
                dvars[t]=dvars[t] + pow(INow - IBefore, 2.0);
            }
    
            dvars[t]=sqrt(dvars[t] / (double)nMaskPoints);
    
            /* Free up memory */
            free(dataNow[0][0]);    free(dataNow[0]);    free(dataNow);
            free(dataBefore[0][0]); free(dataBefore[0]); free(dataBefore);
        }
    }
}

/*
 * Save a vector of doubles to a txt-file
 */
void rsSaveDoubleVector(char *path, double *vector, const int length)
{
    FILE *file; 
    file = fopen(path,"w");
    
    for ( int i=0; i<length; i=i+1 ) {
        fprintf(file, "%.10f\n", vector[i]);
    }
    
    fclose(file);
}

/*
 * Save a vector of indices to a txt-file
 * only the indices of the elements of the boolean vector that contain the value TRUE will be stored
 */
void rsSaveIndexVector(char *path, BOOL *vector, const int length)
{
    FILE *file; 
    file = fopen(path,"w");
    
    for ( int i=0; i<length; i=i+1 ) {
        if ( vector[i] == TRUE ) {
            fprintf(file, "%d\n", i);
        }
    }
    
    fclose(file);
}
