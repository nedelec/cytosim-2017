// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#include "matsparse.h"
#include "cblas.h"
#include "smath.h"
#include <iomanip>
#include <sstream>

#define FASTER_CODE

#define VAL_NOT_USED   -1
#define END_OF_COLUMN  -2



MatrixSparse::MatrixSparse()
{
    mxSize      = 0;
    mxAllocated = 0;
    mxCol       = 0;
    mxRow       = 0;
}



void MatrixSparse::allocate( const unsigned int sz )
{
    mxSize = sz;
    if ( mxSize > mxAllocated )
    {
        real ** mxCol_new = new real*[mxSize];
        int **  mxRow_new = new  int*[mxSize];
        
        unsigned int ii = 0;
        if ( mxCol )
        {
            for ( ; ii < mxAllocated; ++ii )
            {
                mxCol_new[ii] =  mxCol[ii];
                mxRow_new[ii] =  mxRow[ii];
            }
            delete[] mxCol;
            delete[] mxRow;
        }
        
        for ( ; ii < mxSize; ++ii )
        {
            mxCol_new[ii] = 0;
            mxRow_new[ii] = 0;
        }
        
        mxCol = mxCol_new;
        mxRow = mxRow_new;
        mxAllocated = mxSize;
    }
}


void MatrixSparse::deallocate()
{
    if ( mxCol )
    {
        for ( unsigned int ii = 0; ii < mxAllocated; ++ii )
            if ( mxCol[ii] )
            {
                delete[] mxCol[ii];
                delete[] mxRow[ii];
            };
        delete[] mxCol;
        delete[] mxRow;
        mxCol = 0;
        mxRow = 0;
    }
    mxAllocated = 0;
}


void MatrixSparse::allocateColumn( const index_type jj, unsigned int sz )
{
    assert_true( jj < mxSize );
    assert_true( sz > 0 );
    //printf("new S-COL %i %i\n", jj, sz );
    
    const unsigned chunk = 4;
    sz = ( sz + chunk - 1 ) & -chunk;

    real* mxCol_new  = new real[sz];
    int*  mxRow_new  = new int[sz];
    
    unsigned int ii = 0;
    if ( mxCol[jj] )
    {
        for ( ; mxRow[jj][ii] != END_OF_COLUMN ; ++ii )
        {
            mxCol_new[ii] =  mxCol[jj][ii];
            mxRow_new[ii] =  mxRow[jj][ii];
        }
        
        delete[] mxCol[jj];
        delete[] mxRow[jj];
    }
    for ( ; ii < sz-1 ; ++ii )
        mxRow_new[ii] = VAL_NOT_USED;
    mxRow_new[sz-1] = END_OF_COLUMN;
    
    mxCol[jj]  = mxCol_new;
    mxRow[jj]  = mxRow_new;
}


//allocate the position if necessary:
real& MatrixSparse::operator()( index_type x, index_type y )
{
    assert_true( x < mxSize );
    assert_true( y < mxSize );
    
    if ( mxRow[y] )
    {
        index_type ii = 0;
        for ( ; mxRow[y][ii] >= 0; ++ii )
            if ( mxRow[y][ii] == (int)x )
                return mxCol[y][ii];
        
        if ( mxRow[y][ii] == END_OF_COLUMN )
            allocateColumn( y, ii + 1 );
        assert_true( mxRow[y][ii] == VAL_NOT_USED );
        mxRow[y][ii] = x;
        mxCol[y][ii] = 0;
        //printf("allo. %3i %3i\n", x, y );
        return mxCol[y][ii];
    }
    
    allocateColumn( y, 1 );
    //printf("allo. %3i %3i\n", nx, ny );
    assert_true( mxRow[y][0] == VAL_NOT_USED );
    
    //put the diagonal term first:
    mxRow[y][0] = y;
    mxCol[y][0] = 0;
    if ( x == y )
        return mxCol[y][0];
    
    mxRow[y][1] = x;
    mxCol[y][1] = 0;
    return mxCol[y][1];
}


//does not allocate the position:
real* MatrixSparse::addr( index_type x, index_type y) const
{
    int * row = mxRow[y];
    if ( row )
    {
        for ( ; *row >= 0; ++row )
            if ( *row == (int)x )
                return & mxCol[y][ row - mxRow[y] ];
    }
    return 0;
}


void MatrixSparse::makeZero()
{
    for ( unsigned int ii = 0; ii < mxSize; ++ii )
        if ( mxRow[ii] )
            for ( int jj = 0; mxRow[ii][jj] >= 0; ++jj )
                mxRow[ii][jj] = VAL_NOT_USED;
}


void MatrixSparse::scale( real a )
{
    for ( unsigned int ii = 0; ii < mxSize; ++ii )
        if ( mxRow[ii] )
            for ( int jj = 0; mxRow[ii][jj] >= 0; ++jj )
                mxCol[ii][jj] *= a;
}


