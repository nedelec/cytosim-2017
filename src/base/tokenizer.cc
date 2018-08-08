// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "assert_macro.h"
#include "tokenizer.h"
#include "exceptions.h"

/// set to 1 to enable debugging output
#define VERBOSE_TOKENIZER 0


//------------------------------------------------------------------------------
char Tokenizer::eat_space(std::istream & is, bool eat_line)
{
    char c = is.peek();
    while ( isspace(c) )
    {
        if ( c == '\n' && ! eat_line )
            break;
        is.get(c);
        if ( is.fail() )
            return 0;
        c = is.peek();
    }
    return c;
}

//------------------------------------------------------------------------------
char Tokenizer::get_character(std::istream & is, bool eat_line, bool eat_space)
{
    char c = 0;
    do {
        is.get(c);
        if ( 1 != is.gcount() )
            return 0;
#if ( 0 )
        if ( c == COMMENT_START )
        {
            std::string line;
            std::getline(is, line);
            c = '\n';
        }
#endif
        if ( c == '\n' && ! eat_line )
            break;
    } while ( eat_space && isspace(c) );
    return c;
}

//------------------------------------------------------------------------------
std::string Tokenizer::get_symbol(std::istream & is, bool (*valid)(char))
{
    std::string res;
    char c = 0;
    while ( is.good() )
    {
        is.get(c);
        if ( 1 != is.gcount() )
            break;
        if ( valid(c) )
            res.push_back(c);
        else
        {
            is.unget();
            break;
        }
    } 
    return res;
}

//------------------------------------------------------------------------------
#pragma mark -

bool valid_identifier(char c)
{
    return isalnum(c) || c=='_' || c=='-';
}

bool valid_token(char c)
{
    return isalnum(c) || c=='_' || c=='-' || c=='/' || c=='\\' || c=='.' || c==':';
}

bool valid_integer(char c)
{ 
    return isdigit(c);
}

bool valid_number(char c)
{
    return isdigit(c) || c=='.' || c=='e' || c=='x' || c=='+' || c=='-';
}

