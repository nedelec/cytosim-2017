// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "sphere_set.h"
#include "sphere_prop.h"
#include "iowrapper.h"
#include "glossary.h"
#include "wrist.h"
#include "simul.h"


//------------------------------------------------------------------------------
void SphereSet::erase()
{
    ObjectSet::erase();
}


//------------------------------------------------------------------------------
void SphereSet::add(Object * obj)
{
    assert_true(obj->tag() == Sphere::TAG);
    ObjectSet::add(obj);
}

void SphereSet::remove(Object * obj)
{
    ObjectSet::remove(obj);
    assert_true(obj->tag() == Sphere::TAG);
    simul.singles.removeWrists(obj);
}

//------------------------------------------------------------------------------

Property* SphereSet::newProperty(const std::string& kd, const std::string& nm, Glossary&) const
{
    if ( kd == kind() )
        return new SphereProp(nm);
    return 0;
}


//------------------------------------------------------------------------------
Object * SphereSet::newObjectT(const Tag tag, int idx)
{
    Sphere * obj = 0;
    if ( tag == Sphere::TAG )
    {
        Property * p = simul.properties.find_or_die(kind(), idx);
        obj = new Sphere(static_cast<SphereProp*>(p));
    }
    return obj;
}

/**
 @copydetails Sphere::build
 */
ObjectList SphereSet::newObjects(const std::string& kind, const std::string& name, Glossary& opt)
{
    Sphere * obj = 0;
    if ( kind == "sphere" )
    {
        Property * p = simul.properties.find_or_die(kind, name);
        obj = new Sphere(static_cast<SphereProp*>(p), 1);
        
        // set radius if provided as argument
        real rad = -1;
        if ( !opt.set(rad, "radius" ) || rad <= 0 )
            throw InvalidParameter("sphere:radius should be specified and > 0");
        
        // possibly add some variability
        real var = 0;
        if ( opt.set(var, "radius", 1) )
        {
            real drad;
            do
                drad = var * RNG.gauss();
            while ( rad + drad < REAL_EPSILON );
            rad += drad;
        }
        
        obj = new Sphere(static_cast<SphereProp*>(p), rad);
    }
    
    ObjectList res;
    if ( obj )
    {
        res.push_back(obj);
        res.append(obj->build(opt, simul));
    }
    return res;
}

//------------------------------------------------------------------------------
void SphereSet::foldPosition(const Modulo * s) const
{
    for ( Sphere * o=SphereSet::first(); o; o=o->next() )
        o->foldPosition(s);
}

