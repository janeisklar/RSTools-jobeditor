#include "rstimecourse_common.h"
#include "rstimecourse_ui.h"

int main(int argc, char * argv[]) {

    // Parse run arguments
    rsTimecourseParameters *p = rsTimecourseParseParams(argc, argv);
    
    if( argc < 2 ) {
        rsTimecoursePrintHelp(p);
        return 0;
    }
    
    // If arguments are valid, initialize niftis, etc.
    if ( p->parametersValid ) {
        rsTimecourseInit(p);
    }
    
    // If everything went well start the main processing
    if ( p->parametersValid ) {
        rsTimecourseRun(p);
    }

    // Free memory
    rsTimecourseDestroy(p);

    return 0;
}
