// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "matsparsesym1.h"
#include "cblas.h"
#include "smath.h"

#include <iomanip>
#include <sstream>

#define MATRIX_USES_INTEL_SSE3 defined(__SSE3__) &&  !defined(REAL_IS_FLOAT)


//------------------------------------------------------------------------------


MatrixSparseSymmetric1::MatrixSparseSymmetric1()
{
    mxSize      = 0;
    mxAllocated = 0;
    
    col     = 0;
    colSize = 0;
    colMax  = 0;
    
#ifdef MATRIX_OPTIMIZE_MULTIPLY
    colF    = 0;
    nmax    = 0;
    ija     = 0;
    sa      = 0;
#endif
}



void MatrixSparseSymmetric1::allocate( const unsigned int sz )
{
    mxSize = sz;
    if ( mxSize > mxAllocated )
    {
        Element ** col_new     = new Element*[mxSize];
        unsigned int * colSize_new  = new unsigned int[mxSize];
        unsigned int * colMax_new  = new unsigned int[mxSize];
        
        unsigned int ii = 0;
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
        
#ifdef MATRIX_OPTIMIZE_MULTIPLY
        setColF(true);
#endif
    }
}


void MatrixSparseSymmetric1::deallocate()
{
    if ( col )
    {
        for ( unsigned int ii = 0; ii < mxAllocated; ++ii )
        {
            if ( col[ii] )
                delete[] col[ii];
        }
        delete[] col;       col     = 0;
        delete[] colSize;   colSize = 0;
        delete[] colMax;    colMax  = 0;
#ifdef MATRIX_OPTIMIZE_MULTIPLY
        delete[] colF;      colF = 0;
#endif
    }
    mxAllocated = 0;
}


MatrixSparseSymmetric1::Element * MatrixSparseSymmetric1::allocateColumn( const index_type jj, unsigned int sz )
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
            
            //release old memory
            delete[] col[jj];
        }
        col[jj]    = col_new;
        colMax[jj] = sz;
        return col_new;
    }
    
    return col[jj];
}

void MatrixSparseSymmetric1::printColumn( const index_type jj )
{
    printf("mat1 col %ul: ", jj);
    for ( unsigned int k = 0; k < colSize[jj]; ++k )
        printf("%ul ", col[jj][k].line);
    printf("\n");
}


//------------------------------------------------------------------------------

real& MatrixSparseSymmetric1::operator()( index_type ii, index_type jj )
{
    //this allocate the position if necessary
    assert_true( ii < mxSize );
    assert_true( jj < mxSize );
    
    //we swap to get the lower side
    if ( jj > ii )
    {
        index_type tmp = ii;
        ii = jj;
        jj = tmp;
    }
    
    Element * c;
    
    //check if the column is empty:
    if ( colSize[jj] == 0 )
    {
        c = allocateColumn( jj, 2 );
        
        //diagonal term always first:
        c->line = jj;
        c->val  = 0.;
        colSize[jj] = 1;
        
        if ( ii != jj )
        {
            //add the requested term:
            ++c;
            c->line = ii;
            c->val  = 0.;
            colSize[jj] = 2;
        }
        //printColumn(jj);
        
        return c->val;
    }
    
    c = col[jj];
    
    //check if diagonal term is requested
    if ( ii == jj )
    {
        assert_true( c->line == jj ); //the first term should be the diagonal
        return c->val;
    }
    
    Element * e = c + 1;
    Element * last = c + colSize[jj];
    
    
    //the indices are kept ordered in the column, in order to speed up things:
    while ( e < last )
    {
        if ( e->line == ii )
        {
            //std::cerr << "mat1 col " << jj << " line " << ii << " " << e->val << std::endl;
            return e->val;
        }
        if ( e->line > ii )
            break;
        ++e;
    }
    
    int indx = e - c;
    
    //allocate space for new Element if necessary:
    if ( colMax[jj] <= colSize[jj] )
    {
        c = allocateColumn( jj, colSize[jj]+1 );
        assert_true( colMax[jj] > colSize[jj] );
        e = c + indx;
    }
    
    // shift the entire column
    for ( int k = colSize[jj]; k > indx; --k )
        c[k] = c[k-1];
    ++colSize[jj];
    
    // add the requested term
    e->line = ii;
    e->val  = 0.;
    
    //printColumn(jj);
    
    return e->val;
}


