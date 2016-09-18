// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef  MESSAGES_H
#define  MESSAGES_H

#include <cstdio>
#include <cstdarg>
#include "filewrapper.h"


/// Control output file with different levels of verbosity
/**
 @todo build Messages from a OutputStream
 */
class Messages : public FileWrapper
{
        
private:
    
    ///verbose level, PRINT has a option specifying a level which is compared to mVerbose
    int mVerbose;
    
    ///max. number of output-warnings
    static const int maxWarnings = 50;
    
    int nWarnings;

public:
        
    ///Constructor
    Messages();
    
    ///Destructor 
    virtual ~Messages() {}
    
    /// suppress all output by setting Verbose to -1
    void    silent()               { mVerbose = -1; }

    ///suppresses most output by setting Verbose to 0
    void    quiet()                { mVerbose = 0; }

    /// return the verbose level
    int     verboseLevel()         { return mVerbose; }
    
    /// set verbose to level m
    void    setVerbose(int m)      { mVerbose = m; }
        
    ///convenient access to print() with the () operator
    void    operator()(const char* fmt, ...);

    ///convenient access to print() with the () operator, with a verbose level
    void    operator()(int, const char* fmt, ...);
    
    ///warning() is equivalent to print() with "Warning:" in front
    void    warning(const char* fmt, ...);
};

/// global instantiation used for Cytosim output
/** C++ makes no guaranty about the order in which global variables are initialized.
Therefore an error may occur if another constructor uses MSG before it is constructed */
extern Messages MSG;


/// a macro to print a message only once
#define MSG_ONCE(a) { static bool virgin=true; if (virgin) { virgin=false; MSG(a); } }

#endif
