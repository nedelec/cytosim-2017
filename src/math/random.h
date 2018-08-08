// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>
#include <cmath>
#include "real.h"

#define SFMT_MEXP 19937
#include "SFMT.h"


/// Random Number Generator
/**
 The generation of random bits is done with Mersenne Twister from U. of Hiroshima
 
 http://en.wikipedia.org/wiki/Mersenne_twister
 http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
*/
class Random
{
private:

    /// Mersenne Twister state variables
    sfmt_t    sfmt;
    
    /// pointers to Mersenne Twister data vector
    uint32_t *sfmt_first, *sfmt_ptr, *sfmt_last;
    
    /// extract next Mersenne Twister random uint32
    inline uint32_t RAN32()
    {
        if ( sfmt_ptr >= sfmt_last )
        {
            sfmt_gen_rand_all(&sfmt);
            sfmt_ptr = sfmt_first;
        }
        return *sfmt_ptr++;
    }


private:
    
    /// the last bit in a 32-bits integer
    static const uint32_t BIT31 = 1U << 31, FRAC32 = 0x7FFFFFU, EXPON32 = 127U << 23;
    
    /// exponent for a double precision float
    static const uint64_t BIT63 = 1ULL << 63, EXPON64 = 1023ULL << 52;
    
    /// used by gauss()
    real bufferValue;
    
    /// used by gauss()
    bool bufferValid;

public:
            
    /// Constructor
    Random();
    
    /// destructor
    ~Random();
    
    /// seed with integer
    void      seed(const uint32_t s)     { sfmt_init_gen_rand(&sfmt, s); }
    
    /// seed with time()
    uint32_t  seedTimer();

    /// unsigned integer in [0,2^32-1]
    uint32_t  pint()                     { return RAN32(); }
    
    /// unsigned integer in [0,n-1] for n < 2^32
    uint32_t  pint_exc(const uint32_t n) { return uint32_t(RAN32() * (n*0x1p-32)); }
    
    /// unsigned integer in [0,n] for n < 2^32
    uint32_t  pint_inc(const uint32_t n) { return pint_exc(n+1); }
    
    /// integer in [0,n] for n < 2^32, (slow) integer based algorithm
    uint32_t  pint_inc2(const uint32_t n);
        
    /// signed integer in [-2^31+1,2^31-1]; inlined for speed
    int32_t   sint() { return static_cast<int32_t>(RAN32());    }
    
    /// integer in [-N, N], boundaries included
    int32_t   sint_inc(const int32_t n)  { return pint_exc( 2*n+1 ) - n; }
    
    /// integer in [1-N, N-1], i.e. in ]-N,N[ with boundaries excluded
    int32_t   sint_exc(const int32_t n)  { return pint_exc( 2*n-1 ) - n+1; }
    
    /// random integer in [low, high]  ( = low + pint_inc(high-low) )
    int32_t   int_range(const int32_t low, const int32_t high) {
        if ( high >= low )
            return low + pint_inc( high - low );
        else
            return high + pint_inc( low - high );
    }

    /// integer in [0 N], with probabilities given in ratio[] of size N, with sum(ratio)>0
    uint32_t  pint_ratio(uint32_t n, const int ratio[]);

    /// integer k of probability distribution p(k,E) = exp(-E) * pow(E,k) / factorial(k)
    uint32_t  poisson(real E);
 
    /// number of successive unsuccessful trials, when success has probability p (result >= 0)
    uint32_t  geometric(real p);

    /// number of sucesses among n trials of probability p
    uint32_t  binomial(int n, real p);
    
    
    /// true with probability (p), false with probability (1-p)
    bool      test(real const& p)        { return ( RAN32() < p * 0x1p32 ); }
    
    /// true with probability (1-p), false with probability (p)
    bool      test_not(real const& p)    { return ( RAN32() >= p * 0x1p32 ); }
    
