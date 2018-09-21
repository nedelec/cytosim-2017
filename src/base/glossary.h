// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef GLOSSARY_H
#define GLOSSARY_H

#include "key_list.h"
#include "exceptions.h"
#include "assert_macro.h"
#include "tokenizer.h"
#include <iostream>
#include <sstream>
#include <string>
#include <map>

/// Glossary holds a list of (key, values) where both key and values are strings
/** 
 This class is used for reading configuration files:
 - Reads a std::istream to builds a std::map of <key, record>
 - Simple syntax based on = ( ) { } “ , ; % focused on flexible value setting.
 - each `record` is a list of values
 - Provides values upon requests with function set(key, index) .
 - A counter records the usage of the values.
 .
 
 Notes:
-# There can be an arbitrary number of Keys, and an abitrary number of values for each key.
-# Values are kept as strings, and are converted at request by templated functions:
   @code
   template <typename T> int set(T & ptr, std::string key)
   @endcode
   - `key` is the name under which the value appeared,
   - The template argument `T` defines the type of the parameter,
     and the value-string is interpreted accordingly,
   - The interpreted value is stored in `ptr`,
   - Returns 1 if `ptr` was set successfully, 0 otherwise
   .
-# The method warning() can report values that have not been used, 
 or that have been used more than once.
.
 
 Class reviewed by Andre Clapson on 10.03.2011.
 
 @todo parse and instantiate values like 'random.uniform()' or 'PI*30' or '0.1/60'
*/

class Glossary
{
public:
    
    /// type for a key
    typedef std::string                   key_type;
    
    /// a string-encoded value with a counter
    struct val_type 
    {
        /// the value specified in ascii
        std::string  str;
        /// number of times value has been used
        unsigned     cnt;
        
        /// constructor
        val_type()                     { cnt=0; }
        /// constructor with initialization
        val_type(std::string const& s) { str=s; cnt=0; }
    };
   
    /// a record is a set of values associated with a key
    typedef std::vector<val_type>         rec_type;
    
    /// type for the list of (key, record)
    typedef std::map<key_type, rec_type>  map_type;
    
    /// type of a pair (key, record)
    typedef std::pair<key_type, rec_type> pair_type;
    
private:
    
    /// ordered list of key-values pairs
    map_type   mTerms;
    
    //-------------------------------------------------------------------------------
    
    /// write a value, adding enclosing parenthesis if it contains a space
    static std::string format_value(const std::string&);
    
    /// read right-hand-side entry from a stream
    static void  read_values(pair_type&, std::istream&);
    
    /// read one entry from a stream
    static int   read_pair(pair_type&, std::istream&);
    
    /// read one entry from a stream
    void         add_pair(pair_type&, int no_overwrite);
    
    //-------------------------------------------------------------------------------
    #pragma mark -
    
    /// returns first non-space character in null-terminated C-string
    static char const* not_space(const char s[])
    {
        while(*s)
        {
            if ( isspace(*s) )
                ++s;
            else
                return s;
        }
        return 0;
    }
    
    
    /// check that nothing other than spaces is left in `iss`
    static bool check_remainder(std::istringstream& iss, std::string const& msg)
    {
        if ( iss.good() && !iss.eof() )
        {
            char tail[256] = { 0 };
            iss.get(tail, sizeof(tail));

            char const* tt = not_space(tail);
            if ( tt )
            {
                std::cerr << "Warning: ignored trailing `" << tt << "' while reading:" << std::endl;
                std::cerr << PREF << msg << std::endl;
                return 1;
            }
        }
        return 0;
    }
    
    
    /// set `var` from `val`
    template <typename T>
    void set_one(T & var, std::string const& val, key_type const& key) const
    {
        std::istringstream iss(val);
        
        iss >> var;

        if ( iss.fail() )
            throw InvalidSyntax("could not set `"+std::string(key)+"' from `"+val+"'");
        
        check_remainder(iss, key + "=" + val);
    }
    
    
    /// set enum of type T using a dictionary of correspondances
    template <typename T>
    void set_one(T & var, std::string const& val, KeyList<T> const& dict, key_type const& key) const
    {
        if ( ! dict.set(var, val) )
        {
            std::ostringstream oss;
            oss << "could not set `"+std::string(key)+"' from `"+val+"'\n";
            oss << dict;
            throw InvalidParameter(oss.str());
        }
    }
    
    //-------------------------------------------------------------------------------
    #pragma mark -

public:
    
    /// initialize
    explicit Glossary();

    /// this constructor calls read(in)
    explicit Glossary(std::istream& in);

    /// this constructor calls read()
    explicit Glossary(const std::string&);

    //-------------------------------------------------------------------------------

    /// true if no key were set
    bool         empty()   const { return mTerms.empty(); }

    /// number of keys
    size_t       nb_keys() const { return mTerms.size(); }
    
    /// return true if key is present, even if no value was set
    bool         has_key(key_type const&);
    
    /// return true if key is present; the key is deleted 
    bool         use_key(key_type const&);
    
    /// remove given key
    void         erase(key_type const&);
    
    /// clear all entries
    void         clear() { mTerms.clear(); }
    
    /// create a new Glossary with only the given key
    Glossary     extract(key_type const&);

    /// return number of values associated with a key
    size_t       nb_values(key_type const&);
    
    /// gives a pointer to the values corresponding to a key, or null if the key is not present
    rec_type *   values(key_type const&);

    /// gives a const pointer to the values corresponding to a key, or null if the key is not present
    rec_type const* values(key_type const&) const;

