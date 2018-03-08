// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#include "real.h"
#include "smath.h"
#include "assert_macro.h"
#include <iostream>
#include <iomanip>
class Random;

/// Fortran-style matrices of small dimensions: 1, 2 and 3
/** 

 Square matrices of small dimensions.
 
 The matrice's elements are stored in a one dimensional array to be
 compatible with FORTRAN routines
 Matrices for small dimensions: 1 2 or 3 can be used to represent
 transformations of the Vector class
 
 */

template < unsigned SZ >
class MatrixBase
{
    
public:
    
    /// the array of value, column-major
    real val[SZ*SZ];
    
public:
    
    /// The default creator does not initialize
    MatrixBase() {}
    
    /// The default destructor does nothing
    virtual ~MatrixBase() {}
    
    
    /// Copy-creator from a C-style array
    MatrixBase(real v[SZ][SZ]) 
    {
        for ( unsigned i=0; i<SZ; ++i )
            for ( unsigned j=0; j<SZ; ++j )
                val[i + SZ * j] = v[i][j];
    }
    
    /// Copy-creator from a Fortran-style array
    MatrixBase(real v[SZ*SZ]) 
    {
        for ( unsigned ii = 0; ii < SZ*SZ; ++ii )
            val[ii] = v[ii];
    }
    
    /// set all components to zero
    void makeZero() 
    {
        for ( unsigned ii = 0; ii < SZ*SZ; ++ii )
            val[ii] = 0;
    }
    
    /// set to Identity (ones on the diagonal, zero elsewhere)
    void makeIdentity() 
    {
        makeZero();
        for ( unsigned ii = 0; ii < SZ*SZ; ii += SZ+1 )
            val[ii] = 1.;
    }

    
    /// conversion to a modifiable real array
    operator real*() 
    {
        return val;
    }
    
    /// access to modifiable elements of the matrix by (line, column)
    real& operator()(const unsigned ii, const unsigned jj)
    {
        return val[ii + SZ * jj];
    }
    
    /// access to constant elements of the matrix by (line, column)
    real const& operator()(const unsigned ii, const unsigned jj) const
    {
        return val[ii + SZ * jj];
    }
    
    /// ii-th modifiable elements in the matrix
    real& operator[](const unsigned ii)
    {
        return val[ii];
    }
    
    /// ii-th constant element in the matrix
    real operator[](const unsigned ii) const
    {
        return val[ii];
    }

    /// return the transposed matrix
    MatrixBase transposed() const 
    {
        MatrixBase res;
        for ( unsigned ii = 0; ii < SZ; ++ii )
            for ( unsigned jj = 0; jj < SZ; ++jj )
                res.val[ii + SZ*jj] = val[jj + SZ*ii];
        return res;
    }
    
    /// transpose this matrix
    void transpose() 
    {
        real x;
        for ( unsigned ii = 0; ii < SZ; ++ii )
            for ( unsigned jj = 0; jj < ii; ++jj )
            {
                x = val[ii + SZ*jj];
                val[ii + SZ*jj] = val[jj + SZ*ii];
                val[jj + SZ*ii] = x;
            }
    }
    
    /// maximum of all fabs(element)
    real maxNorm() const 
    {
        real res = 0;
        for ( unsigned ii = 0; ii < SZ*SZ; ++ii )
        {
            if ( fabs( val[ii] ) > res )
                res = fabs( val[ii] );
        }
        return res;
    }
    
    /// formatted output
    void write(std::ostream & os) const 
    {
        for ( unsigned ii = 0; ii < SZ; ++ii )
        {
            for ( unsigned jj = 0; jj < SZ; ++jj )
                os << std::setw(9) << std::setprecision(4) << val[ii+SZ*jj];
            os << std::endl;
        }
    }
    
    /// calculate a distance to the subspace of rotations = maxNorm( M'*M - Id )
    real maxDeviationFromRotation() const
    {
        MatrixBase mm = transposed() * (*this);
        for ( unsigned ii = 0; ii < SZ; ++ii )
            mm( ii, ii ) -= 1.0;
        return mm.maxNorm();
    }
    
