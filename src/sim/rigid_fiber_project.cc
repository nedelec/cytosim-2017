// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
//------------------------------------------------------------------------------
//------------------- projections for constrained dynamics  ------------------
//------------------------------------------------------------------------------

#include "exceptions.h"
//#include "vecprint.h"


void RigidFiber::buildProjection()
{
    //reset all variables for the projections:
    rfAllocated       = 0;
    mtJJt             = 0;
    mtJJtiJforce      = 0;
}

//------------------------------------------------------------------------------
void RigidFiber::allocateProjection(const unsigned int nb_points)
{
    if ( rfAllocated < nb_points )
    {
        //std::cerr << reference() << "allocateProjection(" << nb_points << ")\n";
        if ( mtJJt )
            delete[] mtJJt;
        
        // Keep memory aligned to 32 bytes:
        const unsigned chunk = 32 / sizeof(real);
        // make a multiple of chunk to align pointers mtJJt2:
        rfAllocated  = ( nb_points + chunk - 1 ) & -chunk;
        
        mtJJt        = new real[3*rfAllocated];
        mtJJt2       = mtJJt + rfAllocated;
        mtJJtiJforce = mtJJt + rfAllocated*2;
    }
}

//------------------------------------------------------------------------------
void RigidFiber::destroyProjection()
{
    //std::cerr << reference() << "destroyProjection\n";
    if ( mtJJt )        delete[] mtJJt;
    mtJJt        = 0;
    mtJJt2       = 0;
    mtJJtiJforce = 0;
}

//------------------------------------------------------------------------------

void RigidFiber::makeProjection()
{
    assert_true( rfAllocated >= nbPoints() );
    
    //set the diagonal and off-diagonal of J*J'
    const unsigned nbu = nbPoints() - 2;
    real const*const diff = rfDiff;

    // The diagonal terms are all equal to 2, if diff[] vectors are normalized
    for ( unsigned jj = 0; jj < nbu; ++jj )
    {
        real const* X = diff + DIM * jj;
        
#if ( DIM == 2 )
        mtJJt[jj]  = 2 * ( X[0]*X[0] + X[1]*X[1] );
        mtJJt2[jj] = - ( X[0]*X[2] + X[1]*X[3] );
#else
        mtJJt[jj]  = 2 * ( X[0]*X[0] + X[1]*X[1] + X[2]*X[2] );
        mtJJt2[jj] = - ( X[0]*X[3] + X[1]*X[4] + X[2]*X[5] );
#endif
    }
    
    real const* X = diff + DIM*nbu;
#if ( DIM == 2 )
    mtJJt[nbu] = 2 * ( X[0]*X[0] + X[1]*X[1] );
#else
    mtJJt[nbu] = 2 * ( X[0]*X[0] + X[1]*X[1] + X[2]*X[2] );
#endif    
    //VecPrint::vecPrint(std::cerr, nbu+1, mtJJt);
    //VecPrint::vecPrint(std::cerr, nbu,  mtJJt2);
    
    int info = 0;
    lapack_xpttrf( nbu+1, mtJJt, mtJJt2, &info );
    
    if ( info )
        throw Exception("could not build Fiber projection");

}

//------------------------------------------------------------------------------

/**
 Perform first calculation needed by projectForces:
 tmp <- J * X
 */
void projectForcesA(unsigned nbs, const real* diff, const real* X, real* tmp)
{
#pragma ivdep
    for ( unsigned jj = 0; jj < nbs; ++jj )
    {
        const unsigned kk = DIM*jj;
        const real * x = X+kk;
        tmp[jj] = diff[kk  ] * ( x[DIM  ] - x[0] )
                + diff[kk+1] * ( x[DIM+1] - x[1] )
#if ( DIM > 2 )
                + diff[kk+2] * ( x[DIM+2] - x[2] )
#endif
        ;
    }
}


/**
 Perform second calculation needed by projectForces:
 Y <- X + Jt * tmp
 */