void MatrixSparse::addTriangularBlock(real* M, const index_type x, const unsigned int sx ) const
{
    assert_true( x + sx <= mxSize );
    
    for ( unsigned int jj = 0; jj < sx; ++jj )
    {
        int* row = mxRow[jj + x];
        if ( row != 0 )
        {
            real* col = mxCol[jj + x];
            for ( ; *row >= 0 ; ++row, ++col )
            {
                if ( *row > (int)x )
                {
                    index_type ii = *row - x;
                    if ( ii < sx )
                    {
                        assert_true( ii <= jj );
                        M[ii + sx * jj] += * col;
                        //printf("Sp %4i %4i % .4f\n", ii, jj, a );
                    }
                }
            }
        }
    }
}


void MatrixSparse::addDiagonalBlock(real* M, const index_type x, const unsigned int sx ) const
{
    assert_true( x + sx <= mxSize );
    
    for ( index_type jj = 0; jj < sx; ++jj )
    {
        int * row = mxRow[jj + x];
        if ( row )
        {
            real* col = mxCol[jj + x];
            for ( ; *row >= 0 ; ++row, ++col )
            {
                if ( *row > (int)x )
                {
                    index_type ii = *row - x;
                    if ( ii < sx )
                    {
                        M[ii + sx * jj] += * col;
                        //printf("Sp %4i %4i % .4f\n", ii, jj, a );
                    }
                }
            }
        }
    }
}


int MatrixSparse::bad() const
{
    if ( mxSize <= 0 ) return 1;
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
    {
        if ( mxRow[jj] )
            for ( int ii = 0; mxRow[jj][ii] >= 0; ++ii )
            {
                if ( mxRow[jj][ii] < 0     ) return 2;
                if ( mxRow[jj][ii] >= (int)mxSize ) return 3;
            }
    }
    return 0;
}


void MatrixSparse::printSparse(std::ostream & os) const
{
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
    {
        if ( mxRow[jj] )
            for ( int ii = 0; mxRow[jj][ii] >= 0; ++ii )
            {
                os << mxRow[jj][ii] << " " << jj << " ";
                os << std::setw(16) << std::setprecision(8) << mxCol[jj][ii] << std::endl;
            }
    }
}


bool MatrixSparse::nonZero() const
{
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
        if ( mxRow[jj] )
            for ( int ii = 0; mxRow[jj][ii] >= 0; ++ii )
                if ( mxCol[jj][ii] != 0 )
                    return true;
    return false;
}


unsigned int MatrixSparse::nbNonZeroElements() const
{
    //all allocated elements are counted, even if the value is zero
    unsigned int cnt = 0;
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
        if ( mxRow[jj] )
            for ( int ii = 0; mxRow[jj][ii] >= 0; ++ii )
                ++cnt;
    return cnt;
}


std::string MatrixSparse::what() const
{
    std::ostringstream msg;
    msg << "SP (nnz: " << nbNonZeroElements() << ")";
    return msg.str();
}

void MatrixSparse::vecMulAdd( const real* X, real* Y ) const
{
    int kk;
    
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
    {
        if ( mxRow[jj] )
            for ( unsigned int ii = 0; ( kk = mxRow[jj][ii] ) >= 0; ++ii )
            {
                Y[kk] += mxCol[jj][ii] * X[jj];
            }
    }
}

//------------------------------------------------------------------------------
#ifndef FASTER_CODE


void MatrixSparse::vecMulAddIso2D( const real* X, real* Y ) const
{
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
    {
        if ( mxRow[jj] )
            for ( int ii = 0; mxRow[jj][ii] >= 0; ++ii )
            {
                const int kk = 2 * mxRow[jj][ii];
                const real a = mxCol[jj][ii];
                Y[kk  ] += a * X[kk  ];
                Y[kk+1] += a * X[kk+1];
            }
    }
}


void MatrixSparse::vecMulAddIso3D( const real* X, real* Y ) const
{
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
    {
        if ( mxRow[jj] )
            for ( int ii = 0; mxRow[jj][ii] >= 0; ++ii )
            {
                const int kk = 3 * mxRow[jj][ii];
                const real a = mxCol[jj][ii];
                Y[kk  ] += a * X[kk  ];
                Y[kk+1] += a * X[kk+1];
                Y[kk+2] += a * X[kk+2];
            }
    }
}



#else  //FASTER_CODE


void MatrixSparse::vecMulAddIso2D( const real* X, real* Y ) const
{
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
    {
        int* row = mxRow[jj];
        if ( row != 0 )
        {
            real* col = mxCol[jj];
            int ll = 2 * jj;
            
            real X1 = X[ll  ];
            real X2 = X[ll+1];
            
            while ( *row >= 0 )
            {
                int kk = 2 * ( *row );
                Y[kk  ] += (*col) * X1;
                Y[kk+1] += (*col) * X2;
                
                ++row;
                ++col;
            }
        }
    }
}


void MatrixSparse::vecMulAddIso3D( const real* X, real* Y ) const
{
    for ( index_type jj = 0; jj < mxSize; ++jj )
    {
        int* row = mxRow[jj];
        if ( row != 0 )
        {
            real* col = mxCol[jj];
            int ll = 3 * jj;
            
            real X1 = X[ll  ];
            real X2 = X[ll+1];
            real X3 = X[ll+2];
            
            while ( * row >= 0 )
            {
                int kk = 3 * ( *row );
                Y[kk  ] += (*col) * X1;
                Y[kk+1] += (*col) * X2;
                Y[kk+2] += (*col) * X3;
                
                ++row;
                ++col;
            }
        }
    }
}

#endif

