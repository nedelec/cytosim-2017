// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "assert_macro.h"
#include <iostream>
#include <string>

/// elementary tokenizer
/** A Tokenizer is used cut a character stream into words */
namespace Tokenizer
{
    /// return the corresponding closing delimiter character, or 0 if `entry_char` is not a known delimiter
    char block_delimiter(char entry_char);

    /// skip space and new-line if `eat_line`==true, return the next character on the stream
    char eat_space(std::istream & is, bool eat_line);
    
    /// skip all character for which isspace() is true, and new-line if `eat_line`==true
    char get_character(std::istream & is, bool eat_line=false, bool eat_space=true);
    
    /// accumulate characters as long as valid() remains true
    std::string get_symbol(std::istream & is, bool (*valid)(char) );
    
    /// return next token that looks like a variable name
    std::string get_identifier(std::istream & is, bool eat_line=false);
    
    /// return next token
    std::string get_token(std::istream & is, bool eat_line=false);
        
    /// accumulate characters until new-line is found
    std::string get_line(std::istream & is);

    /// read next block delimited by `c_in` and the corresponding closing delimiter
    std::string get_block_content(std::istream & is, char c_in, char c_out);
    
    /// skip spaces and read a block delimited by `c_in`, or return empty string if `c_in` is not found
    std::string get_block(std::istream & is, char c_in);
    
    /// remove enclosing parenthesis at the start and at the end of `blok`
    std::string strip_block(std::string const& blok);

    /// read until `what` is found and stop immediately before (`what` is excluded from the returned string)
    std::string get_until(std::istream & is, std::string what);

    /// remove characters present in `ws` from the beggining and at the end of `str`
    void trim(std::string& str, const std::string& ws = " ");
    
    /// read an integer
    bool get_integer(std::istream&, int&);

    /// read an unsigned integer
    bool get_integer(std::istream&, unsigned&);

    /// try to interpret `str` as `UINT sub`. If successful, `str` is modified to be `sub`
    bool split_integer(std::string& str, unsigned&);
    
}

#endif


