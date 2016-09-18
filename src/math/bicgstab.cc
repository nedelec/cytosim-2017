// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "bicgstab.h"


/**
 Here is defined which norm is used to measure the residual
 */
bool Solver::Monitor::finished(const unsigned int size, const real * x)
{
#if ( 0 )
    // this is the standard Euclidian norm
    mResid = blas_xnrm2(size, x, 1);
#else
    // use the 'infinite' norm
    mResid = blas_xnrm8(size, x);
#endif
    
#if ( 1 )
    if ( mIter > mIterOld+31 )
    {
        if ( mResid >= mResidOld )
        {
            std::cerr << "Stagnant solver?";
            std::cerr << " residuals: " << std::scientific << mResidOld << " at iteration " << mIterOld;
            std::cerr << ", " << std::scientific << mResid << " at " << mIter << std::endl;
        }
        mIterOld = mIter;
        mResidOld = mResid;
    }
#endif
    
#if ( 0 )
    if ( mIter > 64 )
        std::cerr << "Solver step " << mIter << " residual " << mResid << std::endl;
#endif
    
    if ( mResid != mResid )
    {
        std::cerr << "Solver diverged: step " << mIter << " resid = not-a-number" << std::endl;
        mResid = INFINITY;
        return true;
    }
    
    if ( mIter > mIterMax )
        return true;
    
    return ( mResid < mResidMax );
}

