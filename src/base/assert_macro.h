// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
/**
 @file
 @brief Cytosim's assertions
 */

#ifndef ASSERT_MACRO_H
#define ASSERT_MACRO_H

#include <cstdio>
#include <cstdlib>
#include <cstring>

/**
 defining NDEBUG disables: 
 - the standard assert() macro and
 - the assert macros defined below, 
 it makes the executable faster but less safe:
 */
//#define NDEBUG



/// strips the full pathname for a file name
#define SFILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__


/// print the current function name:
//#define SFUNC __func__
#define SFUNC __PRETTY_FUNCTION__

//-------------------------------  TRACE  ---------------------------------------

#define TRACE fprintf(stderr, "      while executing '%s'\n      in file %s line %d\n", SFUNC, SFILE, __LINE__);

//------------------------------- ASSERT ---------------------------------------
/** 
  assert_true(X) stops the program if condition X is false.
  assert_false(X) stops if X is true, and prints the value of X.
*/

#ifdef NDEBUG

  #define assert_true(ignore)  ((void) 0)
  #define assert_false(ignore) ((void) 0)
  #define assert_small(ignore) ((void) 0)

#else

  #include "backtrace.h"

  #define assert_true(expression)\
        if (!(expression)) {\
            fprintf(stderr, "* * * * * * *\n");\
            fprintf(stderr, "Cytosim failed assertion `%s'\n", #expression);\
            TRACE;\
            fprintf(stderr, "* * * * * * *\n");\
            print_backtrace(stderr);\
            abort();\
        }

  #define assert_false(expression)\
        { int e = expression;\
        if (e) {\
            fprintf(stderr, "* * * * * * *\n");\
            fprintf(stderr, "Cytosim failed assert_false(%s) with value %i\n", #expression, e);\
            TRACE;\
            fprintf(stderr, "* * * * * * *\n");\
            print_backtrace(stderr);\
            abort();\
        } }

  #define assert_small(expression)\
        { real e = expression;\
        if ( e > REAL_EPSILON || e < -REAL_EPSILON ) {\
            fprintf(stderr, "- - - - -\n");\
            fprintf(stderr, "Cytosim failed assert_small(%s) with value %e\n", #expression, e);\
            TRACE;\
        } }

#endif


//-------------------------- ERROR HANDLING MACROS -----------------------------


/// macro to abort after printing an error message
#define ABORT_NOW(message)\
    {\
    fprintf(stderr, "Cytosim ERROR `%s'\n", message);\
    TRACE;\
    exit(EXIT_FAILURE);\
    }


#endif
