// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
/**
 Some basic mathematical functions
 Francois Nedelec, 
*/

#ifndef SMATH_H
#define SMATH_H

#include "real.h"
#include <cmath>
#include <sstream>
#include <stdint.h>


#ifndef M_PI
/// Ratio of a circle's perimeter to its radius:
const double M_PI = 3.14159265358979323846264338327950288;
#endif

#ifndef M_E
const double M_E  = 2.71828182845904523536028747135266250;
#endif


/// simple mathematical functions, mostly templated
namespace sMath
{    
    ///maximum of 2 values
    template <typename T> 
    inline const T& max(const T& a, const T& b)
    {
        return (a > b) ? a : b;
    }
    
    ///minimum of 2 values
    template <typename T> 
    inline const T& min(const T& a, const T& b)
    {
        return (a > b) ? b : a;
    }
    
    // limit \a x inside [\a a, \a b]:
    template <typename T> 
    inline const T& constrain(T& x, const T& a, const T& b)
    {
        if ( x < a ) 
            return a;
        if ( x > b )
            return b;
        return x;
    }
    
    
    ///maximum of three arguments
    template <typename T>
    inline const T& max(const T& a, const T& b, const T& c)
    {
        if ( a > b )
            return (a > c) ? a : c;
        else
            return (b > c) ? b : c;
    }
    
    ///minimum of three arguments
    template <typename T>
    inline const T& min(const T& a, const T& b, const T& c)
    {
        if ( a > b )
            return (b < c) ? b : c;
        else
            return (a < c) ? a : c;
    }
    
    /// the sign of a number
    template <typename T> 
    inline int sign(const T& a)
    {
        return (a > 0) ? 1 : ( (a < 0) ? -1 : 0 );
    }
    
    /// square of a number
    template <typename T> 
    inline T sqr(const T& a)
    {
        return a * a;
    }
    
    /// cube of a number
    template <typename T>
    inline T cub(const T& a)
    {
        return a * a * a;
    }
    
    ///fourth power of a number
    template <typename T>
    inline T quad(const T& a)
    {
        T b = a * a;
        return b * b;
    }
    
    /// power of `a` by positive integer exponent `n`
    /** This should be equivalent to std::pow(a, n) */
    template <typename T>
    inline T power_int(const T& a, unsigned n)
    {
        T x = a;
        T y = 1;
        while ( n )
        {
            if ( n & 1 )
                y = y * x;
            x = x * x;
            n = n >> 1;
        }
        return y;
    }
    
    
    ///power of `a` by integer exponent `n`
    template <typename T>
    inline T power(const T& a, const int n)
    {
        if ( n < 0 )
            return power_int(1.0/a, -n);
        return power_int(a, n);
    }
    
    
    ///power of \a by integer exponent \a n
    template <typename T>
    T nextPowerOf2(T n)
    {
        if ( n & n-1 )
        {
            // zeros all lower bits:
            do
                n = n & n-1;
            while ( n & n-1 );
            
            return n << 1;
        }
        // n is already a power of 2
        return n;
    }    
    
    ///swap two numbers
    template <typename T> 
    inline void swap(T & a, T & b)
    {
        T tmp = a;
        a = b;
        b = tmp;
    }
    
    /// return the usual base-10 representation of a number
    template <typename T>
    std::string repr(T & x)
    {
        std::ostringstream oss;
        oss << x;
        return oss.str();
    }
    
    //------------------------------------------------------------------------------
#pragma mark -
    
#ifdef WIN32
    
    //this is needed under windows:
    inline real remainder( const real a, const real b )
    {
        int p = (int)floor( 0.5 + a / b );
        if ( p )
            return a - p * b;
        else
            return a;
    }
    
    inline real round(real x)
    {
        if ( x < 0 )
            return  -floor(-x + 0.5);
        else
            return   floor(x + 0.5);
    }

#endif
    
    //------------------------------------------------------------------------------
#pragma mark -
    
    ///extract a 10-decimal digit form a number:
    /** 1st digit is really the first one, we do not start at zero */
    template <typename T> 
    inline int digit(T x, const int p)
    {
        for ( int q=1; q < p; ++q )
            x /= 10;
        return x % 10;
    }
    
    ///copy bytes
    inline void copyBytes( void * dest, const void * src, const unsigned cnt)
    {
        for ( unsigned ii=0; ii < cnt; ++ii )
            ((char*)dest)[ii] = ((char*)src)[ii];
    }
    

    //------------------------------------------------------------------------------
    
    /// number of '1' bits in a 32-bits integer (Charlie Gordon & Don Clugston)
    /** Should use Intel SIMD instruction POPCNT */
    inline unsigned int count_bits(uint32_t v)
    {
        v = v - ((v >> 1) & 0x55555555);
        v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
        return (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
    }
    
    
    /// number of '1' bits, from: http://graphics.stanford.edu/~seander/bithacks.html
    /** Works up to 128 bits */
    template <typename T>
    unsigned int count_bits2(T v)
    {
        v = v - ((v >> 1) & (T)~(T)0/3);
        v = (v & (T)~(T)0/15*3) + ((v >> 2) & (T)~(T)0/15*3);
        v = (v + (v >> 4)) & (T)~(T)0/255*15;
        return (T)(v * ((T)~(T)0/255)) >> (sizeof(v) - 1) * 8;
    }
   
}


#endif //#ifdef SMATH_H
