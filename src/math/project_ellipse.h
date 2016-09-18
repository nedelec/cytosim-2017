// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef PROJECT_ELLIPSE_H
#define PROJECT_ELLIPSE_H


#include "real.h"


/// calculate `(pX, pY)`, the projection of `(wX, wY)` on the ellipse of axes `lenX, lenY`
void projectEllipse2D(real& pX,        real& pY,
                      const real wX,   const real wY,
                      const real lenX, const real lenY,
                      const real precision = 8 * REAL_EPSILON);


/// calculate `p`, the projection of a 3D point `w` on the ellipse of axes given in `len[]`
void projectEllipse3D(real p[3],
                      const real w[3],
                      const real len[3],
                      const real precision = 8 * REAL_EPSILON);



#endif
