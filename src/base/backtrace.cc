// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "backtrace.h"


#ifdef __APPLE__


#include <stdlib.h>
#include <execinfo.h>

/**
 * print the current call-stack using functions from the GNU C Library:
 * - backtrace()
 * - backtrace_symbols()
 * .
 * provided by <execinfo.h>
 */

void print_backtrace(FILE * out)
{
    void* callstack[128];
    size_t frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);

    fprintf(out, "Execution stack:\n");
    for ( size_t ii = 0; ii < frames; ++ii )
        fprintf(out, "    %s\n", strs[ii]);
    
    free(strs);
}

#else

void print_backtrace(FILE *)
{
    //fprintf(file, "Execution stack not available\n");
}

#endif

