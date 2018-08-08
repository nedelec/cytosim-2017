// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MATRIX_H
#define MATRIX_H

#include "assert_macro.h"
#include <iostream>
#include "real.h"

/// The interface for all the large matrices
class Matrix
{
public:
    
    /// type of an index into the matrix
    typedef unsigned int index_type;
    
private:
    
    ///\todo add Matrix copy constructor and copy assignment
    
    /// Disabled copy constructor
    Matrix(Matrix const&);
    
    /// Disabled copy assignment
    Matrix& operator=(Matrix const&);
    
public:
    
    /// empty constructor
    Matrix() {}
    
    /// empty destructor
    virtual ~Matrix() {}
    
    //----------------------------------------------------------------------
    
    
    /// allocate the matrix to hold ( sz * sz ), all values are lost
    virtual void   allocate( unsigned int sz ) = 0;
        
    /// returns the address of element at (x, y), no allocation is done
    virtual real*  addr( index_type x, index_type y ) const = 0;
    
    /// returns the address of element at (x, y), allocating if necessary
    virtual real&  operator()( index_type x, index_type y ) = 0;
    
    /// returns the value of element at (x, y) or zero if not allocated
    real value( index_type x, index_type y ) const;
    
    //----------------------------------------------------------------------
    
    /// returns the size of the matrix
    virtual unsigned int size() const = 0;
    
    /// set all the elements to zero
    virtual void makeZero() = 0;
    
    /// scale the matrix by a scalar factor
    virtual void scale( real ) = 0;
    
    /// copy the block ( x, y, x+sx, y+sy ) from this matrix into M
    void copyBlock(real* M, index_type x, unsigned int sx, index_type y, unsigned int sy) const;
    
    /// add the block ( x, x, x+sx, x+sx ) from this matrix to M
    virtual void addDiagonalBlock(real* M, index_type x, unsigned int sx) const;
    
    /// add the upper triangular half of the block ( x, x, x+sx, x+sx ) from this matrix to M
    virtual void addTriangularBlock(real* M, index_type x, unsigned int sx) const;
    
    //----------------------------------------------------------------------
    
    /// Optional optimization to accelerate multiplications below
    virtual void prepareForMultiply() {}
    
    /// Vector multiplication: Y <- Y + M * X, size(X) = size(Y) = size(M)
    virtual void vecMulAdd(const real* X, real* Y) const = 0;
    /// Vector multiplication: Y <- M * X, size(X) = size(Y) = size(M)
    virtual void vecMul(const real* X, real* Y) const;

    /// isotropic vector multiplication: Y = Y + M * X, size(X) = size(Y) = 2 * size(M)
    virtual void vecMulAddIso2D(const real*, real*) const = 0;
    /// isotropic vector multiplication: Y = Y + M * X, size(X) = size(Y) = 3 * size(M)
    virtual void vecMulAddIso3D(const real*, real*) const = 0;
    
    //----------------------------------------------------------------------
    
    /// maximum absolute value considering all the elements
    virtual real maxNorm() const;
    
    /// true if the matrix is non-zero
    virtual bool nonZero() const;
    
    /// number of element which are non-zero
    virtual unsigned int nbNonZeroElements() const;
    
    /// returns a string which a description of the type of matrix
    virtual std::string what() const = 0;
    
    /// printf debug function in sparse mode: i, j : value
    virtual void printSparse(std::ostream &) const;
    
    /// printf debug function in full lines, all columns
    virtual void printFull(std::ostream &) const;
    
};


#endif
