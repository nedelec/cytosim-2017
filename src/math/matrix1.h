// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MATRIX1_H
#define MATRIX1_H

#include "matrixbase.h"
#include "vector1.h"

class Matrix1 : public MatrixBase<1>
{
public:

    ///constructor
    Matrix1() {};
    
    /// automatic conversion from MatrixBase<1>
    Matrix1(MatrixBase<1> const& m) : MatrixBase<1>(m) {}

    
    /// extract column vector at index `jj`
    Vector1 getColumn(const unsigned) const
    {
        return Vector1(val[0], 0);
    }

    /// rotation angle
    real rotationAngle() const;
    
    /// rotation from angle
    static Matrix1 rotationFromEulerAngles(real a);

    
    /// return a rotation that transforms (1,0,0) into \a vec ( norm(vec) should be > 0 )
    static Matrix1 rotationToVector(const real vec[1]);
    
    /// return a random rotation that transforms (1,0,0) into \a vec ( norm(vec) should be > 0 )
    static Matrix1 rotationToVector(const real vec[1], Random&)
    {
        return rotationToVector(vec);
    }
    
    /// a random rotation chosen uniformly
    static Matrix1 randomRotation(Random&);

};


inline Vector1 operator * (const Matrix1 & a, const Vector1 & v)
{
    return Vector1(a.val[0] * v[0], 0);
}

#endif
