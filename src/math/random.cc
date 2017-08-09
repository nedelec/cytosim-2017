// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "random.h"
#include "assert_macro.h"

#include <climits>
#include <sys/time.h>
#include <time.h>

///RNG = Random Number Generator
Random RNG;

//------------------------------------------------------------------------------
Random::Random()
{
    seed(1);

    // initialize pointers:
    sfmt_first = &sfmt.state[0].u[0];
    sfmt_last  = sfmt_first + SFMT_N32;
    sfmt_ptr   = sfmt_last;
    
    if ( sizeof(uint32_t) != 4 )
        ABORT_NOW("sizeof( uint32_t ) != 4");
    
    bufferValue = 0;
    bufferValid = false;
}

Random::~Random()
{
}

/**
 Get a uint32_t from t and c
 Better than uint32_t(x) in case x is floating point in [0,1]
 Based on code by Lawrence Kirby (fred@genesis.demon.co.uk)
 */
uint32_t hash(time_t t, clock_t c)
{
    static uint32_t differ = 0;  // guarantee time-based seeds will change
    
    uint32_t h1 = 0;
    unsigned char* p = (unsigned char*) &t;
    for ( size_t i = 0; i < sizeof(t); ++i )
    {
        h1 *= UCHAR_MAX + 2U;
        h1 += p[i];
    }
    uint32_t h2 = 0;
    p = (unsigned char*) &c;
    for ( size_t j = 0; j < sizeof(c); ++j )
    {
        h2 *= UCHAR_MAX + 2U;
        h2 += p[j];
    }
    return ( h1 + differ++ ) ^ h2;
}


uint32_t Random::seedTimer()
{
    struct timeval now;
    gettimeofday(&now, 0);
    uint32_t s = hash( now.tv_sec, now.tv_usec );
    seed(s);
    return s;
}

//------------------------------------------------------------------------------
#pragma mark -

int Random::sign_exc(const real a)
{
    if ( a <= 0 )
    {
        if ( a < 0 )
            return -1;
        return RNG.sflip();
    }
    return 1;
}

//------------------------------------------------------------------------------
#pragma mark -
/**
 Signed real number, following a normal law N(0,1)
 using the polar rejection method (see Numerical Recipe)
 */
real Random::gauss()
{
    if ( bufferValid )
    {
        bufferValid = false;
        return bufferValue;
    }
    else
    {
        real x, y, w;
        do {
            x = sreal();
            y = sreal();
            w = x * x + y * y;
        } while ( w >= 1.0  ||  w == 0 );
        w = sqrt( -2 * log(w) / w );
        bufferValue = w * x;
        bufferValid = true;
        return w * y;
    }
}

/**
 Signed real number, following a normal law N(0,1)
 using the polar rejection method (see Numerical Recipe)
 */
void Random::gauss_pair(real & a, real & b)
{
    real x, y, w;
    do {
        x = sreal();
        y = sreal();
        w = x * x + y * y;
    } while ( w >= 1.0  ||  w == 0 );
    w = sqrt( -2 * log(w) / w );
    a = w * x;
    b = w * y;
}

/**
 Fill \a n values in array \a vec[] with Gaussian ~ N(0,1).
 */
void Random::gauss_array(unsigned int n, real vec[])
{
    unsigned int u = n % 8;
    unsigned int w = u % 2;
    
    if ( w )
        vec[0] = gauss();
    
    for ( ; w < u; w += 2 )
        gauss_pair(vec[w], vec[w+1]);
    
    for ( ; u < n; u += 8 )
    {
        gauss_pair(vec[u  ], vec[u+1]);
        gauss_pair(vec[u+2], vec[u+3]);
        gauss_pair(vec[u+4], vec[u+5]);
        gauss_pair(vec[u+6], vec[u+7]);
    }
}


/**
 this version uses cos() and sin() and is slower than gauss().
 const real PI = 3.14159265358979323846264338327950288;
 */
real Random::gauss_slow()
{
    if ( bufferValid )
    {
        bufferValid = false;
        return bufferValue;
    }
    else
    {
        real angle = real( RAN32() ) * 1.46291807926715968105133780430979e-9;
        //the constant is 2*pi/2^32
        real norm  = sqrt( -2 * log( preal_exc() ));
        bufferValue = norm * cos(angle);
        bufferValid = true;
        return norm * sin(angle);
    }
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 integer in [0,n] for n < 2^32
 */
uint32_t Random::pint_inc2(const uint32_t n)
{
    // Find which bits are used in n
    uint32_t used = n | ( n >> 1 );
    used |= (used >> 2);
    used |= (used >> 4);
    used |= (used >> 8);
    used |= (used >> 16);
    
    // Draw numbers until one is found in [0,n]
    uint32_t i;
    do
        i = RAN32() & used;  // toss unused bits to shorten search
    while ( i > n );
    return i;
}


/**
 returns an integer in [0 n], with the ratios given in the array of ints
 */
uint32_t Random::pint_ratio(const uint32_t n, const int ratio[])
{
    int sum = 0;
    uint32_t ii;
    for ( ii = 0; ii < n; ++ii )
    {
        assert_true( ratio[ii] >= 0 );
        sum += ratio[ii];
    }
    // sum==0 may denotes a careless use of the function, with wrong arguments.
    // it might be safer to throw an exception
    if ( sum == 0 ) return 0;
    sum = (int) floor( preal() * sum );
    ii = 0;
    while ( sum >= ratio[ii] )
        sum -= ratio[ii++];
    return ii;
}

//------------------------------------------------------------------------------
#pragma mark -
/**
 Return Poisson distributed integer, with expectation=E  variance=E
 http://en.wikipedia.org/wiki/Poisson_distribution
 
 This routine is very slow for large values of E.
 If E > 512, this returs a Gaussian distribution of parameter (E, E),
 which is a good approximation of the Poisson distribution.
 
 This method fails for E > 700, in double precision
 */
uint32_t Random::poisson(const real E)
{
    if ( E > 256 )
        return static_cast<uint32_t>( gauss() * sqrt(E) + E );
    
    assert_true( E >= 0 );
    
    real p = exp(-E);
    real s = p;
    uint32_t k = 0;
    real u = preal();
    while ( u > s )
    {
        ++k;
        p *= E / k;
        s += p;
    }
    return k;
}


uint32_t Random::geometric(const real P)
{
    assert_true( P >= 0 );
    uint32_t pi = (uint32_t)( P * 0x1p32 );
    
    uint32_t s = 0;
    while ( RAN32() > pi )
        ++s;
    return s;
}


uint32_t Random::binomial(const int N, const real P)
{
    assert_true( P >= 0 );
    uint32_t pi = (uint32_t)( P * 0x1p32 );
    
    uint32_t s = 0;
    for ( int x = 0; x < N; ++x )
        if ( RAN32() < pi )
            ++s;
    return s;
}

