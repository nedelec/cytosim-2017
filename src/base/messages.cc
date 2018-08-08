// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "messages.h"
#include <cstdio>

namespace Cytosim
{
    FILE * mFile = stdout;
    
    /// verbose level
    int mVerbose = 4;
    
    /// number  of warning already issued
    int nWarnings = 0;
    
    ///max. number of output-warnings
    static const int maxWarnings = 50;

    void MSG(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        vfprintf(mFile, fmt, args);
        va_end(args);
    }
    
    void MSG(int v, const char* fmt, ...)
    {
        if ( mVerbose >= v )
        {
            va_list args;
            va_start(args, fmt);
            vfprintf(mFile, fmt, args);
            va_end(args);
        }
    }

    void warning(const char* fmt, ...)
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
    
    void open(char const* name)
    {
        if ( mFile != stdout )
            fclose(mFile);
        mFile = fopen(name, "w");
        if ( mFile && ferror(mFile) )
        {
            fclose(mFile);
            mFile = 0;
        }
        if ( mFile == 0 )
            mFile = stdout;
    }
    
    void close()
    {
        if ( mFile != stdout )
            fclose(mFile);
        mFile = stdout;
    }

    void flush()
    {
        fflush(mFile);
    }
}
