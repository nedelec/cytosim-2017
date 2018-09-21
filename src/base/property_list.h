// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef PROPERTY_LIST_H
#define PROPERTY_LIST_H

#include "assert_macro.h"
#include "property.h"
#include <iostream>
#include <vector>
class SimulProp;


/// a list of Property
class PropertyList
{
public:
    
    /// type of list used to store pointers to PROP
    typedef std::vector<Property*> vec_type;
    
    /// iterator class type
    typedef vec_type::iterator iterator;

    /// iterator class type
    typedef vec_type::const_iterator const_iterator;
    
private:
    
    /// list of non-null pointers to Properties
    vec_type vec;
    
public:
    
    /// constructor
    PropertyList()   { }
    
    /// destructor forget things without deleting objects
    ~PropertyList()  { }
    
    //-------------------------------------------------------------------------------
    
    /// add a new Property in the list, and set its index()
    void         deposit(Property * p, bool refuse_duplicate = true);
    
    /// push a new Property in the list
    void         push_back(Property * p) { vec.push_back(p); }

    /// forget pointer to p
    int          remove(Property * p);
    
    /// delete all Property
    void         erase();
    
    /// iterator pointing to first element
    iterator     begin()   { return vec.begin(); }
    
    /// iterator that points to a position just past the last element
    iterator     end()     { return vec.end(); }
        
    //-------------------------------------------------------------------------------
    
    /// true if no property are known
    bool         empty() const    { return vec.empty(); }
    
    /// number of known Property
    size_t       size()  const    { return vec.size(); }

    /// number of Property of that kind
    unsigned int number_of(std::string const& kind) const;
    
    /// return property stored at index \c n
    Property *   operator[] (const unsigned int n) const;

    /// apply function to all objects
    void         for_each(void func(Property *)) const;
    
    /// complete all objects
    void         complete(SimulProp const *);
    
    //-------------------------------------------------------------------------------
    
    /// return property which has the provided name, or zero if it cannot be found
    Property *   find(const std::string& name) const;

    /// return property which has the provided name, or zero if it cannot be found
    Property *   find(const std::string& kind, const std::string& name) const;

    /// return property which has the provided index, or zero if it cannot be found
    Property *   find(const std::string& kind, const int index) const;
    
    /// return property which has the provided name, and throw exception if it cannot be found
    Property *   find_or_die(std::string const& kind, std::string const& name) const;
    
    /// return property which has the provided index, and throw exception if it cannot be found
    Property *   find_or_die(std::string const& kind, const unsigned index) const;

    /// return list of properties of the given kind
    PropertyList find_all(const std::string& kind) const;
    
    /// return list of properties of the given kinds
    PropertyList find_all(const std::string& kind1, const std::string& kind2) const;

    /// return list of properties of the given kinds
    PropertyList find_all(const std::string&, const std::string&, const std::string&) const;
    
    /// return a properties of the given kind, that follows the given one
    Property *   find_next(const std::string& kind, Property *) const;
    
    /// return list of properties which are not of the given kind
    PropertyList find_all_except(const std::string& kind) const;

    /// return index where Property is stored, or -1 if not found
    int          find_index(const Property * p) const;

    //-------------------------------------------------------------------------------

    /// print names of known Property
    void         write_names(std::ostream &, std::string const&) const;
    
    /// write all Property
    void         write(std::ostream &, bool prune) const;
};

#endif

