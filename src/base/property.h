// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef PROPERTY_H
#define PROPERTY_H

#include "assert_macro.h"
#include <iostream>
#include <iomanip>
#include <string>
#include "real.h"

class Glossary;
class PropertyList;
class SimulProp;


/// A Property holds the parameters for a particular kind of objects
/**
A property is a list of parameters associated with a kind of object in the simulation
 -# It is identified by kind() and name():
    - kind() indicates the type of object (eg. fiber, hand)
    - The name is unique to each instantiation and is set by the user (eg. actin, microtubule).
 -# Methods mostly deal with input / output, which are done using Glossary.
 -# clear(), read() and complete() should be defined to handle the parameters.
 .
 
 The customizable simulation object hold pointers to an associated Property.
 A Property defacto defines a class of objects.
 All the objects that point to the same property are of the same kind.
 */
class Property
{
private:
    
    /// the name of the property
    std::string  mName;
    
    /// numerical identifier used in output file (default is -1)
    int          mIndex;
    
    /// disabled default constructor:
    Property();

public:
    
    /// constructor must provide a name
    explicit     Property(const std::string& n, const int index = -1);

    /// destructor
    virtual     ~Property();
    
    //-------------------------------------------------------------------------------
    
    /// the 'kind' of property (a class identifier)
    virtual std::string kind()            const { return "undefined"; }
    
    //-------------------------------------------------------------------------------
    
    /// return identifier for instantiation
    std::string  name()                   const { return mName; }
    
    /// change name
    void         name(const std::string& n)     { mName = n; }
        
    /// true if 'name' matches
    bool         is_named(const std::string& n) { return ( n == mName ); }
        
    //-------------------------------------------------------------------------------
    
    /// index, unique among all Property of similar kind()
    int          index()                  const { return mIndex; }
    
    /// set index in the array of Properties
    void         index(int x)                   { mIndex = x; }
    
    //-------------------------------------------------------------------------------
    
    /// clear parameters to default values
    virtual void clear() = 0;
    
    /// return new object of same class with identical parameters
    /**
     The new object is created with `new` and should be destroyed with `delete`
     */
    virtual Property* clone() const = 0;
    
    /// true if at least one value is different than its default setting
    bool         modified() const;
    
    //-------------------------------------------------------------------------------
    
    /// set from a Glossary
    virtual void read(Glossary&) = 0;

    /// set from a string, return number of values assigned
    void         readString(std::string&, char stamp);

    /// set from a string, return number of values assigned
    void         readFile(const char file[]);
    
    //-------------------------------------------------------------------------------
    
    /// set variables derived from the parameters, and check consistency of values
    /**
     The arguments provide the global SimulProp, and the list of all known Property.
     Any Property created within this function should be added to \a plist.
     complete() is usually called after read()
     */
    virtual void complete(SimulProp const* sp, PropertyList* plist) {}
    
    //-------------------------------------------------------------------------------

    /// formatted output of one parameter
    template<typename C>
    static  void write_param(std::ostream& os, std::string const& name, C const& c)
    {
        os << " " << std::left << std::setw(14) << name << " = " << c << ";" << std::endl;
    }

    /// formatted output of one parameter
    template<typename C>
    static  void write_param(std::ostream& os, std::string const& name, C const* c, int cnt)
    {
        assert_true( cnt > 0 );
        os << " " << std::left << std::setw(14) << name << " = " << c[0];
        for ( int i = 1; i < cnt; ++i )
            os << ", " << c[i];
        os << ";" << std::endl;
    }

    /// formatted output of one parameter
    template<typename C, typename D>
    static  void write_param(std::ostream& os, std::string const& name, C const& c, D const& d)
    {
        os << " " << std::left << std::setw(14) << name << " = " << c << ", " << d << ";" << std::endl;
    }

    /// formatted output of one parameter
    template<typename C, typename D, typename E>
    static  void write_param(std::ostream& os, std::string const& name, C const& c, D const& d, E const& e)
    {
        os << " " << std::left << std::setw(14) << name << " = " << c << ", " << d << ", " << e << ";" << std::endl;
    }

    /// formatted output of one parameter
    template<typename C, typename D, typename E, typename F>
    static  void write_param(std::ostream& os, std::string const& name, C const& c, D const& d, E const& e, F const& f)
    {
        os << " " << std::left << std::setw(14) << name << " = " << c << ", " << d << ", " << e << ", " << f << ";" << std::endl;
    }
    
    //-------------------------------------------------------------------------------

    /// write values
    virtual void write_data(std::ostream&) const = 0;
    
    /// write only values that differ from the ones specified in \a ref
    void         write_diff(std::ostream&, const Property* ref) const;

    /// if ( prune == true ), write values that differ from the default values
    void         write_diff(std::ostream&, bool prune) const;
    
    /// write header + data
    void         write(std::ostream&, bool prune = false) const;

};

/// printing operator
std::ostream& operator << (std::ostream&, const Property &);


#endif
