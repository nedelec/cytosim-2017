// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
///F. Nedelec, EMBL, October 2006. nedelec@embl.de

#ifndef  FILEWRAPPER_H
#define  FILEWRAPPER_H

/**
 The keyword _FILE_OFFSET_BITS affects the code in <cstdio> etc.
 Defining this keywords allow us to use 64bits fpos_t, 
 and thus to support files above 2GB
 */
#define _FILE_OFFSET_BITS  64


#include "assert_macro.h"
#include <cstdio>
#include <cstdarg>
#include <sys/types.h>
#include <string>


/// A wrapper around a C-FILE
/**
 The FileWrapper has a cast-operator to FILE*,
 and it can thus be used directly in the functions of the C-library.
 */
class FileWrapper
{    
protected:
    
    /// the C-file descriptor
    FILE*       mFile;
    
    /// the name of the file or some other information:
    std::string mPath;
    
public:
    
    /// constructor - no file
    explicit FileWrapper();
    
    /// constructor which opens a file
    FileWrapper(FILE* , const char * path = 0);
    
    /// constructor which opens a file
    FileWrapper(const char* name, const char* mode);

    /// destructor 
    virtual ~FileWrapper();
    
    /// constructor from an already opened file
    void operator =(FILE *);
    
    /// automatic conversion to a FILE *
    operator FILE*()                     { return mFile; }
    
    /// open a file
    int     open(const char* name, const char* mode);
    
    /// rewind file
    void    rewind()                     { if ( mFile ) std::rewind(mFile); }

    /// rewind file
    void    clearerr()                   { if ( mFile ) std::clearerr(mFile); }

    /// close file
    void    close();
    
    /// return the file pointer
    FILE*   file()                       { return mFile; }
    
    /// the path of the file, or of the last attempt to open a file
    const char * path()          const   { return mPath.c_str(); }
    
    /// true if output goes to stdout
    bool    std()               const    { return mFile==stdout; }
    
    /// true if End-Of-File
    bool    eof()               const    { return mFile && std::feof(mFile); }
    
    /// return the value of ferror()
    int     error()             const    { return std::ferror(mFile); }

    /// true if file is good for writing / reading
    bool    good()              const    { return mFile && !std::ferror(mFile); }

    
    /// current position in input file, relative to beggining
    int     get_pos(fpos_t& p)  const    { return fgetpos(mFile, &p); }

    /// set position relative to the beginning of the file
    void    set_pos(const fpos_t& p)     { fsetpos(mFile, &p); }
    
    
    /// put string
    void    put_line(const char * line, char sep=0);

    /// put string
    void    put_line(const std::string& line, char sep=0) { put_line(line.c_str(), sep); }

    /// read line, including the terminating '\n'; return the position where the line started
    void    get_line(std::string& line, char sep='\n');
    
    /// read stream until given string is found
    void    skip_until(const char * str);
    
    ///lock file by the current thread
    void    lock()                       { flockfile(mFile); }
    
    ///unlock file
    void    unlock()                     { funlockfile(mFile); }
    
    /// read a character
    int     getUL()                      { return getc_unlocked(mFile); }

    /// report next character to be read
    int     peek()                       { int c=getc_unlocked(mFile); if ( c != EOF ) ungetc(c, mFile); return c; }

    /// report next character to be read
    void    unget(int c)                 { ungetc(c, mFile); }

    /// write a character
    int     write(const char c)          { return putc_unlocked(c, mFile); }
    
    /// flush
    void    flush()                      { std::fflush(mFile); }

};

#endif

