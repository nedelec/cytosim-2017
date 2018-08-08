// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "matsparsesym.h"
#include "cblas.h"
#include "smath.h"
#include <iomanip>
#include <sstream>

//------------------------------------------------------------------------------
MatrixSparseSymmetric::MatrixSparseSymmetric()
{
    mxSize      = 0;
    mxAllocated = 0;
    col       = 0;
    colSize   = 0;
    colMax    = 0;
}


//------------------------------------------------------------------------------
void MatrixSparseSymmetric::allocate( const unsigned int sz )
{
    mxSize = sz;
    if ( mxSize > mxAllocated )
    {
        Element ** col_new     = new Element*[mxSize];
        unsigned int * colSize_new = new unsigned int[mxSize];
        unsigned int * colMax_new  = new unsigned int[mxSize];
        
        index_type ii = 0;
        if ( col )
        {
            for ( ; ii < mxAllocated; ++ii )
            {
                col_new[ii]     = col[ii];
                colSize_new[ii] = colSize[ii];
                colMax_new[ii]  = colMax[ii];
            }
            delete[] col;
            delete[] colSize;
            delete[] colMax;
        }
        
        for ( ; ii < mxSize; ++ii )
        {
            col_new[ii]     = 0;
            colSize_new[ii] = 0;
            colMax_new[ii]  = 0;
        }
        
        col       = col_new;
        colSize   = colSize_new;
        colMax    = colMax_new;
        mxAllocated = mxSize;
    }
}

//------------------------------------------------------------------------------
void MatrixSparseSymmetric::deallocate()
{
    if ( col )
    {
        for ( index_type ii = 0; ii < mxAllocated; ++ii )
        {
            if ( col[ii] )
                delete[] col[ii];
        }
        delete[] col;       col     = 0;
        delete[] colSize;   colSize = 0;
        delete[] colMax;    colMax  = 0;
    }
    mxAllocated = 0;
}

//------------------------------------------------------------------------------
void MatrixSparseSymmetric::allocateColumn( const index_type jj, unsigned int sz )
{
    assert_true( jj < mxSize );
    assert_true( sz > 0 );
    //printf("new S-COL %i %i\n", jj, sz );
    
    if ( sz > colMax[jj] )
    {
        const unsigned chunk = 4;
        sz = ( sz + chunk - 1 ) & -chunk;
        Element * col_new = new Element[sz];
        
        if ( col[jj] )
        {
            //copy what is there
            for ( unsigned int ii = 0; ii < colMax[jj]; ++ii )
                col_new[ii] = col[jj][ii];
            delete[] col[jj];
        }
        col[jj]    = col_new;
        colMax[jj] = sz;
    }
}

//------------------------------------------------------------------------------
real& MatrixSparseSymmetric::operator()( index_type ii, index_type jj )
{
    //this allocate the position if necessary
    assert_true( ii < mxSize );
    assert_true( jj < mxSize );
    
    Element * e, * last;
    
    //we swap to get the order right
    if ( jj < ii )
    {
        int tmp = ii;
        ii = jj;
        jj = tmp;
    }
    
    //check if the column is empty:
    if ( colSize[jj] == 0 )
    {
        allocateColumn(jj, 1);
        e = col[jj];
        //add requested term:
        e->line = ii;
        e->val  = 0.;
        colSize[jj] = 1;
        return e->val;
    }
    
    e = col[jj];
    last = e + colSize[jj];
    
    while ( e < last )
    {
        if ( e->line == ii )
            return e->val;
        ++e;
    }
    
    //we will have to create/allocate a new Element
    if ( colMax[jj] <= colSize[jj] )
    {
        allocateColumn( jj, colSize[jj]+1 );
        e = col[jj] + colSize[jj];
    }
    
    assert_true( colMax[jj] > colSize[jj] );
    
    //add the requested term:
    e->line = ii;
    e->val  = 0.;
    ++colSize[jj];
    return e->val;
}

//------------------------------------------------------------------------------
real* MatrixSparseSymmetric::addr( index_type ii, index_type jj ) const
{
    //we swap to get the order right
    if ( jj < ii )
    {
        int tmp = ii;
        ii  = jj;
        jj  = tmp;
    }
    
    for ( unsigned int kk = 0; kk < colSize[jj]; ++kk )
        if ( col[jj][kk].line == ii )
            return &( col[jj][kk].val );
    return 0;
}


//------------------------------------------------------------------------------
void MatrixSparseSymmetric::makeZero()
{
    for ( index_type ii = 0; ii < mxSize; ++ii )
        colSize[ii] = 0;
}


//------------------------------------------------------------------------------
void MatrixSparseSymmetric::scale(const real a)
{
    for ( index_type ii = 0; ii < mxSize; ++ii )
        for ( unsigned int jj = 0; jj < colSize[ii]; ++jj )
            col[ii][jj].val *= a;
}

