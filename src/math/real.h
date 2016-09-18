// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
//SYNOPSIS: we define and use "real" to be able to easily change
//the floating point Precision, which should normally be double
//REAL_EPSILON is a lower limit of the precision achieved

/// @file

#ifndef REAL
#define REAL

#include <limits>

/**
 It is possible to select double or single precision here: 
 - To use float, define REAL_IS_FLOAT below
 - To use double, do not define REAL_IS_FLOAT
 .
 
 Cytosim is faster in single precision, but the iterative solver used
 in Meca::solve() (conjugate-gradient) may fail in adverse conditions.
 
 It is safer, and STRONGLY ADVISED therefore, to use double precision,
 and to not define 'REAL_IS_FLOAT'
*/
//#define REAL_IS_FLOAT

#ifdef REAL_IS_FLOAT
    typedef float real;
#else
    typedef double real;
#endif


/// REAL_EPSILON is a multiple of the round-off error associated with \c real
const real REAL_EPSILON = 4096 * std::numeric_limits<real>::epsilon();


#endif
