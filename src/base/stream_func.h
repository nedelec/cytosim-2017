// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <iostream>

/// Simple operations on C++ streams
namespace StreamFunc
{
    
    /// remove non-conventional characters
    void clean_stream(std::ostream &, std::istream&);
    
    /// export lines of `val` that are not identical to `ref`
    void diff_stream(std::ostream &, std::istream& val, std::istream& ref);
    
    
    /// copy lines that do not start with character `skip`
    void skip_lines(std::ostream &, std::istream&, char skip);

    /// add `insert` before every line, but skip lines that start with `skip`
    void prefix_lines(std::ostream &, std::istream&, std::string const& prefix, char keep, char skip);

    
    /// print the line of `istream` corresponding to position `pos`, with line number
    void show_line(std::ostream &, std::istream &, std::streampos pos);
    
    /// same as `show_line()`, but output is returned as a string
    std::string show_line(std::istream &, std::streampos);
    
    
    /// extract the lines located between `start` and `end`, with line numbers
    void print_lines(std::ostream &, std::istream &, std::streampos start, std::streampos end);

    /// extract the lines located between `start` and `end`, with line numbers
    void show_lines(std::ostream &, std::istream &, std::streampos start, std::streampos end);
    
    /// same as show_lines(), but output is returned as a string
    std::string get_lines(std::istream &, std::streampos start, std::streampos end);
    
    
    /// replace in `src` all occurences of `fnd` by `rep`
    int  find_and_replace(std::string& src, std::string const& fnd, std::string const& rep);
}

