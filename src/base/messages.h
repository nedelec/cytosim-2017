// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef  MESSAGES_H
#define  MESSAGES_H

#include <cstdarg>


/// This facility provides some control over output
/** F. Nedelec, 17.03.2018 */
namespace Cytosim
{
    
    ///verbose level, PRINT has a option specifying a level which is compared to mVerbose
    extern int mVerbose;
    
    /// suppress all output by setting Verbose to -1
    inline void silent()          { mVerbose = -1; }

    ///suppresses most output by setting Verbose to 0
    inline void quiet()           { mVerbose = 0; }
    
    /// set verbose to level m
    inline void setVerbose(int m) { mVerbose = m; }
        
    ///convenient access to print() with the () operator
    void    MSG(const char* fmt, ...);
    
    ///convenient access to print() with the () operator
    void    MSG(int, const char* fmt, ...);
    
    ///warning() is equivalent to print() with "Warning:" in front
    void    warning(const char* fmt, ...);

    /// open file for output
    void    open(char const* filename);
    
    /// close output file
    void    close();

    /// flush file
    void    flush();
};


/// a macro to print a message only once
#define MSG_ONCE(a) { static bool virgin=true; if (virgin) { virgin=false; Cytosim::MSG(a); } }

#endif