real* MatrixSparseSymmetric1::addr( index_type ii, index_type jj ) const
{
    //we swap to get the order right
    if ( jj > ii )
    {
        index_type tmp = ii;
        ii  = jj;
        jj  = tmp;
    }
    
    for ( unsigned int kk = 0; kk < colSize[jj]; ++kk )
        if ( col[jj][kk].line == ii )
            return &( col[jj][kk].val );
    
    return 0;
}


//------------------------------------------------------------------------------
void MatrixSparseSymmetric1::makeZero()
{
    for ( unsigned int ii = 0; ii < mxSize; ++ii )
        colSize[ii] = 0;
}



void MatrixSparseSymmetric1::scale( const real a )
{
    for ( unsigned int ii = 0; ii < mxSize; ++ii )
        for ( unsigned int jj = 0; jj < colSize[ii]; ++jj )
            col[ii][jj].val *= a;
}


void MatrixSparseSymmetric1::addTriangularBlock(real* M, const index_type x, const unsigned int sx ) const
{
    assert_true( x + sx <= mxSize );
    
    for ( index_type jj = 0; jj < sx; ++jj )
    {
        for ( unsigned int kk = 0; kk < colSize[jj+x]; ++kk )
        {
            index_type ii = col[jj+x][kk].line;
            if ( x <= ii )
            {
                ii -= x;
                if ( ii < sx )
                {
                    if ( ii < jj )
                        M[ii+sx*jj] += col[jj+x][kk].val;
                    else
                        M[jj+sx*ii] += col[jj+x][kk].val;
                    //printf("Sp %4i %4i % .4f\n", ii, jj, a );
                }
            }
        }
    }
}


void MatrixSparseSymmetric1::addDiagonalBlock(real* M, const index_type x, const unsigned int sx ) const
{
    assert_true( x + sx <= mxSize );
    
    for ( index_type jj = 0; jj < sx; ++jj )
    {
        for ( unsigned int kk = 0 ; kk < colSize[jj+x] ; ++kk )
        {
            index_type ii = col[jj+x][kk].line;
            if ( x <= ii )
            {
                ii -= x;
                if ( ii < sx )
                {
                    M[ii+sx*jj] += col[jj+x][kk].val;
                    if ( ii != jj )
                        M[jj+sx*ii] += col[jj+x][kk].val;
                    //printf("Sp %4i %4i % .4f\n", ii, jj, a );
                }
            }
        }
    }
}


//------------------------------------------------------------------------------
int MatrixSparseSymmetric1::bad() const
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


void MatrixSparseSymmetric1::printSparse(std::ostream & os) const
{
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
    {
        for ( unsigned int kk = 0 ; kk < colSize[jj] ; ++kk )
        {
            os << col[jj][kk].line << " " << jj << " ";
            os << std::setprecision(8) << col[jj][kk].val << std::endl;
        }
    }
}


bool MatrixSparseSymmetric1::nonZero() const
{
    //check for any non-zero sparse term:
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
        for ( unsigned int kk = 0 ; kk < colSize[jj] ; ++kk )
            if ( col[jj][kk].val )
                return true;
    
    //if here, the matrix is empty
    return false;
}


unsigned int MatrixSparseSymmetric1::nbNonZeroElements() const
{
    //all allocated elements are counted, even if zero
    unsigned int cnt = 0;
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
        cnt += colSize[jj];
    return cnt;
}


std::string MatrixSparseSymmetric1::what() const
{
    std::ostringstream msg;
#if MATRIX_USES_INTEL_SSE3
    msg << "SPS1i (nnz: " << nbNonZeroElements() << ")";
#else
    msg << "SPS1 (nnz: " << nbNonZeroElements() << ")";
#endif
    return msg.str();
}


//------------------------------------------------------------------------------
#ifndef MATRIX_OPTIMIZE_MULTIPLY

void MatrixSparseSymmetric1::prepareForMultiply()
{
}


