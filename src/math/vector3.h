// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef VECTOR3_H
#define VECTOR3_H


#include "real.h"
#include "assert_macro.h"

#include <iostream>
#include <sstream>
#include <cstdio>
#include <cmath>

/// Vector3 is a vector with 3 `real` components.
/**
 We assume that the coordinates XX, YY and ZZ are adjacent in memory, which allows
 us to define an automatic conversion to a C-array.
 This is not guaranteed by the C-standard, but it usually works though...
 */
class Vector3
{
    
public:
    
    /// dimensionality
    static unsigned dimensionality() { return 3; }
    
    /// coordinates are public
    real XX, YY, ZZ;
    
    
    /// by default, coordinates are not initialized
    Vector3() {}
    
    /// construct from values
    Vector3(const real x, const real y, const real z) : XX(x), YY(y), ZZ(z) {}
    
    /// construct from address
    Vector3(const real v[]) : XX(v[0]), YY(v[1]), ZZ(v[2]) {}

    /// construct from array of size d
    Vector3(const real v[], const int& d)
    {
        if ( d > 0 ) XX = v[0]; else XX = 0;
        if ( d > 1 ) YY = v[1]; else YY = 0;
        if ( d > 2 ) ZZ = v[2]; else ZZ = 0;
    }

    /// create new Vector with coordinates from the given array
    static Vector3 make(const real b[]) { return Vector3(b[0], b[1], b[2]); }

    /// destructor (is not-virtual: do not derive from this class)
    ~Vector3() {}
    
    
    /// implicit conversion to a modifiable real[] array
    operator real*()             { return &XX; }
    
    /// implicit conversion to a const real[] array
    operator const real*() const { return &XX; }
    
    /// conversion to a 'real array'
    real * addr()                { return &XX; }
    
    /// modifiable access to individual coordinates
    real& operator[](const unsigned ii)
    {
        assert_true(ii<3);
        return (&XX)[ii];
    }

    
    /// replace coordinates by the ones provided
    void get(const real b[])
    {
        XX = b[0];
        YY = b[1];
        ZZ = b[2];
    }
    
    /// copy coordinates from array of size d
    void get(const real v[], const int& d)
    {
        if ( d > 0 ) XX = v[0]; else XX = 0;
        if ( d > 1 ) YY = v[1]; else YY = 0;
        if ( d > 2 ) ZZ = v[2]; else ZZ = 0;
    }

    /// copy coordinates to given array
    void put(real b[]) const
    {
        b[0] = XX;
        b[1] = YY;
        b[2] = ZZ;
    }
    
    /// add to given array
    void addTo(real b[]) const
    {
        b[0] += XX;
        b[1] += YY;
        b[2] += ZZ;
    }
    
    /// change coordinates by adding given array
    void addFrom(real b[])
    {
        XX += b[0];
        YY += b[1];
        ZZ += b[2];
    }
    
    /// subtract to given array
    void subTo(real b[]) const
    {
        b[0] -= XX;
        b[1] -= YY;
        b[2] -= ZZ;
    }
    
    /// change coordinates by subtracting given array
    void subFrom(real b[])
    {
        XX -= b[0];
        YY -= b[1];
        ZZ -= b[2];
    }
    
    /// change coordinates
    void set(const real x, const real y, const real z)
    {
        XX = x;
        YY = y;
        ZZ = z;
    }

    /// change signs of all coordinates
    void oppose()
    {
        XX = -XX;
        YY = -YY;
        ZZ = -ZZ;
    }
    
    //------------------------------------------------------------------
    
    /// the square of the standard norm
    real normSqr() const
    {
        return XX*XX + YY*YY + ZZ*ZZ;
    }

    
    /// the standard norm = sqrt(x^2+y^2+z^2)
    real norm() const
    {
        return sqrt(XX*XX + YY*YY + ZZ*ZZ);
    }

    /// the 2D norm = sqrt(x^2+y^2)
    real normXY() const
    {
        return sqrt(XX*XX + YY*YY);
    }

    /// the 2D norm = sqrt(x^2+z^2)
    real normXZ() const
    {
        return sqrt(XX*XX + ZZ*ZZ);
    }
    
    /// the 2D norm = sqrt(y^2+z^2)
    real normYZ() const
    {
        return sqrt(YY*YY + ZZ*ZZ);
    }

    /// square of the distance to other point == (a-this).normSqr()
    real distanceSqr(Vector3 const& a) const
    {
        return (a.XX-XX)*(a.XX-XX) + (a.YY-YY)*(a.YY-YY) + (a.ZZ-ZZ)*(a.ZZ-ZZ);
    }
    
