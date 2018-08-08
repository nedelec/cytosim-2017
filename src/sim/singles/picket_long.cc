// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "picket_long.h"
#include "simul.h"
#include "meca.h"
#include "modulo.h"


extern Modulo * modulo;


//------------------------------------------------------------------------------

PicketLong::PicketLong(SingleProp const* p, Vector const& w)
: Picket(p, w)
{
#if ( 0 )
    if ( p->diffusion > 0 )
        throw InvalidParameter("single:diffusion cannot be > 0 if activity=fixed");
#endif

    mArm = nullTorque;
}


PicketLong::~PicketLong()
{
    //std::cerr<<"~PicketLong("<<this<<")"<<std::endl;
}

//------------------------------------------------------------------------------

#if ( DIM == 2 )

/**
 Returns -len or +len
 */
real PicketLong::calcArm(const PointInterpolated & pt, Vector const& pos, real len)
{
    return len * RNG.sign_exc( vecProd( pt.pos()-pos, pt.diff()) );
}

#elif ( DIM == 3 )

/**
 Return a vector or norm len, that is perpendicular to the Fiber referenced by \a pt,
 and also perpendicular to the link.
 */
Vector PicketLong::calcArm(const PointInterpolated & pt, Vector const& pos, real len)
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
Vector PicketLong::posSide() const
{
#if ( DIM > 1 )
    return sHand->pos() + vecProd(mArm, sHand->dirFiber());
#endif
    return sHand->pos();
}

//------------------------------------------------------------------------------
/**
 This calculates the force corresponding to the interSideLink()
 */
Vector PicketLong::force() const
{
    if ( sHand->attached() )
    {
        Vector d = sPos - posSide();
        
        if ( modulo )
            modulo->fold(d);
        
        return prop->stiffness * d;
    }
    return Vector(0,0,0);
}

//------------------------------------------------------------------------------
void PicketLong::setInteractions(Meca & meca) const
{
#if ( DIM == 1 )
    meca.interClamp(sHand->interpolation(), sPos, prop->stiffness);
#else
    PointInterpolated pt = sHand->interpolation();
    
    /* 
     The 'arm' is recalculated every time, but in 2D at least,
     this maybe not necessary, as flipping should occur rarely.
     */
    
    mArm = calcArm(pt, sPos, prop->length);
    
#if ( DIM == 2 )
    meca.interSideClamp2D(pt, sPos, mArm, prop->stiffness);
#elif ( DIM == 3 )
    meca.interSideClamp3D(pt, sPos, mArm, prop->stiffness);
#endif

#endif
}