//------------------------------------------------------------------------------
// M <- M + the upper block contained in [x, x+sx, x, x+sx]
void MatrixSparseSymmetric::addTriangularBlock(real* M, const index_type x, const unsigned int sx) const
{
    assert_true( x + sx <= mxSize );
    
    for ( index_type jj = 0; jj < sx; ++jj )
    {
        for ( unsigned int kk = 0; kk < colSize[jj+x]; ++kk )
        {
            index_type ii = col[jj+x][kk].line;
            if ( x <= ii  &&  ii < x+sx )
            {
                M[ ii-sx + sx * jj ] += col[jj+x][kk].val;
                //printf("Sp %4i %4i % .4f\n", ii, jj, a );
            }
        }
    }
}

//------------------------------------------------------------------------------
// M <- M + the upper block contained in [x, x+sx, x, x+sx]
void MatrixSparseSymmetric::addDiagonalBlock(real* M, const index_type x, const unsigned int sx) const
{
    assert_true( x + sx <= mxSize );
    
    for ( index_type jj = 0; jj < sx; ++jj )
    {
        for ( unsigned int kk = 0 ; kk < colSize[jj+x] ; ++kk )
        {
            index_type ii = col[jj+x][kk].line;
            if ( x <= ii )
            {
                ii  -= x;
                assert_true( ii <= jj );
                M[ii + sx * jj] += col[jj+x][kk].val;
                if ( ii != jj )
                    M[jj + sx * ii] += col[jj+x][kk].val;
                //printf("Sp %4i %4i % .4f\n", ii, jj, a );
            }
        }
    }
}


//------------------------------------------------------------------------------
int MatrixSparseSymmetric::bad() const
{
    if ( mxSize <= 0 ) return 1;
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
    {
        for ( unsigned int kk = 0 ; kk < colSize[jj] ; ++kk )
        {
            if ( col[jj][kk].line >= mxSize ) return 2;
            if ( col[jj][kk].line <= jj )   return 3;
        }
    }
    return 0;
}


//------------------------------------------------------------------------------
void MatrixSparseSymmetric::printSparse(std::ostream & os) const
{
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
        for ( unsigned int kk = 0 ; kk < colSize[jj] ; ++kk )
        {
            os << col[jj][kk].line << " " << jj << " ";
            os << std::setprecision(8) << col[jj][kk].val << std::endl;
        }
}


//------------------------------------------------------------------------------
bool MatrixSparseSymmetric::nonZero() const
{
    //check for any non-zero sparse term:
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
        for ( unsigned int kk = 0 ; kk < colSize[jj] ; ++kk )
            if ( col[jj][kk].val )
                return true;
    
    //if here, the matrix is empty
    return false;
}

//------------------------------------------------------------------------------
unsigned int MatrixSparseSymmetric::nbNonZeroElements() const
{
    //all allocated elements are counted, even if zero
    //the diagonal is not counted
    unsigned int cnt = 0;
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
        cnt += colSize[jj];
    return cnt;
}

//------------------------------------------------------------------------------
std::string MatrixSparseSymmetric::what() const
{
    std::ostringstream msg;
    msg << "SPS (nnz: " << nbNonZeroElements() << ")";
    return msg.str();
}

//------------------------------------------------------------------------------
void MatrixSparseSymmetric::prepareForMultiply()
{
}


//------------------------------------------------------------------------------
void MatrixSparseSymmetric::vecMulAdd( const real* X, real* Y ) const
{
    for ( index_type jj = 0; jj < mxSize; ++jj )
        for ( index_type kk = 0 ; kk < colSize[jj] ; ++kk )
        {
            const index_type ii = col[jj][kk].line;
            const real a = col[jj][kk].val;
            Y[ii] += a * X[jj];
            if ( ii != jj )
                Y[jj] += a * X[ii];
        }
}

//------------------------------------------------------------------------------
void MatrixSparseSymmetric::vecMulAddIso2D( const real* X, real* Y ) const
{
    const unsigned int D = 2;
    for ( index_type jj = 0; jj < mxSize; ++jj )
        for ( index_type kk = 0 ; kk < colSize[jj] ; ++kk )
        {
            const index_type ii = col[jj][kk].line;
            const real a = col[jj][kk].val;
            Y[D*ii  ] += a * X[D*jj  ];
            Y[D*ii+1] += a * X[D*jj+1];
            if ( ii != jj )
            {
                Y[D*jj  ] += a * X[D*ii  ];
                Y[D*jj+1] += a * X[D*ii+1];
            }
        }
}

//------------------------------------------------------------------------------
void MatrixSparseSymmetric::vecMulAddIso3D( const real* X, real* Y ) const
{
    const int D = 3;
    for ( index_type jj = 0; jj < mxSize; ++jj )
        for ( index_type kk = 0 ; kk < colSize[jj] ; ++kk )
        {
            const index_type ii = col[jj][kk].line;
            const real a = col[jj][kk].val;
            Y[D*ii  ] += a * X[D*jj  ];
            Y[D*ii+1] += a * X[D*jj+1];
            Y[D*ii+2] += a * X[D*jj+2];
            if ( ii != jj )
            {
                Y[D*jj  ] += a * X[D*ii  ];
                Y[D*jj+1] += a * X[D*ii+1];
                Y[D*jj+2] += a * X[D*ii+2];
            }
        }
}
