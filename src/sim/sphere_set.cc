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
 @defgroup NewSphere How to create a Sphere
 @ingroup NewObject
 
 Specify radius and number of surface points:
 @code
 new sphere NAME
 {
   radius = REAL
   nb_points = INTEGER
 }
 @endcode
 
 */
Object * SphereSet::newObject(const std::string& kd, const std::string& nm, Glossary & opt)
{
    Sphere * obj = 0;
    if ( kd == kind() )
    {
        Property * p = simul.properties.find_or_die(kd, nm);
        obj = new Sphere(static_cast<SphereProp*>(p), 1);
        assert_true(obj);
        
        // set radius if provided as argument
        real rad;
        if ( opt.set(rad, "radius") )
            obj->resize(rad);
    }
    return obj;
}


/**
 @addtogroup NewSphere 
 @{

 <h3> How to add Single </h3>
 
 Different methods can be used to attach Single to the surface points of a Sphere.

 Method 1 - Specify new points with one Single on each point:
 
 @code
 new sphere NAME
 {
   points0 = INTEGER, surface, SINGLE_SPEC, ...
   points1 = INTEGER, surface, SINGLE_SPEC, ...
   ...
 }
 @endcode
 
 The first line `points0` will add INTEGER points, with specified Single.
 `SINGLE_SPEC` is an optional number (1 by default) followed by the name of the Single,
 for example `10 grafted`, if `grafted` is the name of a Single.
 Several SINGLE_SPEC can be specified, separated by commas.

 
 Moreover, Singles can be added globally on all the points:
 
 @code
 new sphere NAME
 {
    attach      = SINGLE_SPEC [, SINGLE_SPEC] ...
    attach_each = SINGLE_SPEC [, SINGLE_SPEC] ...
 }
 @endcode
 
 With `attach`, the Singles are distributed randomly on the points.
 With `attach_each`, the specification is repeated for each point, and each point thus receive the same number of single.
 
 @}
 */
ObjectList SphereSet::newObjects(const std::string& kind, const std::string& prop, Glossary& opt)
{
    ObjectList res;
    Object * obj = newObject(kind, prop, opt);
    
    if ( obj )
    {
        std::string str, spc;
        res.push_back(obj);
        Sphere * sph = static_cast<Sphere*>(obj);
        
        Vector cen = sph->posPoint(0);
        // add points individually
        unsigned inp = 0;
        std::string var = "point0";
        while ( opt.has_key(var) )
        {
            Vector vec(1, 0, 0);
            if ( opt.is_alpha(var) )
            {
                opt.set(str, var);
                if ( str == "surface" )
                    vec = Vector::randUnit(sph->radius());
                else
                    throw InvalidParameter("sphere::"+var+"[0] should be 'surface'");
            }
            else
            {
                if ( !opt.set(vec, var) )
                    throw InvalidParameter("expected a vector in sphere::"+var+"[0]");
            }
            
            unsigned pp = sph->addSurfacePoint(vec);
            
            unsigned inx = 1;
            while ( opt.set(spc, var, inx) )
            {
                unsigned ns = 1;
                Tokenizer::split_integer(spc, ns);
                SingleProp * sp = simul.findSingleProp(spc);
                for ( unsigned s = 0; s < ns; ++s )
                    res.push_back(sp->newWrist(sph, pp));
                ++inx;
            }
            
            var = "point" + sMath::repr(++inp);
        }
        
        
        // add points in packets
        inp = 0;
        var = "points";
        while ( opt.has_key(var) )
        {
            unsigned nbp = 0;
            
            if ( ! opt.set(nbp, var) )
                throw InvalidParameter("sphere::"+var+" must be an integer (the number of points)");
            
            if ( ! opt.set(str, var, 1) || str != "surface" )
                throw InvalidParameter("sphere::"+var+" should be 'surface'");
            
            unsigned pp = 0;
            for ( unsigned u = 0; u < nbp; ++u )
                pp = sph->addSurfacePoint(Vector::randUnit(sph->radius()));
            
            unsigned inx = 2;
            while ( opt.set(spc, var, inx) )
            {
                unsigned ns = 1;
                Tokenizer::split_integer(spc, ns);
                SingleProp * sp = simul.findSingleProp(spc);
                for ( unsigned s = 0; s < ns; ++s )
                    res.push_back(sp->newWrist(sph, pp-RNG.pint_exc(nbp)));
                ++inx;
            }
            
            var = "points" + sMath::repr(++inp);
        }
        
#ifdef BACKWARD_COMPATIBILITY
        // add points on the surface to reach 'nb_points'
        if ( opt.set(inp, "nb_points") )
        {
            while ( sph->nbPoints() < inp + Sphere::nbRefPts )
                sph->addSurfacePoint(Vector::randUnit(sph->radius()));
        }
#endif
        
        // add Single on every point
        inp = 0;
        if ( opt.set(spc, "attach_each", inp) )
        {
            unsigned ns = 1;
            Tokenizer::split_integer(spc, ns);
            SingleProp * sp = simul.findSingleProp(spc);
            for ( unsigned s = 0; s < ns; ++s )
                for ( unsigned p = sph->nbRefPts; p < sph->nbPoints(); ++p )
                    res.push_back(sp->newWrist(sph, p));
            ++inp;
        }
        
        // distribute Single over all surface points
        inp = 0;
        while ( opt.set(spc, "attach", inp) )
        {
            unsigned nbp = sph->nbPoints() - sph->nbRefPts;
            if ( nbp < 1 )
                throw InvalidParameter("Cannot add Singles without surface points");
            
            unsigned ns = 1;
            Tokenizer::split_integer(spc, ns);
            SingleProp * sp = simul.findSingleProp(spc);
            for ( unsigned s = 0; s < ns; ++s )
            {
                unsigned p = RNG.pint_exc(nbp) + sph->nbRefPts;
                res.push_back(sp->newWrist(sph, p));
            }
            ++inp;
        }
    }
    return res;
}

//------------------------------------------------------------------------------
void SphereSet::foldPosition(const Modulo * s) const
{
    for ( Sphere * o=SphereSet::first(); o; o=o->next() )
        o->foldPosition(s);
}

