// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef VECTOR2_H
#define VECTOR2_H


#include "real.h"
#include "assert_macro.h"

#include <iostream>
#include <sstream>
#include <cstdio>
#include <cmath>

/// Vector2 is a vector with 2 `real` components.
/**
 Note: We assume that the coordinates XX and YY are adjacent in memory, which allows
 us to define an automatic conversion to a C-array.
 This is not guaranteed by the C-standard, but it usually works though...
 */
class Vector2
{
    
public:
    
    /// dimensionality
    static unsigned dimensionality() { return 2; }
    
    /// coordinates are public
    real XX, YY;
    
    
    /// by default, coordinates are not initialized
    Vector2() {}
    
    /// construct from values
    explicit Vector2(const real x, const real y) : XX(x), YY(y) {}
    
    /// construct from values
    Vector2(const real x, const real y, const real) : XX(x), YY(y) {}

    /// construct from address
    Vector2(const real v[]) : XX(v[0]), YY(v[1]) {}
    
    /// construct from array of size d
    Vector2(const real v[], const int& d)
    {
        if ( d > 0 ) XX = v[0]; else XX = 0;
        if ( d > 1 ) YY = v[1]; else YY = 0;
    }

    /// create new Vector with coordinates from the given array
    static Vector2 make(const real b[]) { return Vector2(b[0], b[1]); }

