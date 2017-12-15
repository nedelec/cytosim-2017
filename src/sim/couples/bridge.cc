// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "bridge.h"
#include "bridge_prop.h"
#include "exceptions.h"
#include "modulo.h"
#include "meca.h"
#include "random.h"
#include "space.h"

extern Modulo* modulo;
extern Random RNG;

//------------------------------------------------------------------------------

Bridge::Bridge(BridgeProp const* p, Vector const& w)
: Couple(p, w), prop(p)
{
}


Bridge::~Bridge()
{
    prop = 0;
}

//------------------------------------------------------------------------------

/**
 Calculates the force for the interLongLink()
 */
Vector Bridge::force1() const
{
    Vector d = cHand2->pos() - cHand1->pos();
    
    //correct for periodic space:
    if ( modulo )
        modulo->fold(d);
    
    real dn = d.norm();
    
    return ( prop->stiffness * ( 1 - prop->length / dn ) ) * d;
}


/**
 This uses interLongLink().
 */
void Bridge::setInteractions(Meca & meca) const
{
    meca.interLongLink(cHand1->interpolation(), cHand2->interpolation(), prop->length, prop->stiffness);
}

//------------------------------------------------------------------------------
/**
 This will:
 - prevent binding twice to the same fiber, if ( prop:stiff = true )
 - check prop:specificity with respect to the configuration of the two fibers
 */
bool Bridge::allowAttachment(const FiberBinder & fb)
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
            throw InvalidParameter("unknown bridge:specificity");
    }
    return true;
}


void Bridge::stepFF(const FiberGrid& grid)
{
    assert_true( !attached1() && !attached2() );
    
    // diffusion:
    cPos.addRand( prop->diffusion_dt );
    
    // confinement
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



