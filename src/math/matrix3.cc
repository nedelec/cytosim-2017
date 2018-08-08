// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "matrix3.h"
#include "random.h"
#include "smath.h"

#define SZ 3

//-----------------------------------------------------------------------------------

real Matrix3::determinant() const
{
    return ( + val[0]*val[4]*val[8] + val[2]*val[3]*val[7] + val[1]*val[5]*val[6]
             - val[2]*val[4]*val[6] - val[1]*val[3]*val[8] - val[0]*val[5]*val[7] );
}


Matrix3 Matrix3::inverted() const
{
    Matrix3 res;
    real det = determinant();
    det = 1.0 / det;
    Vector3 v0 = getColumn(0);
    Vector3 v1 = getColumn(1);
    Vector3 v2 = getColumn(2);
    res.setLines(vecProd(v1,v2)*det, vecProd(v2,v0)*det, vecProd(v0,v1)*det);
    return res;
}

//-----------------------------------------------------------------------------------


void Matrix3::getEulerAngles(real& a, real& b, real& c) const
{
    real cb = sqrt(val[0] * val[0] + val[1] * val[1]);
    
    b = atan2( -val[2+SZ*0], cb );
    
    if ( cb ) {
        a = atan2( val[1+SZ*0], val[0+SZ*0] );
        c = atan2( val[2+SZ*1], val[2+SZ*2] );
    }
    else {
        a = 0;
        c = atan2( -val[0+SZ*1], val[1+SZ*1] );
    }
}

Matrix3 Matrix3::rotationAroundX(const real angle)
{
    real ca = cos(angle);
    real sa = sin(angle);
    
    Matrix3 res;
    res.makeZero();
    res.val[0] = 1;
    res.val[4] = ca;
    res.val[8] = ca;
    res.val[7] = -sa;
    res.val[5] = sa;
    
    return res;
}


Matrix3 Matrix3::rotationAroundY(const real angle)
{
    real ca = cos(angle);
    real sa = sin(angle);
    
    Matrix3 res;
    res.makeZero();
    res.val[0] = ca;
    res.val[4] = 1;
    res.val[8] = ca;
    res.val[2] = -sa;
    res.val[6] = sa;
    
    return res;
}

Matrix3 Matrix3::rotationAroundZ(const real angle)
{
    real ca = cos(angle);
    real sa = sin(angle);
    
    Matrix3 res;
    res.makeZero();
    res.val[0] = ca;
    res.val[4] = ca;
    res.val[8] = 1;
    res.val[3] = -sa;
    res.val[1] = sa;
    
    return res;
}

Matrix3 Matrix3::rotationAroundPrincipalAxis(unsigned ii, const real angle)
{
    real ca = cos(angle);
    real sa = sin(angle);
    
    int jj = (ii+1)%3;
    int kk = (ii+2)%3;
    
    Matrix3 res;
    res.makeZero();
    res.val[ii+SZ*ii] = 1;
    res.val[jj+SZ*jj] = ca;
    res.val[kk+SZ*kk] = ca;
    res.val[jj+SZ*kk] = -sa;
    res.val[kk+SZ*jj] = sa;

    return res;
}


Matrix3 Matrix3::rotationFromEulerAngles(const real a[3])
{
    real ca = cos(a[0]), sa = sin(a[0]);
    real cb = cos(a[1]), sb = sin(a[1]);
    real cc = cos(a[2]), sc = sin(a[2]);
    
    Matrix3 res;

    res.val[0+SZ*0] =  ca*cb;
    res.val[1+SZ*0] =  sa*cb;
    res.val[2+SZ*0] = -sb;
    
    res.val[0+SZ*1] =  ca*sb*sc - sa*cc;
    res.val[1+SZ*1] =  sa*sb*sc + ca*cc;
    res.val[2+SZ*1] =  cb*sc;
    
    res.val[0+SZ*2] =  ca*sb*cc + sa*sc;
    res.val[1+SZ*2] =  sa*sb*cc - ca*sc;
    res.val[2+SZ*2] =  cb*cc;
    
    return res;
}


Matrix3 Matrix3::rotationAroundAxisEuler(const real a[3])
{
    real ca = cos(a[0]), sa = sin(a[0]), ca1 = 1 - ca;
    real cb = cos(a[1]), sb = sin(a[1]);
    real cc = cos(a[2]), sc = sin(a[2]);
    
    real sacc        = sa * cc,           sasc        = sa * sc;
    real saccsb      = sacc * sb,         sacccb      = sacc * cb;
    real ccccca1     = cc * cc * ca1,     ccscca1     = cc * sc * ca1;
    real sbccscca1   = sb * ccscca1,      cbccscca1   = cb * ccscca1;
    real cbcbccccca1 = cb * cb * ccccca1, cbsbccccca1 = cb * sb * ccccca1;
    
    Matrix3 res;
    res.val[0+SZ*0] =  cbcbccccca1 + ca;
    res.val[0+SZ*1] =  cbsbccccca1 - sasc;
    res.val[0+SZ*2] =  cbccscca1   + saccsb;
    
    res.val[1+SZ*0] =  cbsbccccca1 + sasc;
    res.val[1+SZ*1] =  ca - cbcbccccca1 + ccccca1;
    res.val[1+SZ*2] =  sbccscca1   - sacccb;
    
    res.val[2+SZ*0] =  cbccscca1 - saccsb;
    res.val[2+SZ*1] =  sbccscca1 + sacccb;
    res.val[2+SZ*2] =  1 - ccccca1;
    
    return res;
}


Matrix3 Matrix3::rotationAroundAxis(const Vector3& axis, const real angle)
{
    Matrix3 R;
    Vector3 X = axis.normalized();
    Vector3 Y = X.orthogonal(1);
    Vector3 Z = vecProd(X,Y);
    R.setColumns(X, Y, Z);
    return R * rotationAroundX(angle) * R.transposed();
}


Matrix3 Matrix3::rotationAroundAxis(const Vector3& axis)
{
    Matrix3 R;
    real n = axis.norm();
    Vector3 X = axis / n;
    Vector3 Y = X.orthogonal(1);
    Vector3 Z = vecProd(X,Y);
    R.setColumns(X, Y, Z);
    return R * rotationAroundX(n) * R.transposed();
}


Matrix3 Matrix3::randomRotation(Random& rng)
{
    //James Arvo, Fast random rotation matrices. in Graphics Gems 3.
    real u2 = M_PI * rng.sreal();
    real u3 = rng.preal();
    Vector3 V( cos(u2)*sqrt(u3), sin(u2)*sqrt(u3), sqrt(1-u3) );
    Matrix3 R = rotationAroundPrincipalAxis(2, M_PI*rng.sreal());
    return ( 2 * Matrix3::projectionMatrix(V) - Matrix3::one() ) * R;
}


Matrix3 Matrix3::rotationToVector(const Vector3& vec)
{
    Matrix3 res;
    Vector3 V1 = vec.normalized();
    Vector3 V2 = V1.orthogonal(1);
    Vector3 V3 = vecProd(V1, V2);
    res.setColumns(V1, V2, V3);
    return res;
}


Matrix3 Matrix3::rotationToVector(const Vector3& vec, Random& rng)
{
    return rotationToVector(vec) * rotationAroundX(M_PI*rng.sreal());
}

