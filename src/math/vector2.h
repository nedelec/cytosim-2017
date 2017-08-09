// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef VECTOR2_H
#define VECTOR2_H


#include "real.h"
#include "assert_macro.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cmath>

/// Vector2 is a vector with 2 `real` components.
/**
 Note: We assume that the coordinates XX and YY are adjacent in memory,
 allowing easy conversion operators to and from C-array.
 Although this is not guaranteed by the C-standard, this is usually the case.
 */
class Vector2
{
    
public:
    
    /// dimensionality is 2
    static unsigned dimensionality() { return 2; }
    
    /// coordinates are public
    real XX, YY;
    
    
    /// by default, coordinates are not initialized
    Vector2() {}
    
    /// construct from values
    Vector2(const real x, const real y) : XX(x), YY(y) {}
    
    /// construct from values
    Vector2(const real x, const real y, real) : XX(x), YY(y) {}
    
    /// construct from address
    Vector2(const real v[]) : XX(v[0]), YY(v[1]) {}
    
    /// construct from array of size d
    Vector2(const real v[], int d)
    {
        if ( d > 0 ) XX = v[0]; else XX = 0;
        if ( d > 1 ) YY = v[1]; else YY = 0;
    }
    
    /// create new Vector with coordinates from the given array
    static Vector2 make(const real b[]) { return Vector2(b[0], b[1]); }
    
    /// destructor (is not-virtual: do not derive from this class)
    ~Vector2() {}
    
    
    /// address of coordinate array
    real * data()                { return &XX; }
    
    /// constant address of coordinate array
    real const* data()     const { return &XX; }
    
#if ( 1 )
    /// implicit conversion to a modifiable real pointer
    operator real*()             { return &XX; }
    
    /// implicit conversion to a constant real pointer
    operator const real*() const { return &XX; }
#else
    /// value of a coordinate
    real operator[](unsigned ii) const
    {
        assert_true(ii<2);
        if ( ii == 1 )
            return YY;
        else
            return XX;
    }
    
    /// modifiable access to individual coordinates
    real& operator[](unsigned ii)
    {
        assert_true(ii<2);
        if ( ii == 1 )
            return YY;
        else
            return XX;
    }
#endif
    
    /// copy coordinates from array of size d
    void get(const real v[], const int& d)
    {
        if ( d > 0 ) XX = v[0]; else XX = 0;
        if ( d > 1 ) YY = v[1]; else YY = 0;
    }
    
    /// replace coordinates by the ones provided
    void get(const float b[])
    {
        XX = b[0];
        YY = b[1];
    }
    
    /// replace coordinates by the ones provided
    void get(const double b[])
    {
        XX = b[0];
        YY = b[1];
    }
    
    /// copy coordinates to given array
    void put(float b[]) const
    {
        b[0] = XX;
        b[1] = YY;
    }
    
    /// copy coordinates to given array
    void put(double b[]) const
    {
        b[0] = XX;
        b[1] = YY;
    }
    
    /// add content to given address
    void add_to(real b[]) const
    {
        b[0] += XX;
        b[1] += YY;
    }
    
    /// add content scaled by `alpha` to given address
    void add_to(real alpha, real b[]) const
    {
        b[0] += alpha * XX;
        b[1] += alpha * YY;
    }
    
    /// add content `n` times to array `b` of size `ldd*n`
    void add_to(real b[], int n, int ldd) const
    {
        for ( int i = 0; i < n; ++i )
        {
            b[ldd*i  ] += XX;
            b[ldd*i+1] += YY;
        }
    }
    
    /// subtract content scaled by `alpha` to given address
    void sub_to(real alpha, real b[]) const
    {
        b[0] -= alpha * XX;
        b[1] -= alpha * YY;
    }
    
    /// subtract to given array
    void sub_to(real b[]) const
    {
        b[0] -= XX;
        b[1] -= YY;
    }
    
    /// set coordinates to zero
    void zero()
    {
        XX = 0;
        YY = 0;
    }
    
    /// change coordinates
    void set(const real x, const real y)
    {
        XX = x;
        YY = y;
    }
    
    /// change coordinates (last argument is discarded)
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
    
    /// returns  min(x, y)
    real minimum() const
    {
        if ( XX > YY )
            return YY;
        else
            return XX;
    }
    
    /// returns  max(x, y)
    real maximum() const
    {
        if ( XX > YY )
            return XX;
        else
            return YY;
    }
    
    /// the infinite norm = max(|x|, |y|)
    real norm_inf() const
    {
        real x = fabs(XX);
        real y = fabs(YY);
        if ( x > y )
            return x;
        else
            return y;
    }
    
    /// true if no component is NaN
    bool valid() const
    {
        return ( XX == XX ) && ( YY == YY );
    }
    
    /// true if all components are zero
    bool null() const
    {
        return ( XX == 0 ) && ( YY == 0 );
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
    
    /// returns a perpendicular vector, of same norm
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
    
    /// convert from cartesian to spherical coordinates ( r, theta, phi )
    const Vector2 spherical() const
    {
        return Vector2(sqrt(XX*XX+YY*YY),
                       atan2(YY, XX));
    }
    
    /// convert from spherical to cartesian coordinates ( x, y, z )
    const Vector2 cartesian() const
    {
        return Vector2(XX*cos(YY),
                       XX*sin(YY));
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
    
    /// returns a vector with each element squared
    const Vector2 e_squared() const
    {
        return Vector2(XX*XX, YY*YY);
    }
    
    /// returns sum of all coordinates
    real e_sum() const
    {
        return XX + YY;
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
    std::string repr() const
    {
        std::ostringstream oss;
        oss << XX << " " << YY;
        return oss.str();
    }
    
    /// conversion to a string with given precision
    std::string repr(int w, int p) const
    {
        std::ostringstream oss;
        oss.precision(p);
        oss << std::setw(w) << XX << " ";
        oss << std::setw(w) << YY;
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
std::istream & operator >> (std::istream&, Vector2&);

/// stream output operator
std::ostream & operator << (std::ostream&, Vector2 const&);

/// linear interpolation: returns a + x * b
const Vector2 interpolate(Vector2 const& a, real x, Vector2 const& b);

/// square of the distance between two points, equivalent to (a-b).normSqr()
real distanceSqr(Vector2 const& a, Vector2 const& b);

/// distance between two points, equivalent to (a-b).norm()
real distance(Vector2 const& a, Vector2 const& b);


#endif

