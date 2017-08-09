// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#ifndef VECTOR1_H
#define VECTOR1_H


#include "real.h"
#include "assert_macro.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cmath>

/// Vector1 is a vector with 1 `real` component.
class Vector1
{
    
public:
    
    /// dimensionality is 1
    static unsigned dimensionality() { return 1; }
    
    /// coordinates are public
    real XX;
    
    
    /// by default, coordinates are not initialized
    Vector1() {}
    
    /// construct from values
    Vector1(const real x, real) : XX(x) {}
    
    /// construct from values
    Vector1(const real x, real, real) : XX(x) {}
    
    /// construct from address
    Vector1(const real v[]) : XX(v[0]) {}
    
    /// construct from array of size d
    Vector1(const real v[], int d)
    {
        if ( d > 0 ) XX = v[0]; else XX = 0;
    }
    
    /// create new Vector with coordinates from the given array
    static Vector1 make(const real b[]) { return Vector1(b[0], 0); }
    
    /// destructor (is not-virtual: do not derive from this class)
    ~Vector1() {}
    
    
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
        assert_true(ii==0);
        return XX;
    }
    
    /// modifiable access to individual coordinates
    real& operator[](unsigned ii)
    {
        assert_true(ii==0);
        return XX;
    }
#endif
    
    /// copy coordinates from array of size d
    void get(const real v[], const int& d)
    {
        if ( d > 0 ) XX = v[0]; else XX = 0;
    }
    
    /// replace coordinates by the ones provided
    void get(const float b[])
    {
        XX = b[0];
    }
    
    /// replace coordinates by the ones provided
    void get(const double b[])
    {
        XX = b[0];
    }
    
    /// copy coordinates to given array
    void put(float b[]) const
    {
        b[0] = XX;
    }
    
    /// copy coordinates to given array
    void put(double b[]) const
    {
        b[0] = XX;
    }
    
    /// add content to given address
    void add_to(real b[]) const
    {
        b[0] += XX;
    }
    
    /// add content scaled by `alpha` to given address
    void add_to(real alpha, real b[]) const
    {
        b[0] += alpha * XX;
    }
    
    /// add content `n` times to array `b` of size `ldd*n`
    void add_to(real b[], int n, int ldd) const
    {
        for ( int i = 0; i < n; ++i )
            b[ldd*i] += XX;
    }
    
    /// subtract content to given array
    void sub_to(real b[]) const
    {
        b[0] -= XX;
    }
    
    /// add content scaled by `alpha` to given address
    void sub_to(real alpha, real b[]) const
    {
        b[0] -= alpha * XX;
    }
    
    /// set coordinates to zero
    void zero()
    {
        XX = 0;
    }
    
    /// change coordinates
    void set(const real x)
    {
        XX = x;
    }
    
    /// change coordinates (last 2 arguments are discarded)
    void set(const real x, const real, const real)
    {
        XX = x;
    }
    
    /// change signs of all coordinates
    void oppose()
    {
        XX = -XX;
    }
    
    //------------------------------------------------------------------
    
    /// the square of the standard norm
    real normSqr() const
    {
        return XX*XX;
    }
    
    
    /// the standard norm = sqrt(x^2+y^2+z^2)
    real norm() const
    {
        return sqrt(XX*XX);
    }
    
    /// square of the distance to other point == (a-this).normSqr()
    real distanceSqr(Vector1 const& a) const
    {
        return (a.XX-XX)*(a.XX-XX);
    }
    
    /// distance to other point == (a-this).norm()
    real distance(Vector1 const& a) const
    {
        return fabs(a.XX-XX);
    }
    
    /// returns X
    real minimum() const
    {
        return XX;
    }
    
    /// returns X
    real maximum() const
    {
        return XX;
    }
    
    /// the infinite norm = |x|
    real norm_inf() const
    {
        return fabs(XX);
    }
    
    /// true if no component is NaN
    bool valid() const
    {
        return ( XX == XX );
    }
    
    /// true if all components are zero
    bool null() const
    {
        return XX == 0;
    }
    
    /// normalize to norm=n
    void normalize(const real n = 1.0)
    {
        if ( XX > 0 )
            XX = n;
        else if ( XX < 0 )
            XX = -n;
    }
    
    /// returns the colinear vector of norm=n
    const Vector1 normalized(const real n = 1.0) const
    {
        if ( XX > 0 )
            return Vector1(n, 0);
        else if ( XX < 0 )
            return Vector1(-n, 0);
        else
            return Vector1(0, 0);
    }
    
    /// returns a perpendicular vector, of comparable but unspecified norm
    const Vector1 orthogonal() const
    {
        ABORT_NOW("Vector::orthogonal() is not meaningful in 1D");
        return Vector1(0, 0);
    }
    
    /// returns a perpendicular vector, of norm `n`
    const Vector1 orthogonal(const real) const
    {
        ABORT_NOW("Vector::orthogonal() is not meaningful in 1D");
        return Vector1(0, 0);
    }
    
    /// convert from cartesian to spherical coordinates ( r, theta, phi )
    const Vector1 spherical() const { return Vector1(XX, 0); }
    
    /// convert from spherical to cartesian coordinates ( x, y, z )
    const Vector1 cartesian() const { return Vector1(XX, 0); }
    
    //------------------------------------------------------------------
    
    /// addition of two vectors
    friend const Vector1 operator +(Vector1 const& a, Vector1 const& b)
    {
        return Vector1(a.XX+b.XX, 0);
    }
    
    /// subtraction of two vectors
    friend const Vector1 operator -(Vector1 const& a, Vector1 const& b)
    {
        return Vector1(a.XX-b.XX, 0);
    }
    
    /// unary + operator does nothing
    friend const Vector1 operator +(Vector1 const& b)
    {
        return b;
    }
    
    /// opposition of a vector
    friend const Vector1 operator -(Vector1 const& b)
    {
        return Vector1(-b.XX, 0);
    }
    
    /// returns the element-by-element product
    const Vector1 e_mul(const real b[]) const
    {
        return Vector1(XX*b[0], 0);
    }
    
    /// returns the element-by-element division
    const Vector1 e_div(const real b[]) const
    {
        return Vector1(XX/b[0], 0);
    }
    
    /// returns a vector with each element squared
    const Vector1 e_squared() const
    {
        return Vector1(XX*XX, 0);
    }
    
    /// returns sum of all coordinates
    real e_sum() const
    {
        return XX;
    }
    
    
    /**
     In dimension 1, the vector product is not really useful,
     but it is defined for completeness with the other class Vector2, Vector3.
     */
    
    /// the cross product of two vectors is a Z-Vector
    friend real vecProd(Vector1 const& a, Vector1 const& b)
    {
        return 0;
    }
    
    /// cross product of a vector with a Z-Vector
    friend const Vector1 vecProd(Vector1 const& a, const real b)
    {
        return Vector1(0, 0);
    }
    
    /// cross product of a Z-vector with a Vector
    friend const Vector1 vecProd(const real a, Vector1 const& b)
    {
        return Vector1(0, 0);
    }
    
    
    /// scalar product of two vectors
    friend real operator *(Vector1 const& a, Vector1 const& b)
    {
        return a.XX*b.XX;
    }
    
    /// multiplication by scalar s
    friend const Vector1 operator *(Vector1 const& a, const real s)
    {
        return Vector1(s*a.XX, 0);
    }
    
    /// mutiplication by scalar s
    friend const Vector1 operator *(const real s, Vector1 const& a)
    {
        return Vector1(s*a.XX, 0);
    }
    
    /// division by scalar s
    friend const Vector1 operator /(Vector1 const& a, const real s)
    {
        return Vector1(a.XX/s, 0);
    }
    
    /// addition of another vector b
    void operator +=(Vector1 const& b)
    {
        XX += b.XX;
    }
    
    /// subtraction of another vector b
    void operator -=(Vector1 const& b)
    {
        XX -= b.XX;
    }
    
    /// multiplication by a scalar
    void operator *=(const real b)
    {
        XX *= b;
    }
    
    /// division by a scalar
    void operator /=(const real b)
    {
        XX /= b;
    }
    
    //------------------------------------------------------------------
    
    /// equality test
    friend bool operator ==(Vector1 const& a, Vector1 const& b)
    {
        return ( a.XX==b.XX );
    }
    
    /// non-equality test
    friend bool operator !=(Vector1 const& a, Vector1 const& b)
    {
        return ( a.XX!=b.XX );
    }
    
    //------------------------------------------------------------------
    
    /// conversion to a string
    std::string repr() const
    {
        std::ostringstream oss;
        oss << XX;
        return oss.str();
    }
    
    /// conversion to a string with given precision
    std::string repr(int w, int p) const
    {
        std::ostringstream oss;
        oss.precision(p);
        oss << std::setw(w) << XX;
        return oss.str();
    }
    
    /// print to a file
    void print(FILE * out = stdout) const
    {
        fprintf(out, "  %+9.3f", XX);
    }
    
    /// print to a file, surrounded by parenthesis
    void pprint(FILE * out = stdout) const
    {
        fprintf(out, "( %+9.3f )", XX);
    }
    
    /// print, followed by a new line
    void println(FILE * out = stdout) const
    {
        fprintf(out, "  %+9.3f\n", XX);
    }
    
    //------------------------------------------------------------------
    
    /// add a random component in [-s, s] to each coordinate
    void addRand(real s);
    
    /// a vector of norm `n`, orthogonal to *this, chosen randomly and uniformly
    const Vector1 randPerp(real n) const;
    
    
    /// random Vector with coordinates set randomly and independently in [-1,+1]
    static const Vector1 randBox();
    
    /// set all coordinates randomly and independently in [-n,+n]
    static const Vector1 randBox(real n);
    
    
    /// random Vector of norm = 1; sampling is uniform
    static const Vector1 randUnit();
    
    /// return a random vector of norm = n; sampling is uniform
    static const Vector1 randUnit(real n);
    
    
    /// return a random vector of norm <= 1; sampling is uniform
    static const Vector1 randBall();
    
    /// return a random vector of norm <= n; sampling is uniform
    static const Vector1 randBall(real n);
    
    
    /// return a random vector with Normally distributed coordinates ~ N(0,n)
    static const Vector1 randGauss(real n);
    
};


//-------------------------- associated global functions -----------------------

/// stream input operator
std::istream & operator >> (std::istream&, Vector1&);

/// stream output operator
std::ostream & operator << (std::ostream&, Vector1 const&);

/// linear interpolation: returns a + x * b
const Vector1 interpolate(Vector1 const& a, real x, Vector1 const& b);

/// square of the distance between two points, equivalent to (a-b).normSqr()
real distanceSqr(Vector1 const& a, Vector1 const& b);

/// distance between two points, equivalent to (a-b).norm()
real distance(Vector1 const& a, Vector1 const& b);


#endif

