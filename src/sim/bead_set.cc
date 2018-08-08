// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "bead_set.h"
#include "bead_prop.h"
#include "iowrapper.h"
#include "glossary.h"
#include "single_prop.h"
#include "wrist.h"
#include "simul.h"

//------------------------------------------------------------------------------
void BeadSet::erase()
{
    ObjectSet::erase();
}

//------------------------------------------------------------------------------
Property* BeadSet::newProperty(const std::string& kd, const std::string& nm, Glossary&) const
{
    if ( kd == kind() )
        return new BeadProp(nm);
    return 0;
}

//------------------------------------------------------------------------------
Object * BeadSet::newObjectT(const Tag tag, int idx)
{
    assert_true( tag == Bead::TAG );
    Property * p = simul.properties.find_or_die(kind(), idx);
    return new Bead(static_cast<BeadProp*>(p), Vector(0,0,0), 0);
}

/**
 @defgroup NewBead How to create a Bead
 @ingroup NewObject

 By definition, a Bead has one point, but you can vary the radius of the Bead:

 @code
 new bead NAME
 {
   radius = REAL
 }
 @endcode

 <h3> Singles can be attached at the center of the Bead </h3>

 @code
 new bead NAME
 {
   radius = REAL
   singles = SINGLE_SPEC [, SINGLE_SPEC] ...
 }
 @endcode
 
 `SINGLE_SPEC` is an optional number (1 by default) followed by the name of the Single,
 for example `grafted` or `10 grafted`, if this is the name of a Single.
 
 */

ObjectList BeadSet::newObjects(const std::string& kd, const std::string& nm, Glossary& opt)
{
    ObjectList res;
    Object * obj = 0;
    if ( kd == kind() )
    {
        real rad = -1;
        opt.set(rad, "radius");
        
        if ( rad < 0 )
            throw InvalidParameter("bead:radius must be specified and > 0");
        
        Property * p = simul.properties.find_or_die(kind(), nm);
        obj = new Bead(static_cast<BeadProp*>(p), Vector(0,0,0), rad);
 
        res.push_back(obj);
        Bead * bid = static_cast<Bead*>(obj);
        
        std::string sn;
        unsigned inx = 0;

        /// attach different kinds of SINGLE
        while ( opt.set(sn, "attach", inx) || opt.set(sn, "single", inx) )
        {
            unsigned cnt = 1;
            Tokenizer::split_integer(sn, cnt);
            Property * p = simul.properties.find_or_die("single", sn);
            SingleProp * sp = static_cast<SingleProp*>(p);
            for ( unsigned s = 0; s < cnt; ++s )
                res.push_back(sp->newWrist(bid, 0));
            ++inx;
        }
    }
    return res;
}

//------------------------------------------------------------------------------
void BeadSet::add(Object * obj)
{
    assert_true(obj->tag() == Bead::TAG);
    ObjectSet::add(obj);
}

void BeadSet::remove(Object * obj)
{
    ObjectSet::remove(obj);
    simul.singles.removeWrists(obj);
    assert_true(obj->tag() == Bead::TAG);
}

//------------------------------------------------------------------------------
void BeadSet::foldPosition(const Modulo * s) const
{
    for ( Bead * o=first(); o; o=o->next() )
        o->foldPosition(s);
}
