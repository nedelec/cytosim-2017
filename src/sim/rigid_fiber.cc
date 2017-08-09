// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "sim.h"
#include "rigid_fiber.h"
#include "cblas.h"
#include "clapack.h"
#include "matrix.h"
#include "smath.h"
#include "random.h"

extern Random RNG;

//optimization for speed:
#define FASTER_FIBER

//------------------------------------------------------------------------------
RigidFiber::RigidFiber()
{
    buildProjection();
    rfMobility  = 0;
    rfRigidity  = 0;
    rfDiff      = 0;
    rfLag       = 0;
    rfTMP       = 0;
}


RigidFiber::~RigidFiber()
{
    destroyProjection();
    if ( rfDiff )
    {
        delete(rfDiff);
        rfDiff = 0;
        rfLag  = 0;
        rfTMP  = 0;
    }
}


//------------------------------------------------------------------------------
RigidFiber::RigidFiber(RigidFiber const&)
{
    ABORT_NOW("unfinished: cannot copy a Fiber");
}


RigidFiber& RigidFiber::operator=(RigidFiber const&)
{
    ABORT_NOW("unfinished: cannot copy a Fiber");
}


//------------------------------------------------------------------------------
unsigned int RigidFiber::allocatePoints(unsigned int nbp)
{
    unsigned int ms = PointSet::allocatePoints(nbp);
    /*
     if PointSet::allocatePoints() allocated memory, it will return the 
     size of the new array, and we allocate the same size for other arrays.
     */
    if ( ms )
    {
        //std::cerr << "RigidFiber::allocatePoints " << ms << std::endl;
        allocateProjection(ms);
        
        // allocate memory:
        if ( rfDiff )
            delete(rfDiff);
        
        rfDiff = new real[ms*(2*DIM+1)];
        rfLag  = rfDiff + ms*DIM;
        rfTMP  = rfLag + ms;
        
        // reset Lagrange multipliers
        for ( unsigned int p = 0; p < ms; ++p )
            rfLag[p] = 0;
    }
    return ms;
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 The argument should be: sc = kT / dt;
 */
real RigidFiber::addBrownianForces( real* rhs, real sc ) const
{
    real b = sqrt( 2 * sc / rfMobility );

    for ( unsigned jj = 0; jj < DIM*nbPoints(); ++jj )
        rhs[jj] += b * RNG.gauss();
    
    return rfMobility * b;
}


//------------------------------------------------------------------------------

/**
 Calculate the normalized difference between successive model point of the fiber:
 @code
 for ( int n = 0; n < DIM*lastPoint(); ++n )
     rfDiff[n] = ( psPos[n+DIM] - psPos[n] ) / segmentation();
 @endcode
 */

void RigidFiber::storeDifferences()
{
    const real sc  = 1.0 / segmentation();
    const unsigned end = DIM * lastPoint();
    for ( unsigned p = 0; p < end; ++p )
        rfDiff[p] = sc * ( psPos[p+DIM] - psPos[p] );
}


/**
 If ( \a rhs != 0 ), then the local array \a rfLag[] is used,
 thus saving the Lagrange multiplier associated with the constraints for later use.
 */
void RigidFiber::setSpeedsFromForces(const real* X, real* Y, const real sc, bool rhs) const
{
    assert_true( X != Y );

    if ( rhs )
        projectForces(X, Y, sc*rfMobility, rfLag);
    else
        projectForces(X, Y, sc*rfMobility, rfTMP);
}

//------------------------------------------------------------------------------
#pragma mark -


/// the manual SSE3 code is only valid with double precision and if DIM == 2:
#define FIBER_USES_INTEL_SSE3 ( DIM == 2 ) && defined(__SSE3__) && !defined(REAL_IS_FLOAT)


#if ( DIM > 1 )

#include "rigid_fiber_project.cc"

#else

void RigidFiber::buildProjection()   {}  //DIM == 1
void RigidFiber::makeProjection()    {}  //DIM == 1
void RigidFiber::destroyProjection() {}  //DIM == 1
void RigidFiber::allocateProjection(unsigned int) {}  //DIM == 1

void RigidFiber::projectForces(const real* X, real* Y, real s, real*) const
{
    real sum = X[0];
    for ( unsigned int ii = 1; ii < nbPoints(); ++ii )
        sum += X[ii];
    
    sum = s * sum / (real) nbPoints();
    for ( unsigned int ii = 0; ii < nbPoints(); ++ii )
        Y[ii] = sum;
}

void RigidFiber::computeTensions(const real*) {} //DIM == 1
void RigidFiber::makeProjectionDiff(const real*) {} //DIM == 1
void RigidFiber::addProjectionDiff(const real*, real*) const {} //DIM == 1

#endif


//-----------------------------------------------------------------------
#pragma mark -

/*
 void RigidFiber::addRigidityMatUp(Matrix & mB, const int offset) const
 //only the upper diagonal is set
 {
     for ( unsigned int ii = 0; ii < nbPoints() - 2 ; ++ii )
     {
         mB( offset + ii   ,  offset + ii   ) += -1 * rfRigidity;
         mB( offset + ii   ,  offset + ii+1 ) += +2 * rfRigidity;
         mB( offset + ii   ,  offset + ii+2 ) += -1 * rfRigidity;
         mB( offset + ii+1 ,  offset + ii+1 ) += -4 * rfRigidity;
         mB( offset + ii+1 ,  offset + ii+2 ) += +2 * rfRigidity;
         mB( offset + ii+2 ,  offset + ii+2 ) += -1 * rfRigidity;
     }
 }
 */

//set rigidity matrix elements, only the upper diagonal is set
void RigidFiber::addRigidityMatUp(Matrix & mB, const int offset) const
{
    int sz = nbPoints();
    if ( sz < 3 ) return;
    
    int ii;
    int s = offset;
    int e = offset + sz;
    
    for (ii = s+2;  ii < e-2 ; ++ii )  mB( ii, ii   ) -= 6*rfRigidity;
    for (ii = s+1;  ii < e-2 ; ++ii )  mB( ii, ii+1 ) += 4*rfRigidity;
    for (ii = s;    ii < e-2 ; ++ii )  mB( ii, ii+2 ) -=   rfRigidity;
    
    mB( s  , s   ) -= rfRigidity;
    mB( e-1, e-1 ) -= rfRigidity;
    if ( sz == 3 ) {
        mB( s+1, s+1 ) -= 4*rfRigidity;
    }
    else {
        mB( s+1, s+1 ) -= 5*rfRigidity;
        mB( e-2, e-2 ) -= 5*rfRigidity;
    }
    mB( s  , s+1 ) += 2*rfRigidity;
    mB( e-2, e-1 ) += 2*rfRigidity;
}


//------------------------------------------------------------------------------

inline void add_rigidity1(real const* X, const unsigned nbs, real* Y, const real rigid)
{
    for ( unsigned jj = 0; jj < nbs; ++jj )
    {
        real f = rigid * ( X[jj] - 2*X[jj+DIM] + X[jj+DIM*2] );
        Y[jj      ] -=   f;
        Y[jj+DIM  ] += f+f;
        Y[jj+DIM*2] -=   f;
    }
}


#if (FIBER_USES_INTEL_SSE3)

#include <pmmintrin.h>

/*
 In this version the loop is unrolled, pointers are used
 and further optimization are made by replacing
 ( a0 -2*a1 + a2 ) by (a2-a1)-(a1-a0).
 
 //fast version with SSE 128bit vector-arithmetics
 */

inline void add_rigiditySSE(real const* X, const unsigned nbs, real* Y, real rigid)
{    
    __m128d R = _mm_set1_pd(rigid);
    
    real * yp = Y;
    const real * xn = X + DIM + DIM;
    real *const end = Y + nbs;
    
    __m128d x   = _mm_load_pd(X+DIM);
    __m128d d   = _mm_sub_pd(x, _mm_load_pd(X));
    __m128d df  = _mm_setzero_pd();
    __m128d of  = _mm_setzero_pd();
    __m128d odf = _mm_setzero_pd();
    
    while ( yp < end )
    {
        __m128d z = _mm_load_pd(xn);
        xn += DIM;
        __m128d e = _mm_sub_pd(z, x);
        x = z;
        
        __m128d f = _mm_mul_pd(R, _mm_sub_pd(e, d));
        d  = e;
        df = _mm_sub_pd(f, of);
        of = f;
        _mm_store_pd(yp, _mm_add_pd(_mm_load_pd(yp), _mm_sub_pd(odf, df)));
        yp += DIM;
        odf = df;
    }
    
    _mm_store_pd(yp, _mm_add_pd(_mm_load_pd(yp), _mm_add_pd(df, of)));
    yp += DIM;
    _mm_store_pd(yp, _mm_sub_pd(_mm_load_pd(yp), of));
}

#endif

//------------------------------------------------------------------------------


#if ( DIM == 1 )

void RigidFiber::addRigidity(const real* X, real* Y) const {}

#else

/**
 calculate the second-differential of points,
 scale by the rigidity term, and add to vector Y
*/
void RigidFiber::addRigidity(const real* X, real* Y) const
{
    if ( nbPoints() > 2 )
#if (FIBER_USES_INTEL_SSE3)
        #warning "Manual SSE3 code in RigidFiber::addRigidity()"
        add_rigiditySSE(X, DIM*(nbPoints()-2), Y, rfRigidity);
#else
        add_rigidity1(X, DIM*(nbPoints()-2), Y, rfRigidity);
#endif
}

#endif


