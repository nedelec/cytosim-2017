// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "wrist_long.h"
#include "simul.h"
#include "meca.h"
#include "modulo.h"


extern Modulo * modulo;


WristLong::WristLong(SingleProp const* p, Mecable const* mec, unsigned pti)
: Wrist(p, mec, pti)
{
#if ( 0 )
    if ( p->diffusion > 0 )
        throw InvalidParameter("single:diffusion cannot be > 0 if activity=anchored");
#endif

    mArm = nullTorque;
}


WristLong::~WristLong()
{
}

//------------------------------------------------------------------------------

#if ( DIM == 2 )

/**
 Returns -len or +len
 */
real WristLong::calcArm(const PointInterpolated & pt, Vector const& pos, real len)
{
    return len * RNG.sign_exc( vecProd( pt.pos()-pos, pt.diff()) );
}

#elif ( DIM == 3 )

/**
 Return a vector or norm len, that is perpendicular to the Fiber referenced by \a pt,
 and also perpendicular to the link.
 */
Vector WristLong::calcArm(const PointInterpolated & pt, Vector const& pos, real len)
{
    Vector a = vecProd( pt.pos()-pos, pt.diff() );
    real an = a.normSqr();
    if ( an > REAL_EPSILON )
        return a * ( len / sqrt(an) );
    else
        return pt.diff().randPerp(len);
}

#endif


//------------------------------------------------------------------------------
Vector WristLong::force() const
{
    if ( sHand->attached() )
    {
        Vector d = sBase.pos() - posSide();
    
        if ( modulo )
            modulo->fold(d);
        
        return prop->stiffness * d;
    }
    return Vector(0,0,0);
}


//------------------------------------------------------------------------------
#if ( 1 )

Vector WristLong::posSide() const
{
#if ( DIM > 1 )
    return sHand->pos() + vecProd(mArm, sHand->dirFiber());
#endif
    return sHand->pos();
}

/**
 Using a Meca::interSideLink()
 */
void WristLong::setInteractions(Meca & meca) const
{
    PointInterpolated pt = sHand->interpolation();
    
    /* 
     The 'arm' is recalculated each time, but in 2D at least,
     this maybe not necessary, as switching should be rare.
     */
    
#if ( DIM == 2 )
    
    mArm = calcArm(pt, sBase.pos(), prop->length);
    meca.interSideLink2D(pt, sBase, mArm, prop->stiffness);
    
#elif ( DIM == 3 )
    
    mArm = calcArm(pt, sBase.pos(), prop->length);
    meca.interSideLinkS(pt, sBase, mArm, prop->length, prop->stiffness);
    
#endif
}


//------------------------------------------------------------------------------
#else

Vector WristLong::posSide() const
{
    return sHand->pos() + mArm;
}

/** 
 This uses Meca::interLongLink() 
 */
void WristLong::setInteractions(Meca & meca) const
{
    PointInterpolated pt = sHand->interpolation();
    mArm = ( sBase.pos() - sHand->pos() ).normalized(prop->length);
    meca.interLongLink(pt, sBase, prop->length, prop->stiffness);
}

#endif