    /// distance to other point == (a-this).norm()
    real distance(Vector3 const& a) const
    {
        return sqrt((a.XX-XX)*(a.XX-XX) + (a.YY-YY)*(a.YY-YY) + (a.ZZ-ZZ)*(a.ZZ-ZZ));
    }
    
    /// returns  min(x, y, z)
    real minimum() const
    {
        if ( XX > YY )
            return ( YY > ZZ ? ZZ : YY );
        else
            return ( ZZ > XX ? XX : ZZ );
    }
    
    /// returns  max(x, y, z)
    real maximum() const
    {
        if ( XX > YY )
            return ( XX > ZZ ? XX : ZZ );
        else
            return ( ZZ > YY ? ZZ : YY );
    }
    
    /// the infinite norm = max(|x|, |y|, |z|)
    real norm_inf() const
    {
        real x = fabs(XX);
        real y = fabs(YY);
        real z = fabs(ZZ);
        if ( x > y )
            return ( x > z ? x : z );
        else
            return ( z > y ? z : y );
    }
    
    /// normalize to norm=n
    void normalize(const real n = 1.0)
    {
        real s = n / norm();
        XX *= s;
        YY *= s;
        ZZ *= s;
    }
    
    /// returns the colinear vector of norm=n
    const Vector3 normalized(const real n = 1.0) const
    {
        real sn = n / norm();
        return Vector3(XX*sn, YY*sn, ZZ*sn);
    }

    //------------------------------------------------------------------

    /// returns a perpendicular vector, of comparable but unspecified norm
    const Vector3 orthogonal() const
    {
        if ( fabs(XX) < fabs(YY) )
        {
            if ( fabs(XX) < fabs(ZZ) )
                return Vector3(0.0, -ZZ, YY); //XX is the smallest
            else
                return Vector3(-YY, XX, 0.0); //ZZ is the smallest
        }
        else
        {
            if ( fabs(YY) < fabs(ZZ) )
                return Vector3(-ZZ, 0.0, XX); //YY is the smallest
            else
                return Vector3(-YY, XX, 0.0); //ZZ is the smallest
        }
    }

    
    /// returns a perpendicular vector, of norm `n`
    const Vector3 orthogonal(const real n) const
    {
        if ( fabs(XX) < fabs(YY) )
        {
            if ( fabs(XX) < fabs(ZZ) )
            {
                real s = n / sqrt(YY*YY+ZZ*ZZ);
                return Vector3(0.0, -s*ZZ, s*YY); //XX is the smallest
            }
            else
            {
                real s = n / sqrt(XX*XX+YY*YY);
                return Vector3(-s*YY, s*XX, 0.0); //ZZ is the smallest
            }
        }
        else
        {
            if ( fabs(YY) < fabs(ZZ) )
            {
                real s = n / sqrt(XX*XX+ZZ*ZZ);
                return Vector3(-s*ZZ, 0.0, s*XX); //YY is the smallest
            }
            else
            {
                real s = n / sqrt(XX*XX+YY*YY);
                return Vector3(-s*YY, s*XX, 0.0); //ZZ is the smallest
            }
        }
    }
    
    
    /// get the Euler angles
    void getEulerAngles(real angles[]) const
    {
        angles[0] = atan2(YY, XX);
        angles[1] = atan2(sqrt(XX*XX + YY*YY), ZZ);
    }
    
    /// set from Euler angles
    void setFromEulerAngles(const real angles[])
    {
        XX = cos(angles[0])*sin(angles[1]);
        YY = sin(angles[0])*sin(angles[1]);
        ZZ = cos(angles[1]);
    }
    
    //------------------------------------------------------------------
    
    /// addition of two vectors
    friend const Vector3 operator +(Vector3 const& a, Vector3 const& b)
    {
        return Vector3(a.XX+b.XX, a.YY+b.YY, a.ZZ+b.ZZ);
    }
    
    /// subtraction of two vectors
    friend const Vector3 operator -(Vector3 const& a, Vector3 const& b)
    {
        return Vector3(a.XX-b.XX, a.YY-b.YY, a.ZZ-b.ZZ);
    }
    
    /// unary + operator does nothing
    friend const Vector3 operator +(Vector3 const& b)
    {
        return b;
    }
    
    /// opposition of a vector
    friend const Vector3 operator -(Vector3 const& b)
    {
        return Vector3(-b.XX, -b.YY, -b.ZZ);
    }
    
    /// returns the element-by-element product
    const Vector3 e_mul(const real b[]) const
    {
        return Vector3(XX*b[0], YY*b[1], ZZ*b[2]);
    }
    
