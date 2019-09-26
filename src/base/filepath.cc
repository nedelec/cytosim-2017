// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "filepath.h"
#include <sys/param.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include "exceptions.h"

//------------------------------------------------------------------------------

std::string FilePath::get_dir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    return cwd;
}


bool FilePath::is_dir(const char path[])
{
    struct stat s;
    if ( 0 == stat(path, &s) )
        return S_ISDIR(s.st_mode);
    return false;
}

int FilePath::change_dir(std::string const& wd)
{
    if ( wd == "." )
        return 0;
    return chdir(wd.c_str());
}


std::string FilePath::dir_part(std::string const& path)
{
    char* res, tmp[PATH_MAX];
    
    if ( 0 == realpath(path.c_str(), tmp) )
        return ".";
    
    res = dirname(tmp);
    
    if ( 0 == res )
        throw InvalidIO("FilePath: stdlib::dirname() failed");
    
    return res;
}


std::string FilePath::file_part(std::string const& path)
{
    char * res = basename(const_cast<char*>(path.c_str()));
    
    if ( 0 == res )
        throw InvalidIO("FilePath: stdlib::basename() failed");
    
    return res;
}


std::string FilePath::full_name(std::string const& dir, std::string const& file)
{
    //if a full path is already specified, we do nothing
    if ( dir.size() > 0  &&  file.size() > 0  &&  file[0] != '/' )
    {
        std::string res = dir + file;
        
        //remove trailling '/' if present
        const long x = res.size() - 1;
        if ( 0 <= x  &&  res[x] == '/' )
            res[x] = '\0';
        
        return res;
    }
    return file;
}

