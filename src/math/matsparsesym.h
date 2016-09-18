// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MATSPARSESYM_H
#define MATSPARSESYM_H

#include <cstdio>
#include "matrix.h"


///real symmetric sparse Matrix
/**
MatrixSparseSymmetric uses a sparse storage, with arrays of elements for each column.
 */
class MatrixSparseSymmetric : public Matrix
{
    
private:
    
    ///Element describes an element in a sparse matrix
    // The elements are stored per columns, that is how the column index is known
    struct Element {
        real val;    ///< The value of the element
        index_type line;   ///< The index of the line
    };

private:
    /// size of matrix
    unsigned int mxSize;
    
    /// amount of memory which has been allocated
    unsigned int mxAllocated;
            
    /// array col[c][] holds Elements of column 'c'
    Element ** col;
    
    /// colSize[c] is the number of Elements in column 'c'
    unsigned int  * colSize;
    
    /// colMax[c] number of Elements allocated in column 'c'
    unsigned int  * colMax;
    
    /// allocate column to hold specified number of values
    void allocateColumn( index_type column_index, unsigned int nb_values );
    
public:
    
    //size of (square) matrix
    unsigned int size() const { return mxSize; }

    /// base for destructor
    void deallocate();
    
    /// default constructor
    MatrixSparseSymmetric();
    
    /// default destructor
    virtual ~MatrixSparseSymmetric()  { deallocate(); }
    
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
    void addDiagonalBlock( real* M, index_type x, unsigned int sx) const;
    
    /// add the upper triagular block ( x, x, x+sx, x+sx ) from this matrix to M
    void addTriangularBlock( real* M, index_type x, unsigned int sx) const;
    
    ///optional optimization that may accelerate multiplications by a vector
    void prepareForMultiply();
    
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
