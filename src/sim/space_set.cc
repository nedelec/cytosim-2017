// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "space_set.h"
#include "space_prop.h"
#include "iowrapper.h"
#include "glossary.h"
#include "simul.h"
#include "space.h"
#include "modulo.h"

//---------------------------- GLOBAL VARIABLE ---------------------------------

Modulo * modulo = 0;

//------------------------------------------------------------------------------

Property * SpaceSet::newProperty(const std::string& kd,const std::string& nm, Glossary&) const
{
    if ( kd == kind() )
        return new SpaceProp(nm);
    return 0;
}

//------------------------------------------------------------------------------
void SpaceSet::step()
{
    for ( Space * sp = first(); sp; sp=sp->next() )
        sp->step();
}


//------------------------------------------------------------------------------
void SpaceSet::erase()
{
    ObjectSet::erase();
    
    // simul has lost its current Space:
    simul.space(0);
}

/**
 This also changes the Simul current Space:
*/
void SpaceSet::add(Object * obj)
{
    //std::cerr << "SpaceSet::add " << obj << std::endl;
    ObjectSet::add(obj);
    
    if ( !simul.space() )
        simul.space(static_cast<Space*>(obj));
}

/**
 If the simul' current Space is deleted,
 the simul space is set to the second Space in the list
 */
void SpaceSet::remove(Object * obj)
{
    //std::cerr << "SpaceSet::remove " << obj << std::endl;
    if ( obj == simul.space() )
    {
        ObjectSet::remove(obj);
        simul.space(static_cast<Space*>(first()));
    }
    else
        ObjectSet::remove(obj);
}

//------------------------------------------------------------------------------
Object * SpaceSet::newObjectT(const Tag tag, int idx)
{
    Space * obj = 0;
    if ( tag == Space::TAG )
    {
        Property * p = simul.properties.find_or_die(kind(), idx);
        SpaceProp * sp = static_cast<SpaceProp*>(p);
        obj = sp->newSpace();
    }
    return obj;
}

/**
 The dimensions of a Space can be specified when it is created
 @code
 new space cell
 {
    dimension = 3 4
 }
 @endcode
 */
ObjectList SpaceSet::newObjects(const std::string& kd, const std::string& nm, Glossary& opt)
{
    Space * obj = 0;
    if ( kd == kind() )
    {
        Property * p = simul.properties.find_or_die(kd, nm);
        SpaceProp * sp = static_cast<SpaceProp*>(p);

        obj = sp->newSpace();
        
        std::string dim;
        if ( opt.set(dim, "dimensions") )
            obj->readLengths(dim);
    }
    
    ObjectList res;
    if ( obj )
        res.push_back(obj);
    
    return res;
}
