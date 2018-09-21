// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "glossary.h"
#include "filepath.h"
#include "stream_func.h"
#include <fstream>
#include <cctype>
#include <iomanip>
#include "vector2.h"
#include "dim.h"


/// set to 1 to enable debugging output
#define VERBOSE_GLOSSARY 0


//------------------------------------------------------------------------------

Glossary::Glossary()
{
}


Glossary::Glossary(std::istream & in)
{
    read(in); 
}

Glossary::Glossary(const std::string& str)
{
    std::istringstream iss(str);
    read(iss);
}


//------------------------------------------------------------------------------
#pragma mark -

bool Glossary::has_key(key_type const& k)
{
    return ( mTerms.end() != mTerms.find(k) );
}


bool Glossary::use_key(key_type const& k)
{
    map_type::iterator w = mTerms.find(k);
    
    if ( w != mTerms.end() )
    {
        mTerms.erase(w);
        return true;
    }
    return false;
}


void Glossary::erase(key_type const& key)
{
    map_type::iterator w = mTerms.find(key);
    
    if ( w != mTerms.end() )
        mTerms.erase(w);
}


Glossary Glossary::extract(key_type const& key)
{
    Glossary res;
    map_type::iterator w = mTerms.find(key);
    
    if ( w != mTerms.end() )
        res.mTerms[key] = w->second;
    
    return res;
}

//------------------------------------------------------------------------------
#pragma mark -

size_t Glossary::nb_values(key_type const& k)
{
    map_type::iterator w = mTerms.find(k);
    if ( w != mTerms.end() )
        return w->second.size();
    else
        return 0;
}


Glossary::rec_type * Glossary::values(key_type const& key)
{
    map_type::iterator w = mTerms.find(key);
    return ( w == mTerms.end() ) ? 0 : &( w->second );
}


Glossary::rec_type const* Glossary::values(key_type const& key) const
{
    map_type::const_iterator w = mTerms.find(key);
    return ( w == mTerms.end() ) ? 0 : &( w->second );
}


