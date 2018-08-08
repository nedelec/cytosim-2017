// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MATSPARSE_H
#define MATSPARSE_H

#include <cstdio>
#include "matrix.h"

/// a real (non-symmetric) sparse Matrix
class MatrixSparse : public Matrix
{
    
private:
    
    /// size of the matrix
    unsigned int mxSize;
    
    /// size of memory which has been allocated
    unsigned int mxAllocated;
    
    
    // array [ size ][ ? ] holding the values for each column
    real ** mxCol;
    
    // array [ size ][ ? ] holding the line index for each column
    int  ** mxRow;
    
    // allocate column to hold nb values
    void allocateColumn( index_type column_index, unsigned int nb_values );
    
public:
    
    //size of (square) matrix
    unsigned int size() const { return mxSize; }

    /// base for destructor
    void deallocate();
    
    /// default constructor
    MatrixSparse();
    
    /// default destructor
    virtual ~MatrixSparse()  { deallocate(); }
    
    /// set all the element to zero
    void makeZero();
    
    /// allocate the matrix to hold ( sz * sz )
    void allocate( unsigned int sz );
        
    /// returns the address of element at (x, y), no allocation is done
    real* addr( index_type x, index_type y ) const;
    
    /// returns the address of element at (x, y), allocating if necessary
    real& operator()( index_type x, index_type y );
    
    /// scale the matrix by a scalar factor
    void scale( real a );
    
    /// add the diagonal block ( x, x, x+sx, x+sx ) from this matrix to M
    void addDiagonalBlock(real* M, index_type x, unsigned int sx) const;
    
    /// add the upper triagular block ( x, x, x+sx, x+sx ) from this matrix to M
    void addTriangularBlock(real* M, index_type x, unsigned int sx) const;
    
    /// multiplication of a vector: Y = Y + M * X, dim(X) = dim(M)
    void vecMulAdd( const real* X, real* Y ) const;
    
    /// 2D isotropic multiplication of a vector: Y = Y + M * X
    void vecMulAddIso2D( const real* X, real* Y ) const;
    
    /// 3D isotropic multiplication of a vector: Y = Y + M * X
    void vecMulAddIso3D( const real* X, real* Y ) const;
    
    /// true if matrix is non-zero
    bool nonZero() const;
    
    /// number of element which are non-zero
    unsigned int  nbNonZeroElements() const;
    
    /// returns a string which a description of the type of matrix
    std::string what() const;
    
    /// printf debug function in sparse mode: i, j : value
    void printSparse(std::ostream &) const;
    
    /// debug function
    int bad() const;
};


#endif
