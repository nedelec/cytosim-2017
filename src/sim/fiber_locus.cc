// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "fiber_locus.h"
#include "space.h"
#include "fiber.h"
#include "simul.h"
#include "modulo.h"

extern Modulo * modulo;

//------------------------------------------------------------------------------
//---------------- DISTANCE FROM A POINT TO A SECTION OF FIBER -----------------
//------------------------------------------------------------------------------

/**
 W is projected on the line that supports this FiberLocus
 The function calculates:
 - abs <- the signed distance from pos1() to the projection of W
 - dist <- the distance between W and its projection
 .
 
 It is assumed here that len() returns the distance between the two points of the FiberLocus
 Attention: `dis` is not set if ( abs < 0 ) or ( abs > len() )
 */
void FiberLocus::projectPoint0(Vector const& w, real& abs, real& dis) const
{
    assert_true( mFib );
    
    Vector dx = diff();
    Vector aw = w - pos1();
    
    if ( modulo )
        modulo->fold(aw);
    
    const real ls = len();
    
    // project with the scalar product:
    abs = ( aw * dx ) / ls;
    
    // calculate distance to projection that fall inside the segment
    if ( 0 <= abs  &&  abs <= ls )
    {
#if ( DIM == 1 )
        dis = 0;
#else
        dis = aw.normSqr() - abs * abs;
#endif
    }
}


/**
 W is projected on the line that supports this FiberLocus
 The function calculates:
 - abs <- the signed distance from pos1() to the projection of W
 - dist <- the distance between W and its projection
 .
 
 It is assumed here that len() returns the distance between the two points of the FiberLocus
 Attention: `dis` may not be set if ( abs < 0 ) or ( abs > len() )
 */
void FiberLocus::projectPoint(Vector const& w, real& abs, real& dis) const
{
    assert_true( mFib );
    
    Vector dx = diff();
    Vector aw = w - pos1();
    
    if ( modulo )
        modulo->fold(aw);
    
    const real ls = len();
    
    // project with the scalar product:
    abs = ( aw * dx ) / ls;
    
    // test boundaries of segment:
    if ( abs < 0 )
    {
        if ( isFirst() )
            dis = w.distanceSqr(pos1());
    }
    else if ( abs > ls )
    {
        if ( isLast() )
            dis = w.distanceSqr(pos2());
    }
    else
    {
#if ( DIM == 1 )
        dis = 0;
#else
        dis = aw.normSqr() - abs * abs;
#endif
    }
}


/**
 This is a faster version of projectPoint(), but it does not work with periodic boundaries
 */
void FiberLocus::projectPointF(const real w[], real& abs, real& dis) const
{
    assert_true( mFib );
    assert_true( modulo == 0 );
    
    const real * p = mFib->addrPoint(mPoint);
    
    real dX = p[DIM  ] - p[0];
    real aX = w[0]     - p[0];
#if ( DIM > 1 )
    real dY = p[DIM+1] - p[1];
    real aY = w[1]     - p[1];
#endif
#if ( DIM > 2 )
    real dZ = p[DIM+2] - p[2];
    real aZ = w[2]     - p[2];
#endif
    
    const real ls = len();

    // project with the scalar product:
#if (DIM == 1)
    abs = ( dX * aX ) / ls;
#elif ( DIM == 2 )
    abs = ( dX * aX + dY * aY ) / ls;
#elif ( DIM == 3 )
    abs = ( dX * aX + dY * aY + dZ * aZ ) / ls;
#endif
    
    // test boundaries of segment:
    if ( abs < 0 )
    {
        if ( isFirst() )
            dis = distanceSqr(w, pos1());
    }
    else if ( abs > ls )
    {
        if ( isLast() )
            dis = distanceSqr(w, pos2());
    }
    else
    {
#if   ( DIM == 1 )
        dis = 0;
#elif ( DIM == 2 )
        dis = aX * aX + aY * aY - abs * abs;
#elif ( DIM == 3 )
        dis = aX * aX + aY * aY + aZ * aZ - abs * abs;
#endif
    
#if ( 0 )
        // verify that the results are identical to projectPoint()
        real a, d;
        projectPoint(Vector(w), a, d);
        assert_small(a-abs);
        assert_small(d-dist);
#endif
    }
}


