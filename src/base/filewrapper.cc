// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "filewrapper.h"
#include "exceptions.h"
#include <sys/param.h>
#include <libgen.h>


//------------------------------------------------------------------------------
FileWrapper::FileWrapper()
{
    mFile = 0;
}


FileWrapper::FileWrapper(FILE * f, const char *path)
{
    mFile = f;
    if ( path )
        mPath = path;
}


FileWrapper::FileWrapper(const char* name, const char* mode)
{
    mFile = 0;
    open(name, mode);
}


FileWrapper::~FileWrapper()
{
    close();
}


//------------------------------------------------------------------------------
#pragma mark -

void FileWrapper::operator =(FILE * f)
{
    close();
    mFile = f;
}


int FileWrapper::open(const char* name, const char* mode)
{
    if ( name[0] == 0 )
        throw InvalidIO("an empty mFile name was specified");

    if ( mode[0] != 'r' && mode[0] != 'w' && mode[0] != 'a' )
        throw InvalidIO("invalid mFile opening mode");

    if ( mFile )
        close();
    
    /// remember the path
    mPath = name;
    
    mFile = fopen(name, mode);

    if ( mFile == 0 )
    {
        if ( mode[0] == 'w'  ||  mode[0] == 'a' )
            throw InvalidIO("output mFile `"+std::string(name)+"' could not be opened");
        return 1;
    }
    
    if ( ferror(mFile) )
    {
        fclose(mFile);
        mFile = 0;
        throw InvalidIO("input mFile `"+std::string(name)+"'  opened with errors");
    }
    
    return 0;
}


void FileWrapper::close()
{
    if ( mFile )
    {
        fflush(mFile);
        
        if ( mFile!=stdout  &&  mFile!=stderr )
        {
            if ( fclose(mFile) )
                throw InvalidIO("failed to close input mFile: fclose() is true");
        }
        mFile = 0;
    }
}

//------------------------------------------------------------------------------
#pragma mark -


/**
 This will write the line to output stream.
 If it is non-zero, 'sep' is also written out.
 */
void FileWrapper::put_line(const char * str, char sep)
{
    size_t s = strlen(str);
    
    char * m = (char*)memchr(str, sep, s);
    
    if ( m )
        fwrite(str, 1, m-str, mFile);
    else
        fwrite(str, 1, s, mFile);
    
    if ( sep )
        putc(sep, mFile);
}



/**
 @returns the starting position of the line
 */
void FileWrapper::get_line(std::string& line, const char sep)
{
    line.clear();

    if ( ferror(mFile) )
        return;

    const size_t CHK = 32;
    char str[CHK];
    
    fpos_t pos;
    char * m;
    
    while ( !feof(mFile) )
    {
        fgetpos(mFile, &pos);
        size_t s = fread(str, 1, CHK, mFile);
        
        m = (char*)memchr(str, sep, s);
        
        if ( m )
        {
            line.append(str, m-str);
#ifdef __APPLE__
            pos += m-str+1;
            fsetpos(mFile, &pos);
#else
            fsetpos(mFile, &pos);
            fread(str, 1, m-str+1, mFile);
#endif
            //fprintf(stderr,"-|%s|-\n", line.c_str());
            return;
        }
        line.append(str, s);
    }
    
    return;
}


/**
 This will search for the string and position the stream
 at the first character of the match.
 If the `str` is not found, the stream will be positionned
 at the end of the file, with a eof() state.
 
 The search might fail if `str` contains repeated sequences
 */
void FileWrapper::skip_until(const char * str)
{
    const size_t CHK = 64;
    char buf[CHK+2];

    fpos_t pos, match;
    size_t offset = 0;
    
    const char sss = str[0];
    const char * s = str;
    const char * b;

    while ( !eof() )
    {
        fgetpos(mFile, &pos);
        size_t nbuf = fread(buf, 1, CHK, mFile);
        
        if ( s == str )
        {
            b = (char*)memchr(buf, sss, nbuf);
            if ( b == 0 )
                continue;
            match  = pos;
            offset = b - buf;
            ++s;
            ++b;
        }
        else
            b = buf;
        
        char *const end = buf + nbuf;
        
        while ( b < end )
        {
            //assert_true( s != str );
            if ( *b == *s )
            {
                ++s;
                if ( *s == 0 )
                {
#ifdef __APPLE__
                    match += offset;
                    fsetpos(mFile, &match);
#else
                    fsetpos(mFile, &match);
                    fread(buf, 1, offset, mFile);
#endif
                    return;
                }
            }
            else
            {
                s = str;
                b = (char*)memchr(b, sss, nbuf-(b-buf));
                if ( b == 0 )
                    break;
                match  = pos;
                offset = b - buf;
                ++s;
            }
            ++b;
        }
    }
}

