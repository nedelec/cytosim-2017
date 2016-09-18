// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "bridge.h"
#include "bridge_prop.h"
#include "exceptions.h"
#include "modulo.h"
#include "meca.h"
#include "random.h"

extern Modulo* modulo;
extern Random RNG;

//------------------------------------------------------------------------------

Bridge::Bridge(BridgeProp const* p, Vector const& w)
: Couple(p, w), prop(p)
{
    mArm = nullTorque;
}


Bridge::~Bridge()
{
    prop = 0;
}

//------------------------------------------------------------------------------

#if ( DIM == 2 )

/**
 Returns -len or +len
 */
real Bridge::calcArm(const PointInterpolated & pt, Vector const& pos, real len)
{
    return len * RNG.sign_exc( vecProd( pt.pos()-pos, pt.diff()) );
}

#elif ( DIM == 3 )

/**
 Return a vector or norm len, that is perpendicular to the Fiber referenced by \a pt,
 and aligned with the link.
 */
Vector Bridge::calcArm(const PointInterpolated & pt, Vector const& pos, real len)
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

Vector Bridge::posSide() const
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
Vector Bridge::force1() const
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
void Bridge::setInteractions(Meca & meca) const
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



