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
        Property * p = simul.properties.find("bead", idx, true);
        Solid * s = new Solid(static_cast<SolidProp*>(p));
        return s;        
    }
    return 0;
}


/**
 Please, see Solid::build
 */
Object * SolidSet::newObject(const std::string& kd, const std::string& nm, Glossary& opt)
{
    Solid * obj = 0;
    if ( kd == kind() )
    {
        // as Solid use a BeadProp, we search for "bead":
        Property * p = simul.properties.find("bead", nm, true);
        obj = new Solid(static_cast<SolidProp*>(p));
        obj->build(opt);
    }
    return obj;
}

/**
 @addtogroup NewSolid
 @{
 <h3> How to add Single </h3>
 
 Different methods can be used to attach Single to the points of a Solid:
 
 @code
 new sphere NAME
 {
   attach      = SINGLE_SPEC [, SINGLE_SPEC] ...
   attach_each = SINGLE_SPEC [, SINGLE_SPEC] ...
 }
 @endcode
 
 Where `SINGLE_SPEC` is 'NUMBER SINGLE_NAME' or simply 'SINGLE_NAME',
 for example `grafted` or `10 grafted`, if this is the name of a Single.
 With `attach`, the Singles are distributed randomly on the points.
 With `attach_each`, the specification is repeated for each point, and each point thus receive the same number of single.
 
 Method 3 - Specify Singles for individual point:
 
 @code
 new solid NAME
 {
   point0 = POSITION, RADIUS, SINGLE_SPEC, ...
   point1 = POSITION, RADIUS, SINGLE_SPEC, ...
   point2 = POSITION, RADIUS, SINGLE_SPEC, ...
   ...
 }
 @endcode
 
 POSITION can be a Vector, or the strings 'inside', 'surface', 'equator'.
 Several SINGLE_SPEC can be specified.
 
 @}
 */
ObjectList SolidSet::newObjects(const std::string& kd, const std::string& nm, Glossary& opt)
{
    ObjectList res;
    Object * obj = newObject(kd, nm, opt);
    
    if ( obj )
    {
        res.push_back(obj);
        Solid * mec = static_cast<Solid*>(obj);
        
        std::string spc;
        unsigned inx = 0;
        
        // Attach Single, one per point:
        while ( opt.set(spc, "attach_each", inx) )
        {
            unsigned ns = 1;
            Tokenizer::split_integer(spc, ns);
            SingleProp * sp = simul.findSingleProp(spc);
            
            // attach one Single at each point:
            for ( unsigned pp = 0; pp < mec->nbPoints(); ++pp )
            {
                for ( unsigned u = 0; u < ns; ++u )
                    res.push_back(sp->newWrist(PointExact(mec, pp)));
            }
            ++inx;
        }
        
        inx = 0;
        // Specify Singles to be distributed on the points:
        while ( opt.set(spc, "attach", inx) )
        {
            unsigned ns = 1;
            Tokenizer::split_integer(spc, ns);
            SingleProp * sp = simul.findSingleProp(spc);
            
            // attach one Single at each point:
            for ( unsigned u = 0; u < ns; ++u )
            {
                unsigned pp = RNG.pint_exc(mec->nbPoints());
                res.push_back(sp->newWrist(PointExact(mec, pp)));
            }
            ++inx;
        }
        
        // Specify Singles differently for each point:
        for ( unsigned ii = 0; ii < mec->nbPoints(); ++ii )
        {
            std::string var = "point" + sMath::repr(ii);
            unsigned inx = 2;
            while ( opt.set(spc, var, inx) )
            {
                unsigned ns = 1;
                Tokenizer::split_integer(spc, ns);
                SingleProp * sp = simul.findSingleProp(spc);
                for ( unsigned u = 0; u < ns; ++u )
                    res.push_back(sp->newWrist(PointExact(mec, ii)));
                ++inx;
            }
        }
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

