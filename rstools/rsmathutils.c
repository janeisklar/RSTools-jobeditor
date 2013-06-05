/*******************************************************************
 *
 * rsmathutils.c
 *
 * 
 * André Hoffmann
 *******************************************************************/

#include "rsmathutils.h"

void rsLinearRegression(int nSamples, double *signal, int nRegressors, double **regressors, double *betas, double *residuals, double *fitted, int verbose)
{
    gsl_multifit_linear_workspace *work = gsl_multifit_linear_alloc(nSamples, nRegressors);
    
    // convert inputs to gsl variables
    gsl_vector *y   = gsl_vector_alloc(nSamples);                 // input/signal
    gsl_matrix *X   = gsl_matrix_alloc(nSamples, nRegressors);    // regressors
    gsl_vector *b   = gsl_vector_alloc(nRegressors);              // betas
    gsl_matrix *cov = gsl_matrix_alloc(nRegressors, nRegressors); // covariance
    gsl_vector *res = gsl_vector_alloc(nSamples);                 // residuals
    
    // fill them
    for (int i=0; i < nSamples; i++){
        gsl_vector_set(y,i,signal[i]);
    }
    
    for (int r=0; r<nRegressors; r++) {
        for (int t=0; t<nSamples; t++) {
            gsl_matrix_set(X,t,r,regressors[r][t]);
        }
    }
    
    // execute linear regression
    double chisq;
    int success = gsl_multifit_linear(X, y, b, cov, &chisq, work);
    
    // compute residuals
    gsl_multifit_linear_residuals(X, y, b, res);
    
    // convert back to basic C variables
    for (int t=0; t < nSamples; t++) {
        residuals[t] = gsl_vector_get(res, t);
    }
    
    for (int r=0; r < nRegressors; r++) {
        betas[r] = gsl_vector_get(b, r);
    }
    
    gsl_matrix_free(X);
    gsl_matrix_free(cov);
    gsl_vector_free(y);
    gsl_vector_free(b);
    gsl_vector_free(res);
    gsl_multifit_linear_free(work);
}

void rsFFTFilter(double *data, const int T, const double sampling_rate, const double f1, const double f2, const int verbose) {
    
    /* Compute the frequency of the spectral bins */
    double F[T];
    for (int i=0; i<T; i=i+1) {
        F[i] = 0.0;
        
        if ( i > 0 ) {
            i      = i+1; // skip the bin holding the real part
            F[i]   = i/(2 * T * sampling_rate); // set the frequency of the complex part's bin
            F[i-1] = F[i]; // copy it to the real part's bin
        }
        
        //printf("%d: %.10f\n", (i>0 ? i-1 : 0), F[i]);
    }
    
    /* Compute which bins to keep */
    int i1=-1, i2=-1;
    for (int i=1; i<=T; i=i+2) {
        if (F[i] > f1) {
            i1 = (i-2 < 0) ? 0 : i-2;
            break;
        }
    }
    for (int i=1; i<=T; i=i+2) {
        if (F[i] > f2) {
            i2 = (i+1 >= T) ? T-1 : i+1;
            break;
        }
    }
    if ( i1 < 0 ) i1 = (T % 2==0) ? T-1 : T-2; /* in the even case the complex part */
    if ( i2 < 0 ) i2 = (T % 2==0) ? T-1 : T-2; /* of the last bin is not stored     */
    
    if (verbose) printf("Bandpass range: %.4fHz..%.4fHz\n", F[i1], F[i2]);
    
    /* Prepare FFT Filtering */
    gsl_fft_real_wavetable        *real;
    gsl_fft_halfcomplex_wavetable *hc;
    gsl_fft_real_workspace        *work;
    
    work = gsl_fft_real_workspace_alloc(T);
    real = gsl_fft_real_wavetable_alloc(T);
    hc   = gsl_fft_halfcomplex_wavetable_alloc(T);
    
    /* FFT */
    gsl_fft_real_transform(data, 1, T, real, work);
    
    /* Remove undesired frequency bins */
    for (int i = 0; i<T; i=i+1) {
        if ( i < i1 || i > i2 ) {
            data[i] = 0;
        }
    }
    
    /* Inverse FFT */
    gsl_fft_halfcomplex_inverse(data, 1, T, hc, work);
    
    //for (int i=0; i<T; i=i+1) {
    //    printf("%03d: %.2f\n", i+1, data[i]);
    //    printf("%.10f\n", data[i]);
    //}
    
    /* Free memory */
    gsl_fft_real_wavetable_free(real);
    gsl_fft_halfcomplex_wavetable_free(hc);
    gsl_fft_real_workspace_free(work);
}

double **d2matrix(int yh, int xh)
{
    int j;
    int nrow = yh+1;
    int ncol = xh+1;
    double **t;
    
    
    /** allocate pointers to ydim */
    t=(double **) malloc((size_t)((nrow)*sizeof(double*)));
    
    /** allocate pointers for zdim */
    t[0]=(double *) malloc((size_t)((ncol*nrow)*sizeof(double)));
    
    /** point everything to the data blob */
    for(j=1;j<nrow;j++) t[j]=t[j-1]+ncol;
    
    return t;
}