//------------------------------------------------------------------------------
/**
 @return 0, +1 or -1:
 - 0 if the segment does not interects the plane (in this case, \a abs is not set)
 - +1 if the segment interect and fiber->dir().n > 0
 - -1 if the segement intersects, and fiber->dir().n < 0
 .
 Intersection with the MINUS_END point is counted,
 but not with the PLUS_END point (return = 0).
 */

int FiberLocus::intersectPlane(Vector const& n, const real a, real & abs) const
{
    const real x = diff() * n;
    
    if ( x < -REAL_EPSILON  ||  REAL_EPSILON < x  )
    {    
        real c = - ( pos1() * n + a ) / x;
        
        if ( 0 <= c  &&  c < 1 )
        {
            abs = mFib->abscissaP(mPoint+c);
            return ( x > 0 ) ? 1 : -1;
        }
    }
    return 0;
}


//------------------------------------------------------------------------------
//---------------- DISTANCE TO ANOTHER SECTION OF A FIBER ----------------------
//------------------------------------------------------------------------------

/**

 Set \a dis to be the square of the minimum distance between the two lines.
 In 2D, the distance should be zero unless the two lines are parallel.

 Abscissa \a abs1 and \a abs1 are set to indicate the points realizing this distance.
 The return value is true if both projections fall inside the segments.
 */


bool FiberLocus::projectSegment(FiberLocus const& that, real& abs1, real& abs2, real& dist) const
{
    Vector d1  = diff();
    Vector d2  = that.diff();
    Vector d12 = that.pos1() - pos1();
    
    real len1 = len();
    real len2 = that.len();
    
    if ( modulo )
        modulo->fold(d12);
    
    real beta = ( d1 * d2 ) / ( len1 * len2 );
    real scal = 1.0 - beta * beta;
    
    if ( scal > REAL_EPSILON )
    {
        // This is the general case of non-parallel lines:
        
        real d1d12 = ( d1 * d12 ) / ( scal * len1 );
        real d2d12 = ( d2 * d12 ) / ( scal * len2 );
        
        //abs1 = (( d1 / len1 - beta * d2 / len2 ) * d12 ) / scal;
        abs1 = d1d12 - beta * d2d12;
        
        if ( abs1 < 0  || len1 < abs1 )
            return false;
        
        //abs2 = (( beta * d1 / len1 - d2 / len2 ) * d12 ) / scal;
        abs2 = beta * d1d12 - d2d12;
        
        if ( abs2 < 0  || len2 < abs2 )
            return false;
        
        //dist = ( d12 + (abs2/len2) * d2 - (abs1/len1) * d1 ).normSqr();
        //dist = ( d12 - d1 * (abs1/len1) ).normSqr() - abs2 * abs2;
        dist = ( d12 + d2 * (abs2/len2) ).normSqr() - abs1 * abs1;
        
        return true;
    }

    /*
     This deals with the case where the two segments are almost parallel:
     beta = +/- 1
     p1 = projection of that.pos1() on this segment
     p2 = projection of that.pos2()
     */
    real p1 = ( d12 * d1 ) / len1;
    real p2 = p1 + beta * len2;
    
    if ( p1 < 0  &&  p2 < 0 )
        return false;
    
    if ( p1 > len1  &&  p2 > len1 )
        return false;
    
    real p = p1;
    dist = d12.normSqr() - p1 * p1;
    
    if ( p1 < 0 ) p1 = 0; else if ( p1 > len1 ) p1 = len1;
    if ( p2 < 0 ) p2 = 0; else if ( p2 > len1 ) p2 = len1;
    
    // take the middle of the intersection:
    abs1 = 0.5 * ( p1 + p2 );
    //abs2 = abs1 * beta - ( d12 * d2 ) / len2;
    abs2 = ( abs1 - p ) * beta;
    
    return true;
}

