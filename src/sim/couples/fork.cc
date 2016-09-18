// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "fork.h"
#include "fork_prop.h"
#include "meca.h"


//------------------------------------------------------------------------------
Fork::Fork(ForkProp const* p, Vector const& w)
: Couple(p, w), prop(p)
{
}


Fork::~Fork()
{
    prop = 0;
}

//------------------------------------------------------------------------------

void Fork::setInteractions(Meca & meca) const
{    
    meca.interLink(cHand1->interpolation(), cHand2->interpolation(), prop->stiffness);
}


void Fork::stepFF(const FiberGrid& grid)
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

