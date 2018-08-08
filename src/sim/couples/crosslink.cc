// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "crosslink.h"
#include "crosslink_prop.h"
#include "exceptions.h"
#include "random.h"
#include "space.h"

extern Random RNG;

//------------------------------------------------------------------------------
Crosslink::Crosslink(CrosslinkProp const* p, Vector const& w)
: Couple(p, w), prop(p)
{
}


Crosslink::~Crosslink()
{
    prop = 0;
}


//------------------------------------------------------------------------------
/**
 This will:
 - prevent binding twice to the same fiber, if ( prop:stiff = true )
 - check prop:specificity with respect to the configuration of the two fibers
 */
bool Crosslink::allowAttachment(const FiberBinder & fb)
{
    const Hand * other = attachedHand();
    
    if ( other == 0 )
        return true;
    
    if ( prop->stiff )
    {
        if ( other->fiber() == fb.fiber()
            && fabs(fb.abscissa()-other->abscissa()) < 2*fb.fiber()->segmentation() )
            return false;
    }
    
    switch( prop->specificity )
    {
        case CoupleProp::BIND_ALWAYS:
            break;

        case CoupleProp::BIND_PARALLEL:
            if ( fb.dirFiber() * other->dirFiber() < 0 )
                return false;
            break;

        case CoupleProp::BIND_ANTIPARALLEL:
            if ( fb.dirFiber() * other->dirFiber() > 0 )
                return false;
            break;

        default:
            throw InvalidParameter("unknown crosslink:specificity");
    }
    return true;
}


void Crosslink::stepFF(const FiberGrid& grid)
{
    assert_true( !attached1() && !attached2() );
    
    // diffusion:
    cPos.addRand( prop->diffusion_dt );
    
    // confinement:
    if ( prop->confine == CONFINE_INSIDE )
    {
        const Space* spc = prop->confine_space_ptr;
        assert_true(spc);
        if ( ! spc->inside(cPos) )
            spc->bounce(cPos);
    }
    else if ( prop->confine == CONFINE_SURFACE )
    {
        const Space* spc = prop->confine_space_ptr;
        assert_true(spc);
        spc->project(cPos);        
    }    
    
    // activity (attachment):
    if ( prop->trans_activated )
    {
        cHand1->stepFree(grid, cPos);
    }
    else
    {
        cHand1->stepFree(grid, cPos);
        cHand2->stepFree(grid, cPos);
    }
}

