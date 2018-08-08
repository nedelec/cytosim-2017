// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "shackle.h"
#include "shackle_prop.h"
#include "fiber_locus.h"
#include "meca.h"


Shackle::Shackle(ShackleProp const* p, Vector const & w)
: Couple(p, w), prop(p)
{
}

Shackle::~Shackle()
{
    prop = 0;
}

//------------------------------------------------------------------------------
/**
 The interaction is slipery on hand1
 */
void Shackle::setInteractions(Meca & meca) const
{
    PointInterpolated pt1 = cHand1->interpolation();
    PointInterpolated pt2 = cHand2->interpolation();

    meca.interSlidingLink(pt1, pt2, prop->stiffness);
}

//------------------------------------------------------------------------------
void Shackle::stepAA()
{
    assert_true( attached1() && attached2() );
    
    real abs, dis = INFINITY;
    // project the position of cHand2 to set abscissa of cHand1
    abs = cHand1->fiber()->projectPoint(cHand2->pos(), dis);
    
    //std::cerr << "Shackle " << proj.abscissa() - cHand1->abscissa() << std::endl; 
    cHand1->moveTo(abs);
    
    if ( ! cHand1->attached() )
        return;
    
    Vector f = force1();
    cHand1->stepLoaded( f);
    cHand2->stepLoaded(-f);
}
