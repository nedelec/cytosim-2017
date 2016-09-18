// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
//Some error conditions are handled by throwing exceptions.
//here we define a very primite Exception class for cytosim

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "assert_macro.h"
#include <string>
#include <sstream>
#include <cstdarg>


/// This is used to align text in the error messages
extern const char PREF[];


/// A mechanism to handle errors (see C++ manual)
/** 
Throw an Exception (not a pointer), and catch a reference to an exception.
This ensures proper memory managment (coordinated calls of constructor / destructor)
*/
class Exception 
{
    
protected:
    
    /// message associated with the exception
    std::string msg;
    
public:
    
    /// Creator with empty message
    Exception()
    {
        msg = "\0";
    }
    
    /// constructor with given message
    Exception(const std::string m)
    {
        msg = m;
        //printf("Exception(%s)\n", msg.c_str());
    }
    
    /// Destructor (exceptions should have empty destructors)
    virtual ~Exception()
    {
        //printf("~Exception(%s)\n", msg.c_str());
    }
    
    /// return the message
    const char* what() const
    {
        return msg.c_str();
    }
    
    /// change the message
    void  what(const std::string& m)
    {
        msg = m;
    }
    
    /// concatenate `s` and `a` to build message
    template <typename A>
    Exception(const std::string& s, const A& a)
    {
        std::ostringstream oss;
        oss << s << a;
        msg = oss.str();
    }
    
    /// concatenate `s`, `a` and `b` to build message
    template <typename A, typename B>
    Exception(const std::string& s, const A& a, const B& b)
    {
        std::ostringstream oss;
        oss << s << a << b;
        msg = oss.str();
    }

    /// concatenate `s`, `a`, `b` and `c` to build message
    template <typename A, typename B, typename C>
    Exception(const std::string& s, const A& a, const B& b, const C& c)
    {
        std::ostringstream oss;
        oss << s << a << b << c;
        msg = oss.str();
    }

    /// concatenate `s`, `a`, `b`, `c` and `d` to build message
    template <typename A, typename B, typename C, typename D>
    Exception(const std::string& s, const A& a, const B& b, const C& c, const D& d)
    {
        std::ostringstream oss;
        oss << s << a << b << c << d;
        msg = oss.str();
    }

    /// append `m` to message
    Exception&  operator << (const std::string m)
    {
        msg.append(m);
        return *this;
    }
    
    /// append `x` to message
    template<typename T>
    Exception&  operator << (const T& x)
    {
        std::ostringstream oss;
        oss << x;
        msg.append(oss.str());
        return *this;
    }
};


//------------------------------------------------------------------------------
/// This class is thrown if a parameter value is invalid
class InvalidParameter : public Exception 
{
    
public:
    
    /// constructor
    InvalidParameter() : Exception()
    {
        //printf("new InvalidParameter [%s]\n", m.c_str());
    }
    
    /// constructor
    InvalidParameter(const std::string m) : Exception(m)
    {
        //printf("new InvalidParameter [%s]\n", m.c_str());
    }

    /// concatenate all arguments to build message
    template <typename A>
    InvalidParameter(const std::string& s, const A& a) : Exception(s, a) {}
    
    /// concatenate all arguments to build message
    template <typename A, typename B>
    InvalidParameter(const std::string& s, const A& a, const B& b) : Exception(s,a,b) {}
 
    /// concatenate all arguments to build message
    template <typename A, typename B, typename C>
    InvalidParameter(const std::string& s, const A& a, const B& b, const C&c) : Exception(s,a,b,c) {}

    /// concatenate all arguments to build message
    template <typename A, typename B, typename C, typename D>
    InvalidParameter(const std::string& s, const A& a, const B& b, const C& c, const D& d) : Exception(s,a,b,c,d) {}
    
    /// destructor
    virtual ~InvalidParameter() {};
};


//------------------------------------------------------------------------------
/// InvalidSyntax is thrown while parsing config file
class InvalidSyntax : public Exception 
{
    
    public :
    
    /// constructor
    InvalidSyntax(std::string const& m) : Exception(m)
    {
        //printf("new InvalidSyntax [%s]\n", m.c_str());
    }
    
    /// destructor
    virtual ~InvalidSyntax() {};
};

//------------------------------------------------------------------------------
/// InvalidIO is thrown during file Input/Output
class InvalidIO : public Exception 
{
    
public :
    
    /// constructor
    InvalidIO(const std::string m) : Exception(m)
    {
        //printf("new InvalidIO [%s]\n", m.c_str());
    }

    /// destructor
    virtual ~InvalidIO() {};
};


#endif
