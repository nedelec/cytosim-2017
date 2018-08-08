// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "assert_macro.h"
#include "nucleus.h"
#include "exceptions.h"
#include "rotation.h"
#include "sphere_prop.h"
#include "bundle_prop.h"
#include "point_exact.h"
#include "fiber_prop.h"
#include "glossary.h"
#include "bundle.h"
#include "meca.h"

extern Random RNG;


void Nucleus::step()
{
}


void Nucleus::setInteractions(Meca & meca) const
{
    Sphere * sph = sphere();
    
    if ( sph == 0 )
        return;
    
    unsigned int nix = sphere()->nbPoints() - Sphere::nbRefPts;
    
    for ( unsigned int ix = 0; ix < nix; ++ix )
    {
        const Fiber * fib = fiber(ix);
        if ( fib )
            meca.interLink( PointExact(sph, ix+Sphere::nbRefPts),
                            fib->exactEnd(prop->focus),
                            prop->stiffness );
    }
}



//------------------------------------------------------------------------------
ObjectList Nucleus::build(Glossary& opt, Simul& simul)
{
    assert_true(prop);
    ObjectList res;
    
    real rad = -1;
    if ( !opt.set(rad, "radius" ) )
        throw InvalidParameter("nucleus:radius should be specified");
   
    Sphere * sph = new Sphere(prop->sphere_prop, rad);
    grasp(sph, 0);
    res.push_back(sph);
    
    // get the center of the sphere
    Vector c = sph->posPoint(0);
    
    if ( prop->nb_fibers > 0 )
    {        
        // create points and clamps and add fiber attached to them
        for ( unsigned int ii = 0; ii < prop->nb_fibers; ++ii )
        {
            Fiber * fib = prop->fiber_prop->newFiber(opt);
            Vector pos = c + Vector::randUnit(sph->radius());
            Vector dir = Vector::randUnit();
            fib->setStraight(pos, dir, prop->focus);
            sph->addPoint(pos);
            res.push_back(fib);
            grasp(fib);
        }
    }
    
    
    if ( prop->nb_bundles > 0 )
    {
        Rotation rotation;
        // add bundles        
        const real len = 0.5 * prop->bundle_prop->overlap;
        const real rad = sph->radius();
        for ( unsigned int ii=0; ii < prop->nb_bundles; ++ii  )
        {
            rotation = Rotation::randomRotation(RNG);
            //a random position on the sphere:
            Vector pos = rotation * Vector(0,sph->radius(),0);
            //a random direction, perpendicular to pos:
            Vector dir = rotation * Vector(1,0,0);
            
            Bundle * bu = new Bundle(prop->bundle_prop);
            ObjectList bu_objs = bu->build(opt, simul);
            res.append(bu_objs);
            res.push_back(bu);
                        
            //position the bundle correctly:
            bu->rotate(rotation);
            bu->translate(pos);
            
            sph->addPoint( c + (pos-len*dir).normalized(rad) );
            grasp(bu->organized(0));
            
            sph->addPoint( c + (pos+len*dir).normalized(rad) );
            grasp(bu->organized(1));
        }        
    }
    
    return res;
}

//------------------------------------------------------------------------------

unsigned int Nucleus::nbLinks() const
{
    if ( sphere() )
        return sphere()->nbPoints() - Sphere::nbRefPts;
    else
        return 0;
}

Vector Nucleus::posLink1(const unsigned int ii) const
{
    assert_true( sphere() );
    return sphere()->posPoint(ii+Sphere::nbRefPts);
}


Vector Nucleus::posLink2(const unsigned int ii) const
{
    if ( organized(ii+1) == 0 )
        return posLink1(ii);
    
    return static_cast<Fiber*>(organized(ii+1))->posEnd(prop->focus);
}

