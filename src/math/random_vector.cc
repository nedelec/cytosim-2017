// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#include "vector.h"
#include "random.h"


/**
 Random vectors are generated using the global Random Generator `RNG`
 */
extern Random RNG;


//------------------------------------------------------------------------------
#pragma mark 1D

const Vector1 Vector1::randBox()          { return Vector1(  RNG.sreal(), 0); }
const Vector1 Vector1::randBox(real n)    { return Vector1(n*RNG.sreal(), 0); }
const Vector1 Vector1::randUnit()         { return Vector1(  RNG.sflip(), 0); }
const Vector1 Vector1::randUnit(real n)   { return Vector1(n*RNG.sflip(), 0); }
void  Vector1::addRand(real n)            { XX += n*RNG.sreal(); }

const Vector1 Vector1::randBall()         { return Vector1(   RNG.sreal(), 0); }
const Vector1 Vector1::randBall(real n)   { return Vector1( n*RNG.sreal(), 0); }
const Vector1 Vector1::randGauss(real n)  { return Vector1( n*RNG.gauss(), 0);  }
const Vector1 Vector1::randPerp(real len) const { ABORT_NOW("Vector1::randPerp() meaningless in 1D"); }


//------------------------------------------------------------------------------
#pragma mark - 2D

const Vector2 Vector2::randBox()          { return Vector2(RNG.sreal(), RNG.sreal()); }
const Vector2 Vector2::randBox(real n)    { return Vector2(n*RNG.sreal(), n*RNG.sreal()); }
const Vector2 Vector2::randGauss(real n)  { return Vector2(n*RNG.gauss(), n*RNG.gauss()); }
void  Vector2::addRand(real n)            { XX += n*RNG.sreal(); YY += n*RNG.sreal(); }

const Vector2 Vector2::randUnit()
{
    real d, x, y;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        d = x*x + y*y;
    } while ( d > 1.0  ||  d == 0 );
    d = sqrt( d );
    return Vector2( x/d, y/d );
}

const Vector2 Vector2::randUnit(real n)
{
    real d, x, y;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        d = x*x + y*y;
    } while ( d > 1.0  ||  d == 0 );
    d = n / sqrt( d );
    return Vector2( x*d, y*d );
}

const Vector2 Vector2::randBall()
{
    real x, y;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
    } while ( x*x + y*y > 1.0 );
    return Vector2( x, y );
}

const Vector2 Vector2::randBall(real n)
{
    real x, y;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
    } while ( x*x + y*y > 1.0 );
    return Vector2( x*n, y*n );
}

const Vector2 Vector2::randPerp(real len) const
{
    real s = RNG.sflip() * len / sqrt( XX * XX + YY * YY );
    return Vector2(-s*YY, s*XX, 0);
}

//------------------------------------------------------------------------------
#pragma mark - 3D

const Vector3 Vector3::randBox()          { return Vector3(RNG.sreal(), RNG.sreal(), RNG.sreal()); }
const Vector3 Vector3::randBox(real n)    { return Vector3(n*RNG.sreal(), n*RNG.sreal(), n*RNG.sreal()); }
const Vector3 Vector3::randGauss(real n)  { return Vector3(n*RNG.gauss(), n*RNG.gauss(), n*RNG.gauss()); }
void  Vector3::addRand(real n)            { XX += n*RNG.sreal(); YY += n*RNG.sreal(); ZZ += n*RNG.sreal(); }


#if ( 1 )

/// hypercube rejection method
const Vector3 Vector3::randUnit(real n)
{
    real x, y, z, d;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        z = RNG.sreal();
        d = x*x + y*y + z*z;
    } while ( d > 1.0  ||  d == 0 );
    d = n / sqrt( d );
    return Vector3( x*d, y*d, z*d );
}

/// hypercube rejection method
const Vector3 Vector3::randUnit()
{
    real x, y, z, d;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        z = RNG.sreal();
        d = x*x + y*y + z*z;
    } while ( d > 1.0  ||  d == 0 );
    d = 1.0 / sqrt( d );
    return Vector3( x*d, y*d, z*d );
}

#else

/**
 From Allen & Tildesley "Computer Simulation of Liquids" Clarendon Pres, Oxford 1987
 This uses only 2 random-numbers!
*/
const Vector3 Vector3::randUnit()
{
    real x, y, d;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        d = x*x + y*y;
    } while ( d >= 1.0 );
    real h  = 2 * sqrt(1-d);
    return Vector3( x*h, y*h, 1-2*d );
}

const Vector3 Vector3::randUnit(real n)
{
    return n * randUnit();
}

#endif


const Vector3 Vector3::randBall()
{
    real x, y, z;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        z = RNG.sreal();
    } while ( x*x + y*y + z*z > 1.0 );
    return Vector3( x, y, z );
}


const Vector3 Vector3::randBall(real n)
{
    real x, y, z;
    do {
        x = RNG.sreal();
        y = RNG.sreal();
        z = RNG.sreal();
    } while ( x*x + y*y + z*z > 1.0 );
    return Vector3( x*n, y*n, z*n );
}


const Vector3 Vector3::randPerp(real len) const
{
    Vector3 b = orthogonal(1);
    Vector3 c = vecProd(*this, b).normalized();
    Vector2 d = Vector2::randUnit();
    return b * ( len * d.XX ) + c * ( len * d.YY );
}