    /// return the value corresponding to the key and the index, or the empty string
    std::string  value(key_type const&, unsigned indx);

    /// report unused values and values used more than `threshold` times
    static int   warnings(std::ostream&, pair_type const&, unsigned threshold=1);
    
    /// report unused values and values used multiple times
    int          warnings(std::ostream &, unsigned threshold=1) const;
    
    //-------------------------------------------------------------------------------
    #pragma mark -
    
    /// this is equivalent to read('k = rhs')
    void         set_values(key_type const&, const std::string& rhs, int no_overwrite = 2);

    /// update the glossary to include assignments stored in a stream
    void         read(std::istream &, int no_overwrite = 2);

    /// update the glossary to include assignments stored in a string
    void         read(const std::string&, int no_overwrite = 2);
    
    /// read file specified in path
    void         readFile(const char path[], int no_overwrite = 2);

    /// a read for C-style command-line arguments
    void         readStrings(int argc, char* argv[], int no_overwrite = 2);
    
    /// write [ key = value1, value2, ... ]
    static void  write(std::ostream &, pair_type const&);

    /// write the number of time each value has been used
    static void  write_cnt(std::ostream &, pair_type const&);

    /// write all [key, values]
    void         write(std::ostream &) const;
    
    //-------------------------------------------------------------------------------
    
    /// set `var` from `key[inx]`. The counter associated to the value is incremented.
    template <typename T>
    int set(T & var, key_type const& key, unsigned inx = 0)
    {
        rec_type * rec = values(key);
        
        if ( rec == 0  || inx >= rec->size() )
            return 0;
        
        val_type & val = rec->at(inx);
        
        if ( val.str.empty() )
            return 0;

        set_one(var, val.str, key);
        ++val.cnt;
        return 1;
    }

    /// set `var` from `key[inx]`. The counter is not changed
    template <typename T>
    int peek(T & var, key_type const& key, unsigned inx = 0) const
    {
        rec_type const * rec = values(key);
        
        if ( rec == 0  || inx >= rec->size() )
            return 0;
        
        val_type const& val = rec->at(inx);

        if ( val.str.empty() )
            return 0;

        set_one(var, val.str, key);
        return 1;
    }
    
    
    /// set `cnt` values in the array `ptr[]`, starting at `key[0]`
    template <typename T>
    int set(T * ptr, unsigned cnt, key_type const& key)
    {
        rec_type * rec = values(key);
        
        if ( rec == 0 )
            return 0;
        
        int set = 0;
        for ( unsigned inx = 0; inx < rec->size() && inx < cnt; ++inx, ++set )
        {
            std::string val = rec->at(inx).str;
            if ( val.empty() )
                return 0;
            set_one(ptr[inx], val, key);
            ++rec->at(inx).cnt;
        }
 
        return set;
    }
   

    /// set `var` from `key[inx]`, using the dictionary `dict`
    template <typename T>
    int set(T & var, key_type const& key, KeyList<T> const& dict, unsigned inx = 0)
    {
        rec_type * rec = values(key);
        
        if ( rec == 0 || inx >= rec->size() )
            return 0;
        
        std::string val = rec->at(inx).str;
        if ( val.empty() )
            return 0;
        set_one(var, val, dict, key);
        ++rec->at(inx).cnt;
        return 1;
    }
    
    
    /// set the first value of the key: `key[0]=var`.
    template <typename T>
    void set_value(key_type const& key, const T& var, int no_overwrite = 2)
    {
        std::ostringstream oss;
        oss << var;
        set_values(key, oss.str(), no_overwrite);
    }
    
    
    /// check if value associated with `key` at index `inx` is made only of alpha characters
    int is_alpha(key_type const& key, unsigned inx = 0) const
    {
        rec_type const * rec = values(key);
        
        if ( rec == 0  || inx >= rec->size() )
            return 0;
        
        std::string const& str = rec->at(inx).str;
        
        if ( str.empty() )
            return 0;
        
        for ( std::string::const_iterator s = str.begin(); s != str.end(); ++s )
        {
            if ( ! isalpha(*s) &&  *s != '_' )
                return 0;
        }
        return 1;
    }
    
    /// check if value associated with `key` at index `inx` is a number
    /**
     @returns:
     - 0 if this is not a number
     - 2 for positive integer
     - 3 for negative integer
     - 4 for positive float
     - 5 for negative float
     .
     */
    int is_number(key_type const& key, unsigned inx = 0) const
    {
        rec_type const * rec = values(key);
        
        if ( rec == 0  || inx >= rec->size() )
            return 0;
        
        std::string const& val = rec->at(inx).str;
        
        char const* str = val.c_str();
        char * end;
        
        long i = strtol(str, &end, 10);
        if ( end > str && 0 == not_space(end) )
            return 2 + ( i < 0 );
        
        double d = strtod(str, &end);
        if ( end > str && 0 == not_space(end) )
            return 4 + ( d < 0 );
        
        return 0;
    }

};


#pragma mark -


/// special function for std::string arguments.
template <>
void Glossary::set_one(std::string& var, std::string const&, key_type const&) const;

/// special function for float
template <>
void Glossary::set_one(float& var, std::string const&, key_type const&) const;

/// special function for double
template <>
void Glossary::set_one(double& var, std::string const&, key_type const&) const;


/// input from stream
std::istream& operator >> (std::istream &, Glossary &);

/// output of one value
std::ostream & operator << (std::ostream &, const Glossary::pair_type &);

/// output of all values
std::ostream & operator << (std::ostream &, const Glossary &);

#endif