void MatrixSparseSymmetric1::vecMulAdd( const real* X, real* Y ) const
{
    for ( index_type jj = 0; jj < mxSize; ++jj )
    {
        for ( unsigned int kk = 0 ; kk < colSize[jj] ; ++kk )
        {
            const index_type ii = col[jj][kk].line;
            const real a = col[jj][kk].val;
            Y[ii] += a * X[jj];
            if ( ii != jj )
                Y[jj] += a * X[ii];
        }
    }
}


void MatrixSparseSymmetric1::vecMulAddIso2D( const real* X, real* Y ) const
{
    for ( index_type jj = 0, Djj=0; jj < mxSize; ++jj, Djj+=2 )
    {
        for ( unsigned int kk = 0 ; kk < colSize[jj] ; ++kk )
        {
            const index_type Dii = 2 * col[jj][kk].line;
            const real  a = col[jj][kk].val;
            Y[Dii  ] += a * X[Djj  ];
            Y[Dii+1] += a * X[Djj+1];
            if ( Dii != Djj )
            {
                Y[Djj  ] += a * X[Dii  ];
                Y[Djj+1] += a * X[Dii+1];
            }
        }
    }
}


void MatrixSparseSymmetric1::vecMulAddIso3D( const real* X, real* Y ) const
{
    for ( index_type jj = 0, Djj=0; jj < mxSize; ++jj, Djj+=3 )
    {
        for ( unsigned int kk = 0 ; kk < colSize[jj] ; ++kk )
        {
            const index_type Dii = 3 * col[jj][kk].line;
            const real  a =     col[jj][kk].val;
            Y[Dii  ] += a * X[Djj  ];
            Y[Dii+1] += a * X[Djj+1];
            Y[Dii+2] += a * X[Djj+2];
            if ( Dii != Djj )
            {
                Y[Djj  ] += a * X[Dii  ];
                Y[Djj+1] += a * X[Dii+1];
                Y[Djj+2] += a * X[Dii+2];
            }
        }
    }
}

#else  // MATRIX_OPTIMIZE_MULTIPLY


void MatrixSparseSymmetric1::setColF(bool alloc)
{
    if ( alloc )
    {
        //allocate and initialize the non-empty column index array
        if ( colF )
            delete[] colF;
        colF = new index_type[mxAllocated+1];
        for ( index_type jj = 0; jj <= mxAllocated; ++jj )
            colF[jj] = jj;
    }
    else
    {
        //update colF[], a pointer to the next non-empty column:
        assert_true( mxSize <= mxAllocated );
        index_type inx = mxSize;
        colF[mxSize] = mxSize;
        for ( int jj = mxSize-1; jj >= 0; --jj )
        {
            if ( colSize[jj] > 0 )
                inx = jj;
            colF[jj] = inx;
        }
    }
}


void MatrixSparseSymmetric1::prepareForMultiply()
{
    setColF(false);
    
    //count number of non-zero elements, including diagonal
    unsigned int nbe = 1;
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
    {
        if ( colSize[jj] > 0 )
            nbe += colSize[jj];
        else
            nbe ++;
    }
    
    //allocate classical sparse matrix storage (Numerical Recipes)
    if ( nbe > nmax )
    {
        if ( ija )  delete[] ija;
        if ( sa )   delete[] sa;
        
        nmax  = nbe + mxSize;
        ija   = new index_type[nmax];
        sa    = new real[nmax];
    }
    
    //create the sparse representation, described in numerical-recipe
    //indices start at zero, unlike in numerical recipe
    ija[0] = mxSize+1;
    index_type kk = mxSize;
    for ( unsigned int jj = 0; jj < mxSize; ++jj )
    {
        if ( colSize[jj] > 0 )
        {
            // diagonal term first:
            assert_true( col[jj][0].line == jj );
            sa[jj]  = col[jj][0].val;
            // other elements:
            for ( unsigned int cc = 1; cc < colSize[jj]; ++cc )
            {
                ++kk;
                if ( kk >= nbe ) ABORT_NOW("out-of-range");
                sa[kk]  = col[jj][cc].val;
                ija[kk] = col[jj][cc].line;
            }
        }
        else {
            sa[jj] = 0;
        }
        ija[jj+1] = kk+1;
    }
    assert_true( kk+1 == nbe );
}


