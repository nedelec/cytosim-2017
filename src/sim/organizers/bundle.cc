// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "assert_macro.h"
#include "bundle.h"
#include "exceptions.h"
#include "point_exact.h"
#include "point_interpolated.h"
#include "fiber_prop.h"
#include "glossary.h"
#include "simul.h"
#include "meca.h"

//------------------------------------------------------------------------------
void Bundle::step()
{
    if ( prop->nucleate )
    {
        for ( unsigned int ii = 0; ii < prop->nb_fibers; ++ii )
        {
            if ( 0 == organized(ii) )
            {
                Fiber* fib = prop->fiber_prop->newFiber();
                fib->setLength(prop->overlap, prop->focus);
                ///\todo: we should orient the new Fiber in bundle direction
                objset()->simul.add(fib);
                grasp(fib, ii);
            }
        }
    }
}


//------------------------------------------------------------------------------

/*
 Parallel connection
*/
void Bundle::setParallel(Meca & meca, Fiber * mt1, Fiber * mt2) const
{
    real const& stiff = prop->stiffness;
    real const& dis = prop->overlap;
    
    meca.interLink(mt1->interpolate(dis, prop->focus), mt2->interpolate(dis, prop->focus), stiff);
    meca.interLink(mt1->exactEnd(prop->focus), mt2->exactEnd(prop->focus), stiff);
}


/**
 Antiparallel connection 
*/
void Bundle::setAntiparallel(Meca & meca, Fiber * mt1, Fiber * mt2) const
{
    real const& stiff = prop->stiffness;
    real const& dis = prop->overlap;

    if ( dis < REAL_EPSILON )
        meca.interLink(mt1->exactEnd(prop->focus), mt2->exactEnd(prop->focus), stiff+stiff);
    else {
        meca.interLink(mt1->interpolate(dis, prop->focus), mt2->exactEnd(prop->focus), stiff);
        meca.interLink(mt2->interpolate(dis, prop->focus), mt1->exactEnd(prop->focus), stiff);
    }
}


/**
 Connect the fibers near their minus-ends, to form a ring:
 1. connect fibers with their neighbors,
 2. close the ring by connecting the first to the last.
 */
void Bundle::setInteractions(Meca & meca) const
{
    assert_true( linked() );
    assert_true( prop->nb_fibers == nbOrganized() );
    
    Fiber * mt0 = static_cast<Fiber*>(organized(0));
    Fiber * mt1 = mt0, * mt2 = 0;
    
    for ( unsigned int ii = 1 ; ii < nbOrganized(); ++ii )
    {
        mt2 = static_cast<Fiber*>(organized(ii));
        setAntiparallel(meca, mt1, mt2);
        mt1 = mt2;
    }
    
    // connect first and last fiber:
    mt1 = mt0;
    
    if ( mt1 && mt2 )
    {    
        assert_true( mt2 == organized(nbOrganized()-1) );
        
        if ( nbOrganized() % 2 == 0 )
            setAntiparallel(meca, mt1, mt2);
        else
            setParallel(meca, mt1, mt2);
    }
}


//------------------------------------------------------------------------------
/**
 It is possible to specify multiple lengths for fibers:
 @code
 new bundle bundle
 {
    length = 3.0, 4.2
 }
 @endcode
 */
ObjectList Bundle::build(Glossary& opt, Simul&)
{
    assert_true(prop);
    Vector pos(0,0,0);
    Vector dir(1,0,0);
    
    for ( unsigned int indx = 0; indx < prop->nb_fibers; ++indx )
    {
        int sign = ( indx % 2 ) ? -1 : +1;
        Vector sdir = sign * dir;
        Fiber* fib = prop->fiber_prop->newFiber(opt);
        real len = 1;
        if ( opt.set(len, "length", indx) )
            fib->setStraight(pos-sdir*0.5*prop->overlap, sdir, len, prop->focus);
        else           
            fib->setStraight(pos-sdir*0.5*prop->overlap, sdir, prop->focus);
        grasp(fib, indx);
    }
    
    ObjectList res;
    for ( unsigned int ii = 0; ii < nbOrganized(); ++ii )
        res.push_back(organized(ii));
    return res;
}


Bundle::~Bundle()
{
    prop = 0;
}


