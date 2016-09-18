// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef ROTATION_H
#define ROTATION_H

#include "dim.h"


#if (DIM==1)

   #include "matrix1.h"
   typedef Matrix1 MatrixD;

#elif (DIM==2)

   #include "matrix2.h"
   typedef Matrix2 MatrixD;

#elif (DIM==3)

   #include "matrix3.h"
   typedef Matrix3 MatrixD;

#else

    #error 'DIM' is not properly defined!

#endif


/// A Rotation is a matrix of dimension DIM x DIM
typedef MatrixD Rotation;


#endif