void projectForcesB(unsigned nbs, const real* diff, const real sca, const real* X, real* Y, real* tmp)
{
    for ( unsigned d = 0; d < DIM; ++d )
    {
        const unsigned kk = DIM * nbs;
        Y[   d] = sca * ( X[   d] + diff[       d] * tmp[    0] );
        Y[kk+d] = sca * ( X[kk+d] - diff[kk-DIM+d] * tmp[nbs-1] );
    }
    
    for ( unsigned jj = 1; jj < nbs; ++jj )
    {
        const unsigned kk = DIM*jj;
        Y[kk  ] = sca * ( X[kk  ] + diff[kk  ] * tmp[jj] - diff[kk-DIM  ] * tmp[jj-1] );
        Y[kk+1] = sca * ( X[kk+1] + diff[kk+1] * tmp[jj] - diff[kk-DIM+1] * tmp[jj-1] );
#if ( DIM > 2 )
        Y[kk+2] = sca * ( X[kk+2] + diff[kk+2] * tmp[jj] - diff[kk-DIM+2] * tmp[jj-1] );
#endif
    }
}


void RigidFiber::projectForces(const real* X, real* Y, const real sca, real* tmp) const
{
    const unsigned nbs = nbSegments();
    
    projectForcesA(nbs, rfDiff, X, tmp);
        
    // tmp <- inv( J * Jt ) * tmp to find the multipliers
    lapack_xptts2(nbs, 1, mtJJt, mtJJt2, tmp, nbs);

    projectForcesB(nbs, rfDiff, sca, X, Y, tmp);

    //printf("Y  "); VecPrint::vecPrint(std::cerr, DIM*nbPoints(), Y );
}



void RigidFiber::computeTensions(const real* forces)
{
    const unsigned nbs = nbSegments();
    
    projectForcesA(nbs, rfDiff, forces, rfLag);
    
    // tmp <- inv( J * Jt ) * tmp to find the multipliers
    lapack_xptts2(nbs, 1, mtJJt, mtJJt2, rfLag, nbs);
}

//------------------------------------------------------------------------------
//========================= PROJECTION DERIVATIVE ============================
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv


void RigidFiber::makeProjectionDiff( const real* force )
{
    //number of constraints:
    const unsigned int nbs = nbSegments();
    assert_true( nbs > 0 );
    
#if ( 0 )
    
    /*
     This code calculates the Lagrange multipliers of the constraints
     */

    //mtJJtiJforce <- J * X
    for ( unsigned int jj = 0; jj < nbs; ++jj )
    {
        mtJJtiJforce[jj] = rfDiff[DIM*jj] * ( force[DIM*jj+DIM] - force[DIM*jj] );
        for ( unsigned d = 1; d < DIM; ++d )
            mtJJtiJforce[jj] += rfDiff[DIM*jj+d] * ( force[DIM*jj+d+DIM] - force[DIM*jj+d] );
    }
    
    // mtJJtiJforce <- inv( J * Jt ) * J * Force
    lapack_xptts2(nbs, 1, mtJJt, mtJJt2, mtJJtiJforce, nbs);

    // verify that the two calculations are identical:
    real n = 0;
    for ( unsigned int ii=0; ii<nbs; ++ii )
    {
        real y = mtJJtiJforce[ii] - rfLag[ii];
        if ( fabs(y) > n ) n = fabs(y);
    }
    if ( n > 0 )
    {
        printf("Error: %e\n", n);
        printf("Lagrange: "); VecPrint::vecPrint(std::cerr, nbs, mtJJtiJforce);
        printf("Multipl.: "); VecPrint::vecPrint(std::cerr, nbs, rfLag);
        printf("\n");
    }

#endif
    
    //----- we remove compressive forces ( negative Lagrange-multipliers )
    const real sc = 1.0 / segmentation();
    
#pragma ivdep
    for ( unsigned jj = 0; jj < nbs; ++jj )
    {
        if ( rfLag[jj] < 0 )
            mtJJtiJforce[jj] = 0;
        else
            mtJJtiJforce[jj] = rfLag[jj] * sc;
    }
}


//------------------------------------------------------------------------------

//straightforward implementation:
inline void add_projection(const unsigned nbs, real const* diff, real const* X, real* Y)
{
#pragma ivdep
    for ( unsigned jj = 0; jj < nbs; ++jj )
    {
        if ( diff[jj] )
        {
            const unsigned ll = DIM*jj;
            const unsigned kk = ll+DIM;
            for ( unsigned int d = 0; d < DIM; ++d )
            {
                real w = diff[jj] * ( X[kk+d] - X[ll+d] );
                Y[ll+d] += w;
                Y[kk+d] -= w;
            }
        }
    }
}


void RigidFiber::addProjectionDiff( const real* X, real* Y ) const
{
    add_projection(nbSegments(), mtJJtiJforce, X, Y);
}