std::string Glossary::value(key_type const& key, unsigned indx)
{
    map_type::iterator w = mTerms.find(key);
    if ( w != mTerms.end() )
    {
        if ( indx < w->second.size() )
        {
            w->second[indx].cnt++;
            return w->second[indx].str;
        }
    }
    return "";
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 This cuts a line into a KEY followed by one or more VALUES:
 KEY = VALUE1, VALUE2, ...
*/
int Glossary::read_pair(pair_type& res, std::istream & is)
{
    std::string k = Tokenizer::get_token(is, false);
    
    if ( k.empty() )
        return 1;

    res.first = k;
    
    char op = Tokenizer::get_character(is, false, true);
    
    if ( op != '=' ) 
        throw InvalidSyntax("missing assignment operator '='");
  
#if VERBOSE_GLOSSARY > 2
    std::cerr << "Glossary::   KEY |" << res.first << "|" << std::endl;
#endif

    read_values(res, is);
    return 0;
}

/**
 read the right-hand side of an assignment
 */
void Glossary::read_values(pair_type& res, std::istream & is)
{
    std::string k;
    
    do {
        k.clear();
        // skip spaces:
        char c = Tokenizer::get_character(is, false, true);
        
        if ( c == 0 || c == '\n' || c == ';' )
            return;
        if ( c == '%' )
        {
            Tokenizer::get_line(is);
            return;
        }
        if ( Tokenizer::block_delimiter(c) )
        {
            k = Tokenizer::get_block_content(is, 0, Tokenizer::block_delimiter(c));
            k.resize(k.size()-1);
        }
        else {
            //get everything up to first comma, semi-column or new-line
            while ( isprint(c) && c != ',' && c != ';' && c != '%' )
            {
                k.push_back(c);
                c = is.get();
            }
            if ( c != ',' )
                is.unget();
        }
        
        //remove any space at the end of the string:
        Tokenizer::trim(k);
        
        //add value:
        res.second.push_back(k);

#if VERBOSE_GLOSSARY > 2
        std::cerr << "Glossary::    VAL |" << k << "|" << std::endl;
#endif
        
    } while ( is.good() );
}



/**
 If `no_overwrite` == 0, an setting can erase a previous value,
 If `no_overwrite` == 1, symbols that are already present are not over-written,
 If `no_overwrite` == 2, a exception is thrown for any duplicate symbol
 */

void Glossary::add_pair(pair_type& pair, int no_overwrite)
{
#if VERBOSE_GLOSSARY > 0
    std::cerr << "Glossary::ADD " << pair << std::endl;
#endif
    
    map_type::iterator w = mTerms.find(pair.first);
    
    if ( w == mTerms.end() )
    {
        // this is a new key: add all values
        rec_type & rec = mTerms[pair.first];
        for ( unsigned v = 0; v < pair.second.size(); ++v )
            rec.push_back(pair.second[v]);
    }
    else
    {
        // this is an existing key, and we have to check each value
        rec_type & rec = w->second;
        for ( unsigned v = 0; v < pair.second.size(); ++v )
        {
            if ( rec.size() <= v )
                rec.push_back(pair.second[v]);
            else
            {
                if ( rec[v].str.empty()  ||  no_overwrite == 0 )
                    rec[v] = pair.second[v];
                else if ( pair.second[v].str != rec[v].str  &&  no_overwrite > 1 )
                {
                    std::ostringstream oss;
                    oss << "parameter `" << pair.first << "[" << v << "]' is already defined:\n";
                    oss << PREF << "known value = `" << rec[v].str << "'\n";
                    oss << PREF << "new setting = `" << pair.second[v].str << "' (ignored)\n";
                    throw InvalidParameter(oss.str());
                }
            }
        }
    }
}




/**
 This should be equivalent to read('k = rhs')
 */
void Glossary::set_values(key_type const& k, const std::string& rhs, int no_overwrite)
{
#if VERBOSE_GLOSSARY > 1
    std::cerr << "Glossary::SET " << k << " = |" << rhs << "|" << std::endl;
#endif
    
    pair_type pair;
    pair.first = k;
    
    std::istringstream iss(rhs);
    read_values(pair, iss);
    add_pair(pair, no_overwrite);
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 If `no_overwrite` == 0, a setting can erase a previous value,
 If `no_overwrite` == 1, symbols that are already present are not over-written,
 If `no_overwrite` == 2, a exception is thrown for any duplicate symbol
 */

void Glossary::read(std::istream & is, int no_overwrite)
{
    std::streampos isp;

    while ( is.good() )
    {
        char c = Tokenizer::eat_space(is, true);
        
        if ( c == EOF )
            return;
        
        // skip comments:
        if ( c == '%' )
        {
            std::string line;
            std::getline(is, line);
            continue;
        }

        isp = is.tellg();
        try {
            //we use a fresh variable every time for safety
            pair_type pair;
            if ( 0 == read_pair(pair, is) )
                add_pair(pair, no_overwrite);
        }
        catch( ... )
        {
            StreamFunc::show_line(std::cerr, is, isp);
            throw;
        }
    }
}


void Glossary::read(std::string const& str, int no_overwrite)
{
#if VERBOSE_GLOSSARY > 2
    std::cerr << "Glossary::READ |" << str << "|" << std::endl;
#endif
    std::istringstream iss(str);
    read(iss, no_overwrite);
}


void Glossary::readFile(const char path[], int no_overwrite)
{
    std::ifstream is(path);
    if ( is.good() )
        read(is, no_overwrite);
    else
        throw InvalidIO("could not open Glossary file");
    is.close();
}


/**
 This is useful to parse the command-line strings given to main().
 
 The following syntax will be accepted:
 FILE.EXT
 and recorded as:
 EXT = FILE.EXT
 */
void Glossary::readStrings(int argc, char* argv[], int no_overwrite)
{
    for ( int ii = 1; ii < argc; ++ii )
    {
#if VERBOSE_GLOSSARY > 0
        std::cerr << "Glossary::ARG |" << argv[ii] << "|" << std::endl;
#endif
        pair_type pair;
        try {
            
            std::string arg(argv[ii]);
            size_t spos = arg.rfind("=");
            if ( spos != std::string::npos )
            {
                std::istringstream iss(argv[ii]);
                if ( 0 == read_pair(pair, iss) )
                    add_pair(pair, no_overwrite);
            }
            else
            {
                /*
                 Here is a key specified without any value:
                 */
                if ( FilePath::is_dir(argv[ii]) )
                {
                    pair.first = "directory";
                    pair.second.push_back(arg);
                }
                else
                {
                    // identify a potential file name by the presence of an extension:
                    size_t spos = arg.rfind(".");
                    if ( spos != std::string::npos )
                    {
                        pair.first = arg.substr(spos);
                        pair.second.push_back(arg);
                    }
                    else
                    {
                        pair.first = arg;
                    }
                }
                add_pair(pair, no_overwrite);
            }
        }
        catch( Exception & e )
        {
            e << " in `" << argv[ii] << "'\n";
            throw;
        }
    }
}

//------------------------------------------------------------------------------
#pragma mark -

std::string Glossary::format_value(std::string const& str)
{
    if ( std::string::npos != str.find(' ') )
        return '(' + str + ')';
    else
        return str;
}


void Glossary::write(std::ostream & os, Glossary::pair_type const& pair)
{
    os << pair.first << " = ";
    if ( pair.second.size() > 0 )
    {
        os << format_value(pair.second[0].str);
        for ( unsigned v = 1; v < pair.second.size(); ++v )
            os << ", " << format_value(pair.second[v].str);
    }
    else
        os << "()";
    os << ";";
}

/**
 Write the usage-counter for each value.
 The width of each record will match what is printed by Glossary::write()
 */
void Glossary::write_cnt(std::ostream & os, Glossary::pair_type const& pair)
{
    if ( pair.second.size() > 0 )
    {
        os << std::setw(pair.first.size()) << "used" << " : ";
        os << std::setw(format_value(pair.second[0].str).size()) << pair.second[0].cnt;
        for ( unsigned v = 1; v < pair.second.size(); ++v )
            os << "," << std::setw(format_value(pair.second[v].str).size()+1) << pair.second[v].cnt;
    }
}


void Glossary::write(std::ostream & os) const
{
    for ( map_type::const_iterator n = mTerms.begin(); n != mTerms.end(); ++n )
    {
        write(os, *n);
        os << std::endl;
    }
}

//------------------------------------------------------------------------------
#pragma mark -

std::istream& operator >> (std::istream & is, Glossary& glos)
{
    glos.read(is);
    return is;
}

std::ostream & operator << (std::ostream & os, Glossary::pair_type const& pair)
{
    Glossary::write(os, pair);
    return os;
}

std::ostream & operator << (std::ostream & os, Glossary const& glos)
{
    glos.write(os);
    return os;
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 @returns:
 - 1 if something suspicious was detected, and a warning was issued,
 - 0 otherwise
 .
 @todo: use color only if the terminal supports it
 */
int Glossary::warnings(std::ostream& os, Glossary::pair_type const& pair, unsigned threshold)
{
    int used = 0, unused = 0, overused = 0;
    const rec_type& rec = pair.second;
        
    for ( unsigned v = 0; v < rec.size(); ++v )
    {
        val_type const& val = rec[v];
        if ( val.cnt > 0 )
            used = 1;
        if ( val.cnt < 1 && val.str.size() )
            unused = 1;
        else if ( val.cnt > threshold )
            overused = 1;
    }
    
    std::string warn;
        
    if ( !used )
        warn = "this parameter was ignored";
    else if ( unused )
        warn = "a value was unused";
    if ( overused )
        warn = "some value may have been overused";
        
    if ( warn.size() )
    {
        os << "Warning, " + warn + ":\n";
        os << PREF;
        write(os, pair);
        os << "\n";
        if ( used )
        {
            os << PREF;
            write_cnt(os, pair);
            os << "\n";
        }
        os << std::flush;
        return 1;
    }
    return 0;
}


/**
 @returns the number of warnings that were issued
 */

int Glossary::warnings(std::ostream& os, unsigned threshold) const
{
    int res = 0;
    for ( map_type::const_iterator n = mTerms.begin(); n != mTerms.end(); ++n )
        res += warnings(os, *n, threshold);
    return res;
}

//------------------------------------------------------------------------------

/**
 This copies the string, removing spaces
*/
template <>
void Glossary::set_one(std::string& var, std::string const& val, key_type const& key) const
{
    var = val;
    Tokenizer::trim(var);
    
#if ( VERBOSE_GLOSSARY > 0 )
    std::cerr << "Glossary::STRING " << key << " = |" << var << "|" << std::endl;
#endif
}


/**
 This reads a floating point value,
 also accepting 'inf', '+inf' and '-inf'
 */
template <>
void Glossary::set_one(float& var, std::string const& val, key_type const& key) const
{
    std::istringstream iss(val);
    iss >> var;
    
    if ( iss.fail() )
    {
        if ( val == "inf" || val == "+inf" )
        {
            var = INFINITY;
            return;
        }
        
        if ( val == "-inf" )
        {
            var = -INFINITY;
            return;
        }

        throw InvalidSyntax("could not set `"+std::string(key)+"' from `"+val+"'");
    }
    
    check_remainder(iss, key + "=" + val);
}

/**
 This reads a floating point value,
 also accepting 'inf', '+inf' and '-inf'
 */
template <>
void Glossary::set_one(double& var, std::string const& val, key_type const& key) const
{
    std::istringstream iss(val);
    iss >> var;
    
    if ( iss.fail() )
    {
        if ( val == "inf" || val == "+inf" )
        {
            var = INFINITY;
            return;
        }
        
        if ( val == "-inf" )
        {
            var = -INFINITY;
            return;
        }
        
        throw InvalidSyntax("could not set `"+std::string(key)+"' from `"+val+"'");
    }
    
    check_remainder(iss, key + "=" + val);
}


