// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "solid_set.h"
#include "solid_prop.h"
#include "iowrapper.h"
#include "glossary.h"
#include "simul.h"
#include "wrist.h"



void SolidSet::step()
{
    for ( Solid * o = first(); o; o=o->next() )
        o->step();
}



//------------------------------------------------------------------------------

Property* SolidSet::newProperty(const std::string& kd, const std::string& nm, Glossary&) const
{
    if ( kd == kind() )
        return new SolidProp(nm);
    return 0;
}


Object * SolidSet::newObjectT(const Tag tag, int idx)
{
    if ( tag == Solid::TAG )
    {
        // as Solid use a BeadProp, we search for "bead":
        Property * p = simul.properties.find_or_die("bead", idx);
        Solid * s = new Solid(static_cast<SolidProp*>(p));
        return s;        
    }
    return 0;
}


/**
 @ref Solid::build
 */
ObjectList SolidSet::newObjects(const std::string& kd, const std::string& nm, Glossary& opt)
{
    Solid * obj = 0;
    
    if ( kd == "solid" )
    {
        // as Solid use a BeadProp, we search for "bead":
        Property * p = simul.properties.find_or_die("bead", nm);
        obj = new Solid(static_cast<SolidProp*>(p));
    }
    
    ObjectList res;
    if ( obj )
    {
        res.push_back(obj);
        res.append(obj->build(opt, simul));
        obj->fixShape();
    }
    return res;
}


//------------------------------------------------------------------------------

void SolidSet::erase()
{
    ObjectSet::erase();
}

void SolidSet::add(Object * obj)
{
    assert_true(obj->tag() == Solid::TAG);
    ObjectSet::add(obj);
}


void SolidSet::remove(Object * obj)
{
    ObjectSet::remove(obj);
    assert_true(obj->tag() == Solid::TAG);
    simul.singles.removeWrists(obj);
}


void SolidSet::foldPosition(const Modulo * s) const
{
    for ( Solid * o=first(); o; o=o->next() )
        o->foldPosition(s);
}

