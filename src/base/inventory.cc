// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "inventory.h"
#include "assert_macro.h"
#include "exceptions.h"


Inventory::Inventory()
{
    lowest    = 1;
    highest   = 0;
    allocated = 8;
    byNames   = new Inventoried*[allocated];
    
    for ( Number n = 0; n < allocated; ++n )
        byNames[n] = 0;
}


Inventory::~Inventory()
{
    delete[] byNames;
}


void Inventory::allocate(Number sz)
{
    const unsigned chunk = 32;
    sz = ( sz + chunk - 1 ) & -chunk;
    
    Inventoried ** byNames_new = new Inventoried*[sz];
    
    Number n = 0;
    for ( ; n < allocated; ++n )
        byNames_new[n] = byNames[n];
    while ( n < sz )
        byNames_new[n++] = 0;
    
    delete[] byNames;
    byNames   = byNames_new;
    allocated = sz;
}


//------------------------------------------------------------------------------

Number Inventory::first_assigned() const
{
    Number n = 1;
    while ( n < allocated )
    {
        if ( byNames[n] )
            return n;
        ++n;
    }
    return 0;
}


Number Inventory::last_assigned() const
{
    Number n = allocated-1;
    while ( n > 0 )
    {
        if ( byNames[n] )
            return n;
        --n;
    }
    return 0;
}


Number Inventory::next_assigned(Number n) const
{
    ++n;
    while ( n < allocated )
    {
        if ( byNames[n] )
            return n;
        ++n;
    }
    return 0;
}


Number Inventory::first_unassigned()
{
    Number n = lowest;
    
    if ( n < allocated )
    {
        if ( byNames[n] == 0 )
            return n;
        
        while ( n < allocated  &&  byNames[n] )
            ++n;
        
        lowest = n;
    }
    
    return n;
}

//------------------------------------------------------------------------------

/**
 This will assign a new serial-number for `obj`, if it does not have one.
 */
void Inventory::assign(Inventoried * obj)
{
    Number & n = obj->nNumber;
    
    if ( n == 0 )
        n = ++highest;
    else if ( highest < n )
        highest = n;
    
    if ( n >= allocated )
        allocate(n+1);
    
    assert_true( byNames[n] == 0 );
    
    byNames[n] = obj;
    //std::err << "Inventory::store() assigned " << n << " to " << obj << "\n";
}


void Inventory::unassign(const Inventoried * obj)
{
    Number n = obj->nNumber;
    assert_true( n < allocated );
    byNames[n] = 0;
    
    if ( lowest >= n )
        lowest = n;
    
    while ( byNames[highest] == 0  &&  highest > 0 )
        --highest;
}


Inventoried * Inventory::get(const Number n) const
{
    if ( n < allocated )
    {
        assert_true( byNames[n]==0  ||  byNames[n]->number()==n );
        return byNames[n];
    }
    return 0;
}


Inventoried* Inventory::first() const
{
    Number n = 1;
    while ( n < allocated )
    {
        if ( byNames[n] )
            return byNames[n];
        ++n;
    }
    return 0;
}


Inventoried* Inventory::last() const
{
    Number n = highest;
    while ( n > 0 )
    {
        if ( byNames[n] )
            return byNames[n];
        --n;
    }
    return 0;
}


Inventoried* Inventory::previous(Inventoried const* i) const
{
    Number n = i->nNumber - 1;
    while ( n > 0 )
    {
        if ( byNames[n] )
            return byNames[n];
        --n;
    }
    return 0;
}

#include <iostream>
Inventoried* Inventory::next(Inventoried const* i) const
{
    Number n = i->nNumber + 1;
    while ( n < allocated )
    {
        if ( byNames[n] )
            return byNames[n];
        ++n;
    }
    return 0;
}

//------------------------------------------------------------------------------
unsigned Inventory::count() const
{
    unsigned cnt = 0;
    for ( Number n = 0; n < allocated; ++n )
        if ( byNames[n] ) ++cnt;
    return cnt;
}


void Inventory::reassign()
{
    Number max = last_assigned();
    Number next = 1;
    Number nn   = 1;
    
    while ( nn <= max )
    {
        while ( nn <= max  &&  byNames[nn] == 0 )
            ++nn;
        if ( nn > max )
            break;
        if ( next < nn )
        {
            byNames[next] = byNames[nn];
            byNames[nn]   = 0;
            byNames[next]->number(next);
        }
        ++next;
        ++nn;
    }
    
    lowest = next;
    highest = next-1;
}


void Inventory::clear()
{
    for ( Number n = 0; n < allocated; ++n )
        byNames[n] = 0;
    //std::clog << "Inventory::forgetAll() removed " << cnt << "numbers\n";
    lowest = 1;
    highest = 0;
}


//------------------------------------------------------------------------------
std::ostream & operator << (std::ostream & os, const Inventory& inv)
{
    os << "Inventory " << &inv << std::endl;
    for ( Number n = 0; n < inv.capacity(); ++n )
        os << n << " -> " << inv[n] << std::endl;
    return os;
}

