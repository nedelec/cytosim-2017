// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "stream_func.h"
#include <sstream>
#include <iomanip>
#include <cctype>


void StreamFunc::clean_stream(std::ostream & os, std::istream& is)
{
    char c;
    while ( is.good() )
    {
        is.get(c);
        
        // terminate the line for new-line and cariage-return
        if ( c == '\r' )
            os << std::endl;
        // all type of spaces are substituted
        else if ( isspace(c) )
            os << ' ';
        // non=printable characters are removed
        else if ( isprint(c) )
            os << c;
        else
            std::cerr << "unprintable ascii "<< (int)c << " found" << std::endl;
    }
}


void StreamFunc::diff_stream(std::ostream & os, std::istream& val, std::istream& ref)
{
    std::string val_l, ref_l;
    val.seekg(0);
    ref.seekg(0);
    
    while ( val.good() )
    {
        std::getline(val, val_l);
        std::getline(ref, ref_l);
#if ( 0 )
        // print any line containing '{' or '}' 
        bool par = ( std::string::npos != ref_l.find_first_of("{}") );
        if ( val_l != ref_l || par )
#else
        if ( val_l != ref_l )
#endif
        {
            os << val_l << std::endl;
            //os << val_l << "  (" << ref_l << ")" << std::endl;
        }
    }
}


void StreamFunc::skip_lines(std::ostream & os, std::istream& is, char skip)
{
    std::string line;
    
    while ( is.good() )
    {
        std::getline(is, line);
        if ( line[0] != skip )
            os << line << std::endl;
    }
}



void StreamFunc::prefix_lines(std::ostream & os, std::istream& is, std::string const& prefix,
                              char keep, char skip)
{
    std::string line;
    
    while ( is.good() )
    {
        std::getline(is, line);
        if ( line[0] == 0 )
            os << line << std::endl;
        else if ( line[0] == keep )
            os << line << std::endl;
        else if ( line[0] == skip )
            ;
        else
            os << prefix << line << std::endl;
    }
}


/**
 The alignment of the vertical bar should match the one in PREF
 */
void print_line(std::ostream & os, unsigned int cnt, std::string line)
{
    os << std::setw(8) << cnt << "  " << line << std::endl;
}


/**
 show_line() will output one line extracted from `is',
 and indicate the position `pos` with a arrowhead in a second line.
 */
void StreamFunc::show_line(std::ostream & os, std::istream & is, std::streampos pos)
{
    if ( !is.good() )
        is.clear();
    
    std::streampos isp = is.tellg();
    is.seekg(0, std::ios::beg);

    unsigned int cnt = 0;
    std::string line;
    
    while ( is.good()  &&  is.tellg() <= pos )
    {
        std::getline(is, line);
        ++cnt;
    }
    
    print_line(os, cnt, line);
    
    std::streampos lip = line.size() + 1 + pos - is.tellg();
    os << "        | ";
    for ( int n = 0; n < lip; ++n )
        os << " ";
    os << "^" << std::endl;
    
    is.clear();
    is.seekg(isp, std::ios::beg);
}


std::string StreamFunc::show_line(std::istream & is, std::streampos pos)
{
    std::ostringstream oss;
    show_line(oss, is, pos);
    return oss.str();
}


/**
 Output enough lines to cover the area specified by [start, end].
 Each line is printed with a line number
 */
void StreamFunc::print_lines(std::ostream & os, std::istream & is,
                             std::streampos start, std::streampos end)
{
    if ( !is.good() )
        is.clear();
    
    std::streampos isp = is.tellg();
    is.seekg(0, std::ios::beg);
    std::string line;
    
    unsigned int cnt = 0;
    while ( is.good()  &&  is.tellg() <= start  )
    {
        std::getline(is, line);
        ++cnt;
    }

    print_line(os, cnt, line);
    while ( is.good() &&  is.tellg() < end )
    {
        std::getline(is, line);
        ++cnt;
        print_line(os, cnt, line);
    }

    is.clear();
    is.seekg(isp, std::ios::beg);
}


void StreamFunc::show_lines(std::ostream & os, std::istream & is,
                            std::streampos start, std::streampos end)
{
    os << "in\n";
    print_lines(os, is, start, end);
}


std::string StreamFunc::get_lines(std::istream & is, std::streampos s, std::streampos e)
{
    std::ostringstream oss;
    print_lines(oss, is, s, e);
    return oss.str();
}


int StreamFunc::find_and_replace(std::string & src,
                                 std::string const& fnd, std::string const& rep)
{
    int num = 0;
    size_t fLen = fnd.size();
    size_t rLen = rep.size();
    size_t pos = src.find(fnd, 0);
    while ( pos != std::string::npos )
    {
        ++num;
        src.replace(pos, fLen, rep);
        pos += rLen;
        pos = src.find(fnd, pos);
    }
    return num;
}


