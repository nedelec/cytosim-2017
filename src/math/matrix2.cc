// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "matrix2.h"
#include "random.h"
#include "smath.h"

#define SZ 2

//-----------------------------------------------------------------------------------

real Matrix2::determinant() const
{
    return ( val[0] * val[3] - val[1] * val[2] );
}


Matrix2 Matrix2::inverted() const
{
    Matrix2 res;
    real det = determinant();
    det = 1.0 / det;
    res.val[0] =  det * val[3];
    res.val[1] = -det * val[1];
    res.val[2] = -det * val[2];
    res.val[3] =  det * val[0];
    return res;
}

//-----------------------------------------------------------------------------------

real Matrix2::rotationAngle() const
{
    return atan2( val[1+SZ*0], val[0+SZ*0] );
}


Matrix2 Matrix2::rotationFromEulerAngles(const real angle)
{
    Matrix2 res;
    real ca = cos(angle), sa = sin(angle);
    res.val[0] =  ca;
    res.val[1] =  sa;
    res.val[2] = -sa;
    res.val[3] =  ca;
    return res;
}


Matrix2 Matrix2::randomRotation(Random& rng)
{
    return rotationFromEulerAngles(M_PI*rng.sreal());
}


Matrix2 Matrix2::rotationToVector(const Vector2& vec)
{
    Matrix2 res;
    real n = vec.norm();
    real x = vec.XX / n;
    real y = vec.YY / n;
    res.val[0] =  x;
    res.val[1] =  y;
    res.val[2] = -y;
    res.val[3] =  x;
    return res;
}

