// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <iostream>
#include <string>

/// Functions to handle UNIX-like directory paths
namespace FilePath
{
    /// true if 'path' is an existing directory
    bool        is_dir(const char path[]);

    /// return path to current working directory
    std::string get_dir();

    /// change current working directory
    int         change_dir(std::string const& path);

    /// extract the directory part from the given path
    std::string dir_part(std::string const& path);

    /// extract the file part from the given path
    std::string file_part(std::string const& path);

    /// complete the file name using the given directory
    std::string full_name(std::string const& dir, std::string const& file);

}


