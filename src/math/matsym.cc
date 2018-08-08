// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "matsym.h"
#include "cblas.h"


//------------------------------------------------------------------------------
MatrixSymmetric::MatrixSymmetric()
{
    mxSize      = 0;
    mxAllocated = 0;
    val       = 0;
    do_not_delete_array = 0;
}

//------------------------------------------------------------------------------
void MatrixSymmetric::allocate( unsigned int sz )
{
    mxSize = sz;
    if ( mxSize > mxAllocated ) {
        mxAllocated = mxSize;
        if ( val ) delete[] val;
        val = new real[mxSize*mxSize];
    }
}

//------------------------------------------------------------------------------
void MatrixSymmetric::deallocate()
{
    if ( do_not_delete_array ) return;
    if ( val ) delete[] val;
    mxAllocated = 0;
    val = 0;
}

//------------------------------------------------------------------------------
void MatrixSymmetric::makeZero()
{
    for ( index_type ii = 0; ii < mxSize * mxSize; ++ii )
        val[ii] = 0;
}

//------------------------------------------------------------------------------
void MatrixSymmetric::scale( real a )
{
    for ( index_type ii = 0; ii < mxSize * mxSize; ++ii )
        val[ii] *= a;
}

//------------------------------------------------------------------------------
real& MatrixSymmetric::operator()( index_type x, index_type y)
{
    assert_true( x < mxSize );
    assert_true( y < mxSize );
    if ( x < y )
        return val[ x+mxSize*y ];
    else
        return val[ y+mxSize*x ];
}

//------------------------------------------------------------------------------
real* MatrixSymmetric::addr( index_type x, index_type y) const
{
    assert_true( x < mxSize );
    assert_true( y < mxSize );
    if ( x < y )
        return &val[ x+mxSize*y ];
    else
        return &val[ y+mxSize*x ];
}

//------------------------------------------------------------------------------
bool MatrixSymmetric::nonZero() const
{
    return true;
}

//------------------------------------------------------------------------------
unsigned int MatrixSymmetric::nbNonZeroElements() const
{
    return mxSize * mxSize;
}

//------------------------------------------------------------------------------
std::string MatrixSymmetric::what() const
{
    return "full-symmetric";
}

//------------------------------------------------------------------------------
void MatrixSymmetric::vecMulAdd( const real* X, real* Y ) const
{
    blas_xsymv( 'U', mxSize, 1.0, val, mxSize, X, 1, 1.0, Y, 1 );
}

//------------------------------------------------------------------------------
void MatrixSymmetric::vecMulAddIso2D( const real* X, real* Y ) const
{
    blas_xsymv( 'U', mxSize, 1.0, val, mxSize, X+0, 2, 1.0, Y+0, 2 );
    blas_xsymv( 'U', mxSize, 1.0, val, mxSize, X+1, 2, 1.0, Y+1, 2 );
}

//------------------------------------------------------------------------------
void MatrixSymmetric::vecMulAddIso3D( const real* X, real* Y ) const
{
    blas_xsymv( 'U', mxSize, 1.0, val, mxSize, X+0, 3, 1.0, Y+0, 3 );
    blas_xsymv( 'U', mxSize, 1.0, val, mxSize, X+1, 3, 1.0, Y+1, 3 );
    blas_xsymv( 'U', mxSize, 1.0, val, mxSize, X+2, 3, 1.0, Y+2, 3 );
}