bool valid_hexadecimal(char c)
{ 
    return isxdigit(c) || c=='x';
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 get_identifier() reads words:
 - starting with a alpha-character,
 - followed by alphanum characters
 .
 */
std::string Tokenizer::get_identifier(std::istream & is, bool eat_line)
{
    char c = Tokenizer::eat_space(is, eat_line);
    
    if ( !isalpha(c) )
        return "";
     
    std::string res = Tokenizer::get_symbol(is, valid_identifier);

#if VERBOSE_TOKENIZER > 0
    std::cerr << "NAME |" << res << "|" << std::endl;
#endif

    return res;
}


/**
 get_token() reads a block enclosed by '{}', '()' and '""',
 and returns it verbatim with the delimiting characters.
 */

#if VERBOSE_TOKENIZER > 0
std::string get_token0(std::istream & is, bool eat_line)
#else
std::string Tokenizer::get_token(std::istream & is, bool eat_line)
#endif
{
    char c = Tokenizer::get_character(is, eat_line, true);
    char d = is.peek();
    
    if ( Tokenizer::block_delimiter(c) )
        return Tokenizer::get_block_content(is, c, Tokenizer::block_delimiter(c));
    
    if ( isalpha(c) || c == '/' || c == '.' )
    {
        if ( d == EOF )
            return std::string(1,c);
        is.unget();
        return Tokenizer::get_symbol(is, valid_token);
    }
    
    if ( c=='0' && d=='x' )
    {
        is.unget();
        return Tokenizer::get_symbol(is, valid_hexadecimal);
    }
    
    if ( isdigit(c) || (( c=='-' || c=='+' ) && isdigit(d)) )
    {
        is.unget();
        return Tokenizer::get_symbol(is, valid_number);
    }
    
    // anything else is void or one character long:
    if ( c )
        return std::string(1,c);
    else
        return std::string("");
}



#if VERBOSE_TOKENIZER > 0
std::string Tokenizer::get_token(std::istream & is, bool eat_line)
{
    std::string res = get_token0(is, eat_line);
    std::cerr << "TOKEN |" << res << "|" << std::endl;
    return res;
}
#endif

//------------------------------------------------------------------------------
std::string Tokenizer::get_line(std::istream & is)
{
    std::string res;
    std::getline(is, res);
    return res;
}


//------------------------------------------------------------------------------

char Tokenizer::block_delimiter(char c)
{
    switch(c)
    {
        case '(': return ')';
        case '{': return '}';
        case '[': return ']';
        case '"': return '"';
    }
    return 0;
}

/**
 This will read a block, assuming that character \a c_in has been read already.
 It will read characters until the corresponding closing delimiter is found.
 */
std::string Tokenizer::get_block_content(std::istream & is, char c, const char c_out)
{
    assert_true(c_out);
    std::string res;
    
    if ( c )
        res.push_back(c);
    is.get(c);
    
    while ( is.good() )
    {
        res.push_back(c);
        
        if ( c == c_out )
            return res;
        else if ( block_delimiter(c) )
            res.append( get_block_content(is, 0, block_delimiter(c)) );
        else if ( c == ')' || c == '}' )
            throw InvalidSyntax("unclosed block '"+std::string(1,c)+"'");
#if ( 0 )
        else if ( c == COMMENT_START )
        {
            // Read comments as lines, to inactivate symbols within: ')' and '}'
            std::string line;
            std::getline(is, line);
            res.append(line+"\n");
        }
#endif
        is.get(c);
    }
    
    throw InvalidSyntax("missing '"+std::string(1,c_out)+"'");
    return "";
}


/**
 This will skip spaces and new-lines until a character is found.
 If this character is equal to \a c_in, then the block is read and returned.
 Otherwise returns empty string "".
 
 @returns content of the block without delimiters
 */
std::string Tokenizer::get_block(std::istream & is, char c_in)
{
    assert_true(c_in);
    
    char c = get_character(is, true);
    
    if ( c == c_in )
    {
        std::string res = get_block_content(is, 0, block_delimiter(c_in));
        res.resize(res.size()-1);
#if VERBOSE_TOKENIZER > 0
        std::cerr << "BLOCK |" << res << "|" << std::endl;
#endif
        return res;
    }

    is.unget();
    return "";
}


std::string Tokenizer::strip_block(std::string const& blok)
{
    int x = blok.size()-1;
    
    if ( x < 1 )
        return blok;
    
    char c = block_delimiter( blok[0] );
    if ( c )
    {
        if ( blok[x] != c )
            throw InvalidSyntax("missmatched enclosing symbols");
        return blok.substr(1, x-1);
    }
    return blok;
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 read an integer, or return false if that is not possible.
 The value of T will not change, if the input fails.
 */
bool Tokenizer::get_integer(std::istream & is, unsigned& what)
{
    unsigned val = what;
    is >> what;
    if ( is.fail() )
    {
        is.clear();
        what = val;
        return false;
    }
    return true;
}

/**
 read an integer, or return false if that is not possible.
 The value of T will not change, if the input fails.
 */
bool Tokenizer::get_integer(std::istream & is, int& what)
{
    int val = what;
    is >> what;
    if ( is.fail() )
    {
        is.clear();
        what = val;
        return false;
    }
    return true;
}


/**
 split the string into an integer and the remaining string.
 The space after the integer is discarded.
 
 @return success == true
 */
bool Tokenizer::split_integer(std::string& str, unsigned& what)
{
    std::istringstream iss(str);
    unsigned val = what;
    iss >> what;
    eat_space(iss, false);
    if ( iss.fail() )
    {
        what = val;
        return false;
    }
    str = str.substr(iss.tellg());
    return true;
}


std::string Tokenizer::get_until(std::istream& is, std::string what)
{
    std::string res;
    int d = 0;
    char c;
    is.get(c);
    
    while ( is.good() )
    {
        if ( c == what[d] )
        {
            ++d;
            if ( what[d] == '\0' )
                return res;
        }
        else
        {
            if ( d == 0 )
            {
                res.push_back(c);
            }
            else
            {
                res.push_back(what[0]);
                if ( d > 1 ) {
                    is.seekg(-d, std::ios_base::cur);
                    d = 0;
                } else {
                    if ( c == what[0] )
                        d = 1;
                    else {
                        res.push_back(c);
                        d = 0;
                    }
                }
            }
        }
        is.get(c);
    }
    return res;
}


void Tokenizer::trim(std::string& str, const std::string& ws)
{
    std::string::size_type pos = str.find_last_not_of(ws);
    if ( pos != std::string::npos )
    {
        str.erase(pos+1);
        pos = str.find_first_not_of(ws);
        if ( pos != std::string::npos )
            str.erase(0, pos);
    }
    else str.clear();
}



