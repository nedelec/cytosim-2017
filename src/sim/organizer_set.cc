// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "glossary.h"
#include "point_exact.h"
#include "organizer_set.h"
#include "nucleus.h"
#include "bundle.h"
#include "aster.h"
#include "fake.h"
#include "solid.h"
#include "simul.h"

//------------------------------------------------------------------------------
void OrganizerSet::step()
{
    for ( Organizer * as=first(); as ; as=as->next() )
        as->step();
}

//------------------------------------------------------------------------------

Property* OrganizerSet::newProperty(const std::string& kd, const std::string& nm, Glossary&) const
{
    if ( kd == "aster" )   return new AsterProp(nm);
    if ( kd == "bundle" )  return new BundleProp(nm);
    if ( kd == "nucleus" ) return new NucleusProp(nm);
    if ( kd == "fake" )    return new FakeProp(nm);
    return 0;
}

//------------------------------------------------------------------------------
Object * OrganizerSet::newObjectT(const Tag tag, int idx)
{
    if ( tag == Aster::TAG )
    {
        Property * p = simul.properties.find("aster", idx, true);
        return new Aster(static_cast<AsterProp*>(p));
    }
    
    if ( tag == Bundle::TAG )
    {
        Property * p = simul.properties.find("bundle", idx, true);
        return new Bundle(static_cast<BundleProp*>(p));
    }
    
    if ( tag == Nucleus::TAG )
    {
        Property * p = simul.properties.find("nucleus", idx, true);
        return new Nucleus(static_cast<NucleusProp*>(p));
    }
    
    if ( tag == Fake::TAG )
    {
        Property * p = simul.properties.find("fake", idx, true);
        return new Fake(static_cast<FakeProp*>(p));
    }
    
    throw InvalidIO("unknown Organizer TAG `"+std::string(1,tag)+"'");
    return 0;
}


Object * OrganizerSet::newObject(const std::string& kd, const std::string& nm, Glossary&)
{
    Property * p = simul.properties.find(kd, nm, true);
    Organizer * obj = 0;
    
    if ( kd == "aster" )
    {
        obj = new Aster(static_cast<AsterProp*>(p));
    }
    
    if ( kd == "bundle" )
    {
        obj = new Bundle(static_cast<BundleProp*>(p));
    }
    
    if ( kd == "nucleus" )
    {
        obj = new Nucleus(static_cast<NucleusProp*>(p));
    }

    if ( kd == "fake" )
    {
        obj = new Fake(static_cast<FakeProp*>(p));
    }
    
    return obj;
}


ObjectList OrganizerSet::newObjects(const std::string& kind, const std::string& prop, Glossary& opt)
{
    ObjectList res;
    Object * obj = newObject(kind, prop, opt);
    
    if ( obj )
    {
        Organizer * org = static_cast<Organizer*>(obj);
        res = org->build(opt);
        // add the organizer last in the list:
        res.push_back(obj);
    }
    
    return res;
}

//------------------------------------------------------------------------------
void OrganizerSet::add(Object * obj)
{
    ObjectSet::add(obj);
}

//------------------------------------------------------------------------------
Aster * OrganizerSet::findAster(const Number n) const
{
    Object * obj = find(n);
    if ( obj  &&  obj->tag() == Aster::TAG )
        return static_cast<Aster*>(obj);
    return 0;
}


//------------------------------------------------------------------------------
void OrganizerSet::foldPosition(const Modulo * s) const
{
    for ( Organizer * o=first(); o; o=o->next() )
        o->foldPosition(s);
}



