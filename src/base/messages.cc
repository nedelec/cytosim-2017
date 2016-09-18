// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "messages.h"

//the global instantiation Messages used for input/output
Messages MSG;

#include "assert_macro.h"
#include <cstdlib>
#include <cctype>

//------------------------------------------------------------------------------
/**
 default verbose level = 4
 */
Messages::Messages() : FileWrapper(stdout)
{
    mVerbose = 4;
    nWarnings = 0;
}

//------------------------------------------------------------------------------
void Messages::operator()(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(mFile, fmt, args);
    va_end(args);
}


//------------------------------------------------------------------------------
void Messages::operator()(int level, const char* fmt, ...)
{
    if ( mVerbose >= level )
    {
        va_list args;
        va_start(args, fmt);
        vfprintf(mFile, fmt, args);
        va_end(args);
    }
}


//------------------------------------------------------------------------------
void Messages::warning(const char* fmt, ...)
{    
    if ( mVerbose >= 0  &&  nWarnings < maxWarnings )
    {
        fprintf(mFile, "warning: ");
        va_list args;
        va_start(args, fmt);
        vfprintf(mFile, fmt, args);
        va_end(args);
        
        if (++nWarnings >= maxWarnings)
            fprintf(mFile, "warning messages are now silent\n");
    }
}