    /// returns the element-by-element division
    const Vector3 e_div(const real b[]) const
    {
        return Vector3(XX/b[0], YY/b[1], ZZ/b[2]);
    }
    
    
    /// cross product of two vectors
    friend const Vector3 vecProd(Vector3 const& a, Vector3 const& b)
    {
        return Vector3(a.YY * b.ZZ - a.ZZ * b.YY,
                       a.ZZ * b.XX - a.XX * b.ZZ,
                       a.XX * b.YY - a.YY * b.XX);
    }

    
    /// scalar product of two vectors
    friend real operator *(Vector3 const& a, Vector3 const& b)
    {
        return a.XX*b.XX + a.YY*b.YY + a.ZZ*b.ZZ;
    }
    
    /// multiplication by scalar s
    friend const Vector3 operator *(Vector3 const& a, const real s)
    {
        return Vector3(s*a.XX, s*a.YY, s*a.ZZ);
    }
    
    /// mutiplication by scalar s
    friend const Vector3 operator *(const real s, Vector3 const& a)
    {
        return Vector3(s*a.XX, s*a.YY, s*a.ZZ);
    }
    
    /// division by scalar s
    friend const Vector3 operator /(Vector3 const& a, const real s)
    {
        return Vector3(a.XX/s, a.YY/s, a.ZZ/s);
    }
    
    /// addition of another vector b
    void operator +=(Vector3 const& b)
    {
        XX += b.XX;
        YY += b.YY;
        ZZ += b.ZZ;
    }
    
    /// subtraction of another vector b
    void operator -=(Vector3 const& b)
    {
        XX -= b.XX;
        YY -= b.YY;
        ZZ -= b.ZZ;
    }
    
    /// multiplication by a scalar
    void operator *=(const real b)
    {
        XX *= b;
        YY *= b;
        ZZ *= b;
    }
    
    /// division by a scalar
    void operator /=(const real b)
    {
        XX /= b;
        YY /= b;
        ZZ /= b;
    }
    
    //------------------------------------------------------------------
    
    /// equality test
    friend bool operator ==(Vector3 const& a, Vector3 const& b)
    {
        return ( a.XX==b.XX  &&  a.YY==b.YY  &&  a.ZZ==b.ZZ );
    }
    
    /// non-equality test
    friend bool operator !=(Vector3 const& a, Vector3 const& b)
    {
        return ( a.XX!=b.XX  ||  a.YY!=b.YY  ||  a.ZZ!=b.ZZ );
    }
    
    //------------------------------------------------------------------
    
    /// conversion to a string
    std::string str() const
    {
        std::ostringstream oss;
        oss << XX << " " << YY << " " << ZZ;
        return oss.str();
    }
    
    /// print to a file
    void print(FILE * out = stdout) const
    {
        fprintf(out, "  %+9.3f %+9.3f %+9.3f", XX, YY, ZZ);
    }
    
    /// print to a file, surrounded by parenthesis
    void pprint(FILE * out = stdout) const
    {
        fprintf(out, "( %+9.3f %+9.3f %+9.3f )", XX, YY, ZZ);
    }

    /// print, followed by a new line
    void println(FILE * out = stdout) const
    {
        fprintf(out, "  %+9.3f %+9.3f %+9.3f\n", XX, YY, ZZ);
    }

    //------------------------------------------------------------------
    
    /// add a random component in [-s, s] to each coordinate
    void addRand(real s);
    
    /// a vector of norm `n`, orthogonal to *this, chosen randomly and uniformly
    const Vector3 randPerp(real n) const;
    
    
    /// random Vector with coordinates set randomly and independently in [-1,+1]
    static const Vector3 randBox();
    
    /// set all coordinates randomly and independently in [-n,+n]
    static const Vector3 randBox(real n);
    
    
    /// random Vector of norm = 1; sampling is uniform
    static const Vector3 randUnit();
    
    /// return a random vector of norm = n; sampling is uniform
    static const Vector3 randUnit(real n);
    
    
    /// return a random vector of norm <= 1; sampling is uniform
    static const Vector3 randBall();
    
    /// return a random vector of norm <= n; sampling is uniform
    static const Vector3 randBall(real n);
    
    
    /// return a random vector with Normally distributed coordinates ~ N(0,n)
    static const Vector3 randGauss(real n);
    
};


//-------------------------- associated global functions -----------------------

/// stream input operator
std::istream & operator >> ( std::istream& is, Vector3& v);

/// stream output operator
std::ostream & operator << ( std::ostream& os, Vector3 const& v);

/// linear interpolation: returns a + x * b
const Vector3 interpolate(Vector3 const& a, real x, Vector3 const& b);

/// square of the distance between two points, equivalent to (a-b).normSqr()
real distanceSqr(Vector3 const& a, Vector3 const& b);

/// distance between two points, equivalent to (a-b).norm()
real distance(Vector3 const& a, Vector3 const& b);


#endif

