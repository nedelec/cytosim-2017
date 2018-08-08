// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/*
 Tests Intel's Streaming SIMD
 F. Nedelec, HD, July 2013
 
 To compile: c++ -O4 tictoc.cc -mavx test.cc
 To generate assembly: c++ -S test.cc
 */

#include <cstdio>
#include "tictoc.h"

// intel SSE3:
#include <emmintrin.h>

typedef double real;


typedef __m128d vec2;
#define SSE(x) _mm_##x##_pd

union uvec2 {
    vec2  v;
    real  s[2];
};

real sum(uvec2 const& u) { return u.s[0]+u.s[1]; }


#ifdef __AVX__

#include <immintrin.h>

typedef __m256d vec4;
#define AVX(x) _mm256_##x##_pd

union uvec4 {
    vec4  v;
    real  s[4];
};

real sum(uvec4 const& u) { return u.s[0]+u.s[1]+u.s[2]+u.s[3]; }

#endif


const unsigned size = 1<<10;

real a[size], b[size], c[size];


void init()
{
    for ( unsigned ii=0; ii<size; ++ii )
    {
        a[ii] = 0.5/real(size-ii);
        b[ii] = 2.0/real(size-ii);
        c[ii] = 0.0;
    }
}



real vector2()
{
    vec2 s = SSE(setzero)();
    for ( unsigned ii=0; ii<size; ii+=2 )
        s = SSE(add)(s, SSE(mul)( SSE(load)(a+ii), SSE(load)(b+ii) ));
    _mm_empty();
    
    uvec2 res;
    res.v = s;
    return sum(res);
}

#ifdef __AVX__


real vector4()
{
    vec4 s = AVX(setzero)();
    for ( unsigned ii=0; ii<size; ii+=4 )
        s = AVX(add)(s, AVX(mul)( AVX(load)(a+ii), AVX(load)(b+ii) ));
    _mm_empty();
    
    uvec4 res;
    res.v = s;
    return sum(res);
}


real vectorU()
{
    vec4 x = AVX(setzero)();
    vec4 y = AVX(setzero)();
    vec4 z = AVX(setzero)();
    vec4 t = AVX(setzero)();
    
    for ( unsigned ii=0; ii<size; ii+=16 )
    {
        x = AVX(add)(x, AVX(mul)( AVX(load)(a+ii   ), AVX(load)(b+ii   ) ));
        y = AVX(add)(y, AVX(mul)( AVX(load)(a+ii+4 ), AVX(load)(b+ii+4 ) ));
        z = AVX(add)(z, AVX(mul)( AVX(load)(a+ii+8 ), AVX(load)(b+ii+8 ) ));
        t = AVX(add)(t, AVX(mul)( AVX(load)(a+ii+12), AVX(load)(b+ii+12) ));
    }
    
    x = AVX(add)(AVX(add)(x, y), AVX(add)(z, t));
    _mm_empty();
    
    uvec4 res;
    res.v = x;
    return sum(res);
}

#else

real vector4()
{
    return 0;
}

real vectorU()
{
    return 0;
}

#endif


real scalar()
{
    real d = 0;
    for ( unsigned ii=0; ii<size; ++ii )
        d += a[ii] * b[ii];
    return d;
}


void run(real (*func)(), const char name[])
{
    const int rep = 1<<9;
    real a, b, c, d, e, f, g, h;
    fprintf(stderr, "%s:  ", name);
    TicToc::tic();
    for ( int ii=0; ii<rep; ++ii )
    {
        a = (*func)();
        b = (*func)();
        c = (*func)();
        d = (*func)();
        e = (*func)();
        f = (*func)();
        g = (*func)();
        h = (*func)();
    }
    
    double ms = TicToc::toc();
    fprintf(stderr, " %f :  %.0f ms\n", a, ms);
}


int main(int argc, char * argv[])
{
    init();
    run(scalar,  "scalar ");
    run(vector2, "vector2");
    run(vector4, "vector4");
    run(vectorU, "vectorU");
    
    return 0;
}

