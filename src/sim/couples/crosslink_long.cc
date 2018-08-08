// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "crosslink_long.h"
#include "crosslink_prop.h"
#include "exceptions.h"
#include "random.h"
#include "modulo.h"
#include "meca.h"

extern Modulo* modulo;
extern Random RNG;

//------------------------------------------------------------------------------

CrosslinkLong::CrosslinkLong(CrosslinkProp const* p, Vector const& w)
: Crosslink(p, w)
{
    mArm = nullTorque;
}


CrosslinkLong::~CrosslinkLong()
{
}

//------------------------------------------------------------------------------

#if ( DIM == 2 )

/**
 Returns -len or +len
 */
real CrosslinkLong::calcArm(const PointInterpolated & pt, Vector const& pos, real len)
{
    return len * RNG.sign_exc( vecProd( pt.pos()-pos, pt.diff()) );
}

#elif ( DIM == 3 )

/**
 Return a vector or norm len, that is perpendicular to the Fiber referenced by \a pt,
 and aligned with the link.
 */
Vector CrosslinkLong::calcArm(const PointInterpolated & pt, Vector const& pos, real len)
{
    Vector a  = pt.diff();
    Vector as = pos - pt.pos();
    Vector p = ( as - ( ( as * a ) / a.normSqr() ) * a );
    real pn = p.normSqr();
    if ( pn > REAL_EPSILON )
        return p * ( len / sqrt(pn) );
    else
        return a.randPerp(len);
    //return vecProd( pt.pos()-pos, pt.diff() ).normalized(len);
}

#endif

//------------------------------------------------------------------------------

Vector CrosslinkLong::posSide() const
{
#if ( DIM == 1 )
    
    return cHand1->pos();
    
#elif ( DIM == 2 )
    
    return cHand1->pos() + vecProd(mArm, cHand1->dirFiber());
    
#elif ( DIM == 3 )
    
    return cHand1->pos() + mArm;

#endif
}


/**
 Calculates the force for the interSideLink()
 */
Vector CrosslinkLong::force1() const
{
    Vector d = cHand2->pos() - posSide();
    
    //correct for periodic space:
    if ( modulo )
        modulo->fold(d);
    
    return prop->stiffness * d;
}


/**
 This uses interSideLink().
 
 Another possibility would be SideSideLink, which is fully symmetric.
 */
void CrosslinkLong::setInteractions(Meca & meca) const
{
    PointInterpolated pt1 = cHand1->interpolation();
    PointInterpolated pt2 = cHand2->interpolation();
    
    /* 
     The 'arm' is recalculated each time, but in 2D at least,
     this maybe not necessary, as switching should be rare.
     */
    
#if ( DIM == 2 )
    
    mArm = calcArm(pt1, pt2.pos(), prop->length);
    meca.interSideLink2D(pt1, pt2, mArm, prop->stiffness);
    
#elif ( DIM == 3 )

    mArm = calcArm(pt1, pt2.pos(), prop->length);
    meca.interSideLinkS(pt1, pt2, mArm, prop->length, prop->stiffness);
    
#endif
}


