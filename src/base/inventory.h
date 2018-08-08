// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef INVENTORY_H
#define INVENTORY_H

#include "inventoried.h"
#include "assert_macro.h"
#include <ostream>

/// Attributes and remember serial-numbers to Inventoried
/**
 A Inventory assigns serial-numbers (of type Number) to Inventoried,
 and it records a pointer to these objects.
 
 The pointers can be recovered from their 'number' in constant time.
 
 \author Nedelec, August 2003. EMBL Heidelberg. nedelec@embl.de
 */
class Inventory
{
private:
    
    /// array of objects, by their name
    /* Stores pointers to the objects, such that byNames[N]->nNumber == N ) for any N > 0. */
    Inventoried ** byNames;
    
    
    /// size of memory allocated
    Number    allocated;
    
    /// lowest i > 0 for which byNames[i] == 0
    Number    lowest;
    
    /// highest i > 0 for which byNames[i] != 0
    Number    highest;
    
    /// memory allocation function
    void      allocate(Number size);
    
    ///update available when the spot has been taken
    void      updateFirstFree(Number start = 0);
    
    /// Disabled copy constructor
    Inventory(Inventory const&);
    
    /// Disabled copy assignment
    Inventory& operator=(Inventory const&);
    
public:
    
    /// Constructor
    Inventory();
    
    /// Destructor
    ~Inventory();
    
    /// the smallest assigned number
    Number         first_assigned() const;
    
    /// the largest assigned number
    Number         last_assigned() const;
    
    /// lowest assigned number strictly greater than `n`
    Number         next_assigned(Number n) const;
    
    /// the smallest unassigned number
    Number         first_unassigned();
    
    /// current size of array
    Number         capacity() const { return allocated; }
    
    /// remember `obj`, assign a new Number if necessary
    void           assign(Inventoried * obj);
    
    /// forget the object and release its serial number
    void           unassign(const Inventoried * obj);
    
    /// return the object with given serial number, or 0 if not found
    Inventoried*   get(Number number) const;
    
    /// object with the smallest inventory number
    Inventoried*   first() const;
    
    /// object with the largest inventory number
    Inventoried*   last() const;
    
    /// return object just before in the inventory
    Inventoried*   previous(Inventoried const*) const;
    
    /// return object just after in the inventory
    Inventoried*   next(Inventoried const*) const;
    
    /// return object with given number
    Inventoried*   operator[](Number n) const { assert_true(n<allocated); return byNames[n]; }
    
    /// number of non-zero entries in the registry
    unsigned int   count() const;
    
    /// reattribute all serial numbers consecutively and pack the array
    void           reassign();
    
    /// clear all entries
    void           clear();
};



/// output of all values
std::ostream & operator << (std::ostream&, const Inventory&);


#endif