    /// destructor (is not-virtual: do not derive from this class)
    ~Vector2() {}
    
    
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
    }
    
    /// copy coordinates from array of size d
    void get(const real v[], const int& d)
    {
        if ( d > 0 ) XX = v[0]; else XX = 0;
        if ( d > 1 ) YY = v[1]; else YY = 0;
    }

    /// copy coordinates to given array
    void put(real b[]) const
    {
        b[0] = XX;
        b[1] = YY;
    }
    
    /// add to given array
    void addTo(real b[]) const
    {
        b[0] += XX;
        b[1] += YY;
    }
    
    /// change coordinates by adding given array
    void addFrom(real b[])
    {
        XX += b[0];
        YY += b[1];
    }
    
    /// subtract to given array
    void subTo(real b[]) const
    {
        b[0] -= XX;
        b[1] -= YY;
    }
    
    /// change coordinates by subtracting given array
    void subFrom(real b[])
    {
        XX -= b[0];
        YY -= b[1];
    }
    
    /// change coordinates
    void set(const real x, const real y)
    {
        XX = x;
        YY = y;
    }
    
    /// change coordinates
    void set(const real x, const real y, const real)
    {
        XX = x;
        YY = y;
    }

    /// change signs of all coordinates
    void oppose()
    {
        XX = -XX;
        YY = -YY;
    }
    
    //------------------------------------------------------------------
    
    /// the square of the standard norm
    real normSqr() const
    {
        return XX*XX + YY*YY;
    }

    
    /// the standard norm = sqrt(x^2+y^2)
    real norm() const
    {
        return sqrt(XX*XX + YY*YY);
    }
    
    /// the 2D norm = sqrt(x^2+y^2)
    real normXY() const
    {
        return sqrt(XX*XX + YY*YY);
    }

    /// square of the distance to other point == (a-this).normSqr()
    real distanceSqr(Vector2 const& a) const
    {
        return (a.XX-XX)*(a.XX-XX) + (a.YY-YY)*(a.YY-YY);
    }
    
    /// distance to other point == (a-this).norm()
    real distance(Vector2 const& a) const
    {
        return sqrt((a.XX-XX)*(a.XX-XX) + (a.YY-YY)*(a.YY-YY));
    }
    
    /// returns  min(x, y, z)
    real minimum() const
    {
        if ( XX > YY )
            return YY;
        else
            return XX;
    }
    
    /// returns  max(x, y, z)
    real maximum() const
    {
        if ( XX > YY )
            return XX;
        else
            return YY;
    }
    
    /// the infinite norm = max(|x|, |y|, |z|)
    real norm_inf() const
    {
        real x = fabs(XX);
        real y = fabs(YY);
        if ( x > y )
            return x;
        else
            return y;
    }
    
    /// normalize to norm=n
    void normalize(const real n = 1.0)
    {
        real s = n / norm();
        XX *= s;
        YY *= s;
    }
    
    /// returns the colinear vector of norm=n
    const Vector2 normalized(const real n = 1.0) const
    {
        real sn = n / norm();
        return Vector2(XX*sn, YY*sn);
    }

    //------------------------------------------------------------------

    /// returns a perpendicular vector, of comparable but unspecified norm
    const Vector2 orthogonal() const
    {
        return Vector2(-YY, XX);
    }

    
    /// returns a perpendicular vector, of norm `n`
    const Vector2 orthogonal(const real n) const
    {
        real s = n / sqrt( XX*XX + YY*YY );
        return Vector2(-s*YY, s*XX);
    }

    
    /// get the Euler angles
    void getEulerAngles(real& angle) const
    {
        angle = atan2(YY, XX);
    }
    
    /// set from Euler angles
    void setFromEulerAngles(const real angle)
    {
        XX = cos(angle);
        YY = sin(angle);
    }
    
    //------------------------------------------------------------------
    
    /// addition of two vectors
    friend const Vector2 operator +(Vector2 const& a, Vector2 const& b)
    {
        return Vector2(a.XX+b.XX, a.YY+b.YY);
    }
    
    /// subtraction of two vectors
    friend const Vector2 operator -(Vector2 const& a, Vector2 const& b)
    {
        return Vector2(a.XX-b.XX, a.YY-b.YY);
    }
    
    /// unary + operator does nothing
    friend const Vector2 operator +(Vector2 const& b)
    {
        return b;
    }
    
    /// opposition of a vector
    friend const Vector2 operator -(Vector2 const& b)
    {
        return Vector2(-b.XX, -b.YY);
    }
    
    /// returns the element-by-element product
    const Vector2 e_mul(const real b[]) const
    {
        return Vector2(XX*b[0], YY*b[1]);
    }
    
    /// returns the element-by-element division
    const Vector2 e_div(const real b[]) const
    {
        return Vector2(XX/b[0], YY/b[1]);
    }

    
    /**
     In dimension 2, we define a cross-product operator which returns a real,
     which in this case represents a Vector aligned with the Z axis.
     We also define the cross-product with a scalar, also corresponding to a 
     Vector aligned with Z. This is a fair contraction of the 3D vector product.
     */
    
    /// the cross product of two vectors is a Z-Vector
    friend real vecProd(Vector2 const& a, Vector2 const& b)
    {
        return a.XX * b.YY - a.YY * b.XX;
    }

    /// cross product of a vector with a Z-Vector
    friend const Vector2 vecProd(Vector2 const& a, const real b)
    {
        return Vector2(a.YY*b, -a.XX*b);
    }

    /// cross product of a Z-vector with a Vector
    friend const Vector2 vecProd(const real a, Vector2 const& b)
    {
        return Vector2(-a*b.YY, a*b.XX);
    }

    
    /// scalar product of two vectors
    friend real operator *(Vector2 const& a, Vector2 const& b)
    {
        return a.XX*b.XX + a.YY*b.YY;
    }
    
    /// multiplication by scalar s
    friend const Vector2 operator *(Vector2 const& a, const real s)
    {
        return Vector2(s*a.XX, s*a.YY);
    }
    
    /// mutiplication by scalar s
    friend const Vector2 operator *(const real s, Vector2 const& a)
    {
        return Vector2(s*a.XX, s*a.YY);
    }
    
    /// division by scalar s
    friend const Vector2 operator /(Vector2 const& a, const real s)
    {
        return Vector2(a.XX/s, a.YY/s);
    }
    
    /// addition of another vector b
    void operator +=(Vector2 const& b)
    {
        XX += b.XX;
        YY += b.YY;
    }
    
    /// subtraction of another vector b
    void operator -=(Vector2 const& b)
    {
        XX -= b.XX;
        YY -= b.YY;
    }
    
    /// multiplication by a scalar
    void operator *=(const real b)
    {
        XX *= b;
        YY *= b;
    }
    
    /// division by a scalar
    void operator /=(const real b)
    {
        XX /= b;
        YY /= b;
    }
    
    //------------------------------------------------------------------
    
    /// equality test
    friend bool operator ==(Vector2 const& a, Vector2 const& b)
    {
        return ( a.XX==b.XX  &&  a.YY==b.YY );
    }
    
    /// non-equality test
    friend bool operator !=(Vector2 const& a, Vector2 const& b)
    {
        return ( a.XX!=b.XX  ||  a.YY!=b.YY );
    }
    
    //------------------------------------------------------------------
    
    /// conversion to a string
    std::string str() const
    {
        std::ostringstream oss;
        oss << XX << " " << YY;
        return oss.str();
    }
    
    /// print to a file
    void print(FILE * out = stdout) const
    {
        fprintf(out, "  %+9.3f %+9.3f", XX, YY);
    }
    
    /// print to a file, surrounded by parenthesis
    void pprint(FILE * out = stdout) const
    {
        fprintf(out, "( %+9.3f %+9.3f )", XX, YY);
    }

    /// print, followed by a new line
    void println(FILE * out = stdout) const
    {
        fprintf(out, "  %+9.3f %+9.3f\n", XX, YY);
    }

    //------------------------------------------------------------------
    
    /// add a random component in [-s, s] to each coordinate
    void addRand(real s);
    
    /// a vector of norm `n`, orthogonal to *this, chosen randomly and uniformly
    const Vector2 randPerp(real n) const;
    
    
    /// random Vector with coordinates set randomly and independently in [-1,+1]
    static const Vector2 randBox();
    
    /// set all coordinates randomly and independently in [-n,+n]
    static const Vector2 randBox(real n);
    
    
    /// random Vector of norm = 1; sampling is uniform
    static const Vector2 randUnit();
    
    /// return a random vector of norm = n; sampling is uniform
    static const Vector2 randUnit(real n);
    
    
    /// return a random vector of norm <= 1; sampling is uniform
    static const Vector2 randBall();
    
    /// return a random vector of norm <= n; sampling is uniform
    static const Vector2 randBall(real n);
    
    
    /// return a random vector with Normally distributed coordinates ~ N(0,n)
    static const Vector2 randGauss(real n);
    
};


//-------------------------- associated global functions -----------------------

/// stream input operator
std::istream & operator >> ( std::istream& is, Vector2& v);

/// stream output operator
std::ostream & operator << ( std::ostream& os, Vector2 const& v);

/// linear interpolation: returns a + x * b
const Vector2 interpolate(Vector2 const& a, real x, Vector2 const& b);

/// square of the distance between two points, equivalent to (a-b).normSqr()
real distanceSqr(Vector2 const& a, Vector2 const& b);

/// distance between two points, equivalent to (a-b).norm()
real distance(Vector2 const& a, Vector2 const& b);


#endif

