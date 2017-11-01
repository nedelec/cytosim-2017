// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "fork.h"
#include "fork_prop.h"
#include "meca.h"
#include "space.h"


//------------------------------------------------------------------------------
Fork::Fork(ForkProp const* p, Vector const& w)
: Couple(p, w), prop(p)
{
    flip = 1;
}


Fork::~Fork()
{
    prop = 0;
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


void Fork::setInteractions(Meca & meca) const
{
    PointInterpolated pt1 = cHand1->interpolation();
    PointInterpolated pt2 = cHand2->interpolation();
    
    meca.interLink(pt1, pt2, prop->stiffness);
    
#if ( DIM == 2 )
    // flip the angle to match the current configuration of the bond
    if ( prop->flip )
    {
        if ( vecProd(pt1.diff(), pt2.diff()) < 0 )
            flip = -1;
        else
            flip = +1;
    }
    
    meca.interTorque2D(pt1, pt2, prop->cosinus, flip * prop->sinus, prop->angular_stiffness);
#else
    std::cerr << "The Fork does not work in 3D (unfinished code)\n";
    exit(0);
#endif
}
