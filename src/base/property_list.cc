// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "property_list.h"
#include "exceptions.h"
#include <iomanip>

void PropertyList::erase()
{
    for ( const_iterator n = vec.begin(); n != vec.end(); ++n )
        delete( *n );
    vec.clear();
}

/**
 If ( p == 0 ) nothing is done.

 The index of the property is set from the number of 
 Property of the same kind() already present in the list.
 */
void PropertyList::deposit(Property * p, bool refuse_duplicate)
{
    if ( p == 0 )
        return;
    
    int cnt = 0;
    for ( const_iterator n = vec.begin(); n != vec.end(); ++n )
    {
        if ( (*n)->kind() == p->kind() )
        {
            ++cnt;
            if ( refuse_duplicate  &&  (*n)->is_named(p->name()) )
                throw InvalidParameter(p->kind()+" '"+p->name()+"' already exists");
        }
    }
    
    //std::cerr << this << "->deposit(" << p->kind() << ", " << p->name() << ") " << idx << std::endl;

    vec.push_back(p);
    p->index(cnt);
}


/**
 The size of the array will be reduced by one
 */
int PropertyList::remove(Property * p)
{
    int idx = 0;
    for ( iterator n = vec.begin(); n != vec.end(); ++n, ++idx )
    {
        if ( *n == p )
        {
            vec.erase(n);
            return idx;
        }
    }
    return -1;
}


unsigned int PropertyList::number_of(const std::string& kd) const
{
    unsigned int res = 0;
    
    for ( const_iterator n = vec.begin(); n != vec.end(); ++n )
        res += ( (*n)->kind() == kd );
    
    return res;
}


Property * PropertyList::operator[] (const unsigned int n) const
{
    if ( n >= vec.size() )
    {
        std::ostringstream oss;
        oss << "out-of-range index " << n << " ( list-size = " << vec.size() << " )";
        throw InvalidSyntax(oss.str());
    }
    return vec[n];
}

//-------------------------------------------------------------------------------

void PropertyList::for_each(void func(Property *)) const
{
    //std::cerr << "Running function for "<<vec.size()<<" properties"<<std::endl;
    for ( const_iterator n = vec.begin(); n != vec.end(); ++n )
        func(*n);
}

void PropertyList::complete(SimulProp const* sp)
{
    for ( const_iterator n = vec.begin(); n != vec.end(); ++n )
        (*n)->complete(sp, this);
}

int PropertyList::find_index(const Property * p) const
{
    int idx = 0;
    for ( const_iterator n = vec.begin(); n != vec.end(); ++n, ++idx )
        if ( *n == p ) return idx;
    return -1;
}


//-------------------------------------------------------------------------------
#pragma mark -

/** 
 returns the first match
 */
Property * PropertyList::find(const std::string& nm) const
{
    //std::clog << this << "->find(" << nm << ")" << std::endl;
    
    for ( const_iterator n = vec.begin(); n != vec.end(); ++n )
        if ( (*n)->is_named(nm) )
            return *n;

    return 0;
}

/**
 returns the first match
 */
Property * PropertyList::find(const std::string& kd, const std::string& nm) const
{
    //std::clog << this << "->find(" << kd << ", " << nm << ")" << std::endl;

    for ( const_iterator n = vec.begin(); n != vec.end(); ++n )
    {
        if ( (*n)->kind()==kd  &&  (*n)->is_named(nm) )
            return *n;
    }
    
    return 0;
}


Property * PropertyList::find(const std::string& kd, const int idx) const
{
    //std::clog << this << "->find(" << kd << ", " << idx << ")" << std::endl;

    for ( const_iterator n = vec.begin(); n != vec.end(); ++n )
        if ( (*n)->kind()==kd  &&  (*n)->index()==idx )
            return *n;
    
    return 0;
}



Property * PropertyList::find_or_die(std::string const& knd, std::string const& nm) const
{
    Property * res = find(knd, nm);
    
    if ( !res )
    {
        std::ostringstream oss;
        oss << "Unknown " << knd << " `" << nm << "'\n";
        write_names(oss, PREF);
        throw InvalidSyntax(oss.str());
    }
    
    return res;
}


Property * PropertyList::find_or_die(std::string const& knd, const unsigned idx) const
{
    Property * res = find(knd, idx);
    
    if ( !res )
    {
        std::ostringstream oss;
        oss << "Unknown " << knd << "(" << idx << ")\n";
        write_names(oss, PREF);
        throw InvalidSyntax(oss.str());
    }
    
    return res;
}


PropertyList PropertyList::find_all(const std::string& kd) const
{
    PropertyList list;
    //std::cerr << this << "->find_all(" << kd << ") " << std::endl;

    for ( const_iterator n = vec.begin(); n != vec.end(); ++n )
    {
        if ( (*n)->kind() == kd )
            list.vec.push_back(*n);
    }
    
    return list;
}


PropertyList PropertyList::find_all(const std::string& kd1, const std::string& kd2) const
{
    PropertyList list;
    //std::cerr << this << "->find_all(" << kd << ") " << std::endl;
    
    for ( const_iterator n = vec.begin(); n != vec.end(); ++n )
    {
        if ( (*n)->kind() == kd1 || (*n)->kind() == kd2 )
            list.vec.push_back(*n);
    }
    
    return list;
}


PropertyList PropertyList::find_all(const std::string& kd1, const std::string& kd2, const std::string& kd3) const
{
    PropertyList list;
    //std::cerr << this << "->find_all(" << kd << ") " << std::endl;
    
    for ( const_iterator n = vec.begin(); n != vec.end(); ++n )
    {
        if ( (*n)->kind() == kd1 || (*n)->kind() == kd2 || (*n)->kind() == kd3 )
            list.vec.push_back(*n);
    }
    
    return list;
}


Property* PropertyList::find_next(const std::string& kd, Property * p) const
{
    //std::cerr << this << "->find_next(" << kd << ") " << std::endl;
    bool found = ( p == 0 );
    
    for ( const_iterator n = vec.begin(); n != vec.end(); ++n )
    {
        if ( (*n)->kind() == kd )
        {
            if ( found )
                return *n;
            found = ( *n == p );
        }
    }
    
    if ( ! found ) 
        return 0;
    
    for ( const_iterator n = vec.begin(); n != vec.end(); ++n )
    {
        if ( (*n)->kind() == kd )
            return *n;
    }
    
    return 0;
}


PropertyList PropertyList::find_all_except(const std::string& kd) const
{
    PropertyList list;
    //std::cerr << this << "->all_expect(" << kd << ") " << std::endl;
    
    for ( const_iterator n = vec.begin(); n != vec.end(); ++n )
    {
        if ( (*n)->kind() != kd )
            list.vec.push_back(*n);
    }
    
    return list;
}


//-------------------------------------------------------------------------------
#pragma mark -

void PropertyList::write_names(std::ostream & os, std::string const& pf) const
{
    os << pf << "Known properties:" << std::endl;

    int idx = 0;
    for ( const_iterator n = vec.begin(); n != vec.end(); ++n, ++idx )
    {
        os << pf << idx << " : ";
        if ( *n )
            os << std::setw(16) << (*n)->kind() << "  `"<< (*n)->name() << "'";
        else
            os << "void";
        os << std::endl;
    }
}

/**
 The values identical to the default settings are skipped if prune==1
 */
void PropertyList::write(std::ostream & os, const bool prune) const
{
    const_iterator n = vec.begin();
    
    if ( n != vec.end() )
    {
        (*n)->write(os, prune);
    
        while ( ++n != vec.end() )
        {
            os << std::endl;
            (*n)->write(os, prune);
        }
    }
}