    /// returns the matrix multiplied by a real scalar
    friend MatrixBase operator * (const MatrixBase & a, const real b)
    {
        MatrixBase res;
        for ( unsigned ii = 0; ii < SZ*SZ; ++ii )
            res.val[ii] = a.val[ii] * b;
        return res;
    }
    
    /// returns the matrix multiplied by the real scalar a
    friend MatrixBase operator * (const real a, const MatrixBase & b)
    {
        MatrixBase res;
        for ( unsigned ii = 0; ii < SZ*SZ; ++ii )
            res.val[ii] = a * b.val[ii];
        return res;
    }
    
    /// multiply the matrix by the real scalar a
    void operator *=(const real a)
    {
        for ( unsigned ii = 0; ii < SZ*SZ; ++ii )
            val[ii] *= a;
    }
    
    /// divide the matrix by the real scalar a
    void operator /=(const real a)
    {
        for ( unsigned ii = 0; ii < SZ*SZ; ++ii )
            val[ii] /= a;
    }
    
    /// add matrix m
    void operator += (const MatrixBase m)
    {
        for ( unsigned ii = 0; ii < SZ*SZ; ++ii )
            val[ii] += m.val[ii];
    }
    
    /// subtract matrix m
    void operator -= (const MatrixBase m)
    {
        for ( unsigned ii = 0; ii < SZ*SZ; ++ii )
            val[ii] -= m.val[ii];
    }
    
    /// add two matrices and return the result
    friend MatrixBase operator + (const MatrixBase & a, const MatrixBase & b)
    {
        MatrixBase res;
        for ( unsigned ii = 0; ii < SZ*SZ; ++ii )
            res.val[ii] = a.val[ii] + b.val[ii];
        return res;
    }
    
    /// substract two matrices and return the result
    friend MatrixBase operator - (const MatrixBase & a, const MatrixBase & b)
    {
        MatrixBase res;
        for ( unsigned ii = 0; ii < SZ*SZ; ++ii )
            res.val[ii] = a.val[ii] - b.val[ii];
        return res;
    }
    
    /// multiply two matrices and return the result
    friend MatrixBase operator * (const MatrixBase & a, const MatrixBase & b)
    {
        MatrixBase res;
        for ( unsigned ii = 0; ii < SZ; ++ii )
            for ( unsigned jj = 0; jj < SZ; ++jj )
            {
                real x = a.val[ii] * b.val[SZ*jj];
                for ( unsigned kk = 1; kk < SZ; ++kk )
                    x += a.val[ii+SZ*kk] * b.val[kk+SZ*jj];
                res.val[ii+SZ*jj] = x;
            }
        return res;
    }
    
    /// Vector multiplication: out <- M * in
    void vecMul(const real* in, real* out) const
    {
        for ( unsigned ii = 0; ii < SZ; ++ii )
        {
            real x = val[ii] * in[0];
            for ( unsigned jj = 1; jj < SZ; ++jj )
                x += val[ii + SZ*jj] * in[jj];
            out[ii] = x;
        }
    }
    
    /// Vector multiplication: vec <- M * vec
    void vecMul(real* vec) const
    {
        real copy[SZ];
        for ( unsigned ii = 0; ii < SZ; ++ii )
            copy[ii] = vec[ii];
        vecMul(copy, vec);
    }
    
    
    //------------------------------- STATIC ----------------------------------------
    
    
    /// return identity matrix (ones on the diagonal, zero elsewhere)
    static MatrixBase one()
    {
        MatrixBase res;
        res.makeIdentity();
        return res;
    }
    
    /// return zero matrix
    static MatrixBase zero()
    {
        MatrixBase res;
        res.makeZero();
        return res;
    }
    
    /// build the projection matrix V * V'
    static MatrixBase projectionMatrix(const real V[])
    {
        MatrixBase res;
        for ( unsigned ii = 0; ii < SZ; ++ii )
        {
            for ( unsigned jj = 0; jj < SZ; ++jj )
                res.val[ii+SZ*jj] = V[ii] * V[jj];
        }
        return res;
    }
};

/// output
template < unsigned SZ >
inline std::ostream & operator << (std::ostream & os, const MatrixBase<SZ>& mat)
{
    mat.write(os);
    return os;
}