void MatrixSparseSymmetric1::vecMulAdd( const real* X, real* Y ) const
{
    for ( index_type jj = colF[0]; jj < mxSize; jj = colF[jj+1] )
    {
        real X0 = X[jj];
        real Y0 = Y[jj] + sa[jj] * X0;
        const index_type end = ija[jj+1];
        for ( index_type kk = ija[jj]; kk < end; ++kk )
        {
            real a = sa[kk];
            index_type ii = ija[kk];
            Y[ii] += a * X0;
            Y0    += a * X[ii];
        }
        Y[jj] = Y0;
    }
}

//------------------------------------------------------------------------------

#if (MATRIX_USES_INTEL_SSE3)

#include <pmmintrin.h>
#warning "Manual SSE3 code in MatrixSparseSymmetric1"

#define SSE(x) _mm_##x##_pd

void MatrixSparseSymmetric1::vecMulAddIso2D( const real* X, real* Y ) const
{
    for ( index_type jj = colF[0]; jj < mxSize; jj = colF[jj+1] )
    {
        __m128d x, y, a, t;
        x = SSE(load)(X+2*jj);
        a = SSE(loaddup)(sa+jj);
        y = SSE(add)(_mm_load_pd(Y+2*jj), SSE(mul)(a, x));
        const index_type end = ija[jj+1];
        for ( index_type kk = ija[jj]; kk < end; ++kk )
        {
            a = SSE(loaddup)(sa+kk);
            t = SSE(add)(SSE(load)(Y+2*ija[kk]), SSE(mul)(x, a));
            y = SSE(add)(y, SSE(mul)(SSE(load)(X+2*ija[kk]), a));
            SSE(store)(Y+2*ija[kk], t);
        }
        SSE(store)(Y+2*jj, y);
    }
}

#else

void MatrixSparseSymmetric1::vecMulAddIso2D( const real* X, real* Y ) const
{    
    for ( index_type jj = colF[0]; jj < mxSize; jj = colF[jj+1] )
    {
        index_type Djj = 2 * jj;
        real X0 = X[Djj  ];
        real X1 = X[Djj+1];
        real Y0 = Y[Djj  ] + sa[jj] * X0;
        real Y1 = Y[Djj+1] + sa[jj] * X1;
        const index_type end = ija[jj+1];
        for ( index_type kk = ija[jj]; kk < end; ++kk )
        {
            index_type Dii = 2 * ija[kk];
            assert_true( Djj != Dii );
            real a = sa[kk];
            Y0       += a * X[Dii  ];
            Y1       += a * X[Dii+1];
            Y[Dii  ] += a * X0;
            Y[Dii+1] += a * X1;
        }
        Y[Djj  ] = Y0;
        Y[Djj+1] = Y1;
    }
}

#endif


void MatrixSparseSymmetric1::vecMulAddIso3D( const real* X, real* Y ) const
{
    for ( index_type jj = colF[0]; jj < mxSize; jj = colF[jj+1] )
    {
        index_type Djj = 3 * jj;
        real X0 = X[Djj  ];
        real X1 = X[Djj+1];
        real X2 = X[Djj+2];
        real Y0 = Y[Djj  ] + sa[jj] * X0;
        real Y1 = Y[Djj+1] + sa[jj] * X1;
        real Y2 = Y[Djj+2] + sa[jj] * X2;
        const index_type next = ija[jj+1];
        for ( index_type kk = ija[jj]; kk < next; ++kk )
        {
            index_type Dii = 3 * ija[kk];
            assert_true( Djj != Dii );
            real a = sa[kk];
            Y0       += a * X[Dii  ];
            Y1       += a * X[Dii+1];
            Y2       += a * X[Dii+2];
            Y[Dii  ] += a * X0;
            Y[Dii+1] += a * X1;
            Y[Dii+2] += a * X2;
        }
        Y[Djj  ] = Y0;
        Y[Djj+1] = Y1;
        Y[Djj+2] = Y2;
    }
}

#endif

