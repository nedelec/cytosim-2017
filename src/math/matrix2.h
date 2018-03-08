// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MATRIX2_H
#define MATRIX2_H


#include "matrixbase.h"
#include "vector2.h"

class Matrix2 : public MatrixBase<2>
{
public:
    
    ///constructor
    Matrix2() {};

    /// automatic conversion from MatrixBase<2>
    Matrix2(MatrixBase<2> const& m) : MatrixBase<2>(m) { }
    
    
    /// extract column vector at index `n`
    Vector2 getColumn(const unsigned n) const
    {
        if ( n == 0 )
            return Vector2(val[0], val[1]);
        else
            return Vector2(val[2], val[3]);
    }
    
    /// return the determinant of the matrix
    real determinant() const;
    
    /// return the inverse of the matrix
    Matrix2 inverted() const;

    /// rotation angle
    real rotationAngle() const;
    
    /// calculate rotation angle and Euler angle of axis
    static Matrix2 rotationFromEulerAngles(real a);
    
    
    /// a rotation around the given axis
    static Matrix2 rotationAroundAxis(real dir, real angle)
    {
        if ( dir > 0 )
            return rotationFromEulerAngles(angle);
        else
            return rotationFromEulerAngles(-angle);
    }
    
    /// return a rotation that transforms (1,0,0) into \a vec ( norm(vec) should be > 0 )
    static Matrix2 rotationToVector(const Vector2& vec);
    
    /// return a random rotation that transforms (1,0,0) into \a vec ( norm(vec) should be > 0 )
    static Matrix2 rotationToVector(const Vector2& vec, Random&)
    {
        return rotationToVector(vec);
    }
    
    /// a random rotation chosen uniformly
    static Matrix2 randomRotation(Random&);

};


inline Vector2 operator * (const Matrix2 & a, const Vector2 & v)
{
    return Vector2( a.val[0] * v[0] + a.val[2] * v[1],
                    a.val[1] * v[0] + a.val[3] * v[1] );
}

#endif