    /// true with probability p / 2^32
    bool      test_uint(uint32_t p)      { return RAN32() < p; }
    
    /// true  or  false  with equal chance
    bool      flip()                     { return RAN32() & 1024; }
    
    /// returns -1  or  1 with equal chance
    int       sflip()                    { return RAN32() & 1024 ? -1 : 1; }
    
    /// this sign-function returns -1 or +1 randomly, if the argument is 0
    int       sign_exc(const real a);
    
    /// positive real number in [0,1[, zero included
    real      preal()                    { return RAN32() * 0x1p-32; }
    
    /// positive real number in [0,n[ = n * preal() : deprecated, use preal() * n
    //real      preal(const real n)        { return n * ( RAN32() * 0x1p-32 ); }
    
    /// signed real number in ]-1,1[, boundaries excluded
    real      sreal()                    { return (int32_t)(RAN32()) * 0x1p-31; }

    /// non-zero real number in ]0,1]
    real      preal_exc()                { return RAN32() * 0x1p-32 + 0x1p-32;  }
    
    /// non-zero real number in ]0,n]
    real      preal_exc(real n)          { return preal_exc() * n; }
    
    /// real number uniformly in [a,b]
    real      real_range(real a, real b) { return a + preal() * ( b - a ); }
    
    
    /// random Gaussian number, following a normal law N(0,1)
    real      gauss();
   
    /// set two number, following a normal law N(0,1)
    void      gauss_pair(real &, real &);

    /// fill array \a vec with normal law N(0,1).
    void      gauss_array(unsigned int n, real vec[]);

    /// signed real number, following a normal law N(0,1), slower algorithm
    real      gauss_slow();
    
    /// positive real x, according to distribution P(x) = exp(-x), expectancy = 1.0
    real      exponential() { return -log( preal_exc() );  }
    
    /// positive real x, with distribution P(x) = exp(-x/E) / E    : [ E = 1/Rate ]
    real      exponential(const real E) { return -E * log( preal_exc() );  }

    ///uniform choice among the 2 values given:  x,y
    template<typename T>
    T         choice(const T& x, const T& y)
    {
        if ( flip() )
            return x;
        else
            return y;
    }
    
    template<typename T>
    T         choice(const T& x, const T& y, const T& z)
    {
        switch( pint_exc(3) )
        {
            case 0:  return x;
            case 1:  return y;
            default: return z;
        }
    }
    
    template<typename T>
    T         choice(const T val[], int size)
    {
        return val[ pint_exc(size) ];
    }
    
    ///uniform shuffling of array \a T[].
    /** Algorithm from knuth's The Art of Programming, Vol 2 chp. 3.4.2 */
    template <typename T> 
    void mix(T val[], int size)
    {
        int  jj = size, kk;
        while ( jj > 1 )
        {
            kk = RAN32() % jj;
            --jj;
            T tmp   = val[jj];
            val[jj] = val[kk];
            val[kk] = tmp;
        }
    }
        
};



/**
 Linear congruential random number generator
 The coefficients are found in Numerical Recipes 3rd Ed. Chapter 7
 See also http://en.wikipedia.org/wiki/Linear_congruential_generator
 
 The low-order bits should never be relied on for any degree of randomness whatsoever. 
 To use the topmost bit:
 @code 
   if ( z & 0x80000000U )
 @endcode
 */

inline uint32_t lcrng1(uint32_t z) { return z * 2024337845U + 797082193U; }

inline uint32_t lcrng2(uint32_t z) { return z * 279470273U + 4294967291U; }

inline uint32_t lcrng3(uint32_t z) { return z * 1372383749U + 1289706101U; }

inline uint32_t lcrng4(uint32_t z) { return z * 1103515245U + 12345U; }

inline uint32_t lcrng3(uint32_t z, int n)
{
    uint32_t r = z;
    for ( int c = 0; c < n; ++c )
        r = lcrng3(r);
    return r;
}

#endif  //RANDOM_H
