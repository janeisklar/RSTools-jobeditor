#ifndef rstools_rstimecourse_ui_h
#define rstools_rstimecourse_ui_h

#include <stdio.h>
#include <strings.h>
#include <nifti1.h>
#include <fslio.h>
#include <glib.h>
#include "src/nifti/rsniftiutils.h"
#include "src/maths/rsmathutils.h"

#define RSTOOLS_TIMECOURSE_ALGORITHM_MEAN   1
#define RSTOOLS_TIMECOURSE_ALGORITHM_SPCA   2
#define RSTOOLS_TIMECOURSE_ALGORITHM_TPCA   3
#define RSTOOLS_TIMECOURSE_ALGORITHM_CSP    4
#define RSTOOLS_TIMECOURSE_ALGORITHM_STDDEV 5

typedef struct {
	
	char *inputpath;
	char *maskpath;
	char *mask2path;
    char *savemaskpath;
	char *eigenvaluespath;
	char *spatialmappath;
	
	short algorithm;
	
	BOOL useStandardScores;
	
	float minVariance;
	int nComponents;
    
	rsNiftiFile *input;
	rsMask *mask;
	rsMask *mask2;
	Point3D *point;
    
	BOOL parametersValid;
	BOOL verbose;
	GOptionContext *context;
	char *callString;
    int threads;

} rsTimecourseParameters;

rsTimecourseParameters* rsTimecourseParseParams(int argc, char * argv[]);
rsTimecourseParameters* rsTimecourseInitParameters();
void rsTimecourseFreeParams(rsTimecourseParameters* p);
void rsTimecoursePrintHelp(rsTimecourseParameters* p);

gboolean rsTimecourseParseAlgorithm(const gchar *option_name, const gchar *value, gpointer data, GError **error);
gboolean rsTimecourseParsePoint(const gchar *option_name, const gchar *value, gpointer data, GError **error);
gboolean rsCorrelationParseMonteCarloParams(const gchar *option_name, const gchar *value, gpointer data, GError **error);
#endif