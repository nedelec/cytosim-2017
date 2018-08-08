// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "assert_macro.h"
#include "matrix.h"
#include "cblas.h"
#include <iomanip>

//------------------------------------------------------------------------------
real Matrix::value(const index_type x, const index_type y) const
{
    real* v = addr( x, y );
    if ( v == 0 )
        return 0;
    else
        return *v;
}

//------------------------------------------------------------------------------
real Matrix::maxNorm() const
{
    const unsigned int sz = size();
    real result = 0;
    for ( unsigned int ii = 0; ii < sz; ++ii )
    {
        for ( unsigned int jj = 0; jj < sz; ++jj )
        {
            real* v = addr( ii, jj );
            if ( v  &&  ( *v > result ) )
                result = *v;
        }
    }
    return result;
}

//------------------------------------------------------------------------------
bool Matrix::nonZero() const
{
    const unsigned int sz = size();
    for ( unsigned int ii = 0; ii < sz; ++ii )
        for ( unsigned int jj = 0; jj < sz; ++jj )
            if ( 0 != value( ii, jj ) )
                return true;
    return false;
}

//------------------------------------------------------------------------------
unsigned int  Matrix::nbNonZeroElements() const
{
    const unsigned int sz = size();
    unsigned int result = 0;
    for ( unsigned int ii = 0; ii < sz; ++ii )
        for ( unsigned int jj = 0; jj < sz; ++jj )
            result += ( 0 != value( ii, jj ) );
    return result;
}

//------------------------------------------------------------------------------
void Matrix::copyBlock(real* M, const index_type x, const unsigned int sx, const index_type y, const unsigned int sy) const
{
    assert_true( x + sx <= size() );
    assert_true( y + sy <= size() );
    for ( unsigned int ii = 0; ii < sx; ++ii )
        for ( unsigned int jj = 0; jj < sy; ++jj )
            M[ii + sx * jj] = value( x + ii, y + jj );
}

//------------------------------------------------------------------------------
void Matrix::addDiagonalBlock(real* M, const index_type x, const unsigned int sx) const
{
    for ( unsigned int ii = 0; ii < sx; ++ii )
        for ( unsigned int jj = 0; jj < sx; ++jj )
            M[ii + sx * jj] += value( x + ii, x + jj );
}

//------------------------------------------------------------------------------
void Matrix::addTriangularBlock(real* M, const index_type x, const unsigned int sx) const
{
    for ( unsigned int ii = 0; ii < sx; ++ii )
        for ( unsigned int jj = ii; jj < sx; ++jj )
            M[ii + sx * jj] += value( x + ii, x + jj );
}

//------------------------------------------------------------------------------
void Matrix::vecMul( const real* X, real* Y ) const
{
    blas_xzero(size(), Y);
    vecMulAdd( X, Y );
}

//------------------------------------------------------------------------------
void Matrix::printFull(std::ostream & os) const
{
    const unsigned int sz = size();
    //printf("%i %i\n", size, size);
    for ( unsigned int ii = 0; ii < sz; ++ii )
    {
        for ( unsigned int jj = 0; jj < sz; ++jj )
        {
            if ( value(ii, jj) == 0 )
                os << "       . ";
            else
                os << std::setprecision(2) << std::setw(8) << value(ii, jj) << " ";
        }
        os << std::endl;
    }
}

//------------------------------------------------------------------------------
void Matrix::printSparse(std::ostream & os) const
{
    const unsigned int sz = size();
    for ( unsigned int ii = 0; ii < sz; ++ii )
        for ( unsigned int jj = 0; jj < sz; ++jj )
            if ( addr( ii, jj ) )
                os << ii << " " << jj << " " << std::scientific << *addr(ii, jj) << std::endl;
}



