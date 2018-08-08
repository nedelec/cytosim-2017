// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "shackle_long.h"
#include "shackle_prop.h"
#include "modulo.h"
#include "meca.h"

extern Modulo * modulo;
extern Random RNG;

//------------------------------------------------------------------------------
ShackleLong::ShackleLong(ShackleProp const* p, Vector const& w)
: Shackle(p, w)
{
    mArm = nullTorque;
}


//------------------------------------------------------------------------------
#if ( DIM == 2 )

/**
 Returns -len or +len
 */
real ShackleLong::calcArm(const PointInterpolated & pt, Vector const& pos, real len)
{
    return len * RNG.sign_exc( vecProd( pt.pos()-pos, pt.diff()) );
}

#elif ( DIM == 3 )

/**
 Return a vector or norm len, that is perpendicular to the Fiber referenced by \a pt,
 and aligned with the link.
 */
Vector ShackleLong::calcArm(const PointInterpolated & pt, Vector const& pos, real len)
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
Vector ShackleLong::posSide() const
{
#if ( DIM == 1 )
    
    return cHand1->pos();
    
#elif ( DIM == 2 )
    
    return cHand1->pos() + vecProd(mArm, cHand1->dirFiber());
    
#elif ( DIM == 3 )
    
    return cHand1->pos() + mArm;
    
#endif
}

//------------------------------------------------------------------------------
/**
 When ( prop->length > 0 ), we calculate the force for the interSideLink()
 */
Vector ShackleLong::force1() const
{
    Vector d = cHand2->pos() - posSide();
        
    //correct for periodic space:
    if ( modulo )
        modulo->fold(d);
    
    return prop->stiffness * d;
}

//------------------------------------------------------------------------------
/**
 The interaction is slipery on hand1
 */
void ShackleLong::setInteractions(Meca & meca) const
{
    PointInterpolated pt1 = cHand1->interpolation();
    PointInterpolated pt2 = cHand2->interpolation();

#if ( DIM == 2 )
    
    mArm = calcArm(pt1, pt2.pos(), prop->length);
    meca.interSideSlidingLink2D(pt1, pt2, mArm, prop->stiffness);
    
#elif ( DIM == 3 )
    
    mArm = calcArm(pt1, pt2.pos(), prop->length);
    meca.interSideSlidingLinkS(pt1, pt2, mArm, prop->length, prop->stiffness);
    
#endif
}

