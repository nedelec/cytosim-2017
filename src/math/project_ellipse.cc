

#include <cmath>
#include "real.h"
#include <cstdio>
#include "assert_macro.h"

/**
 Calculate the projection P = (pX, pY) of the point W = (wX, wY) on the ellipse that
 is aligned with the X and Y axis, and has radii (lenX, lenY).
 
 Method:
 
 The normal to the ellipse at position P is N = ( pX / lenX^2, pY / lenY^2 ),
 and we can thus write W = P + h * N, which leads to:
 @code
 pX = wX * lenX^2 / ( lenX^2 + h );
 pY = wY * lenY^2 / ( lenY^2 + h );
 @endcode
 if wX and wY are not both null.
 
 Moreover, the projection should be on the ellipse and thus `h` should be a zero of:
 @code
 F(h) = ( pX / lenX )^2 + ( pY / lenY )^2 - 1
 @endcode
 We follow Newton's rule to find the root of F(h), and use the formula above to
 calculate the projection.

 In double precision (real=double), `tolerance` should be greater than 1e-13.
 */
void projectEllipse2D(real&   pX, real&  pY,
                      real    wX, real   wY,
                      real  lenX, real lenY,
                      const real tolerance)
{
    assert_true( lenX > REAL_EPSILON );
    assert_true( lenY > REAL_EPSILON );
    assert_true( tolerance >=  1e-13 );
    
    // handle the pathological cases:
    if ( wX == 0 )
    {
        pX = 0;
        pY = ( wY > 0 ) ? lenY : -lenY;
        return;
    }
    if ( wY == 0 )
    {
        pX = ( wX > 0 ) ? lenX : -lenX;
        pY = 0;
        return;
    }
    
    real aa = lenX * lenX;
    real bb = lenY * lenY;
    
    real h = 0, dh = INFINITY;
    
    // we derive a lower limit for 'h' from  pX^2 + pY^2 > max(lenX,lenY)^2
    real RR = ( bb < aa ) ? aa : bb;
    // 'hmin' is the minimum value that 'h' could have
    real hmin = sqrt( ( wX*wX*aa*aa + wY*wY*bb*bb ) / RR ) - RR;
    
    // we derive another lower limit for 'h' from  |pX| < lenX
    real hm = ( fabs(wX) - lenX ) * lenX;
    if ( hm > hmin )
        hmin = hm;

    // we derive another lower limit for 'h' from  |pY| < lenY
    hm = ( fabs(wY) - lenY ) * lenY;
    if ( hm > hmin )
        hmin = hm;

    // if the point is outside, then 'h' should be positive:
    if ( wX*wX/aa + wY*wY/bb > 1 )
    {
        if ( hmin < 0 )
            hmin = 0;
        h = hmin;
    }
    else
    {
        // we derive an upper limit for 'h' from  pX^2 + pY^2 > min(lenX,lenY)^2
        real rr = ( aa < bb ) ? aa : bb;
        hm = sqrt( ( wX*wX*aa*aa + wY*wY*bb*bb ) / rr ) - rr;
        if ( hm < 0 )
            h = hm;
    }

    //fprintf(stderr, " <<< %+.10f  %+.10f    hmin %+10.4f", wX, wY, hmin);

    // follow Newton's iteration to find the root
    unsigned cnt = 0;
    while ( fabs(dh) > tolerance )
    {
        real aah = aa + h;
        real bbh = bb + h;

        pX = wX * aa / aah;
        pY = wY * bb / bbh;
         
        real pXX = pX * pX / aa;
        real pYY = pY * pY / bb;
        
        real F    = 1 - ( pXX         + pYY       );
        real dF   = 2 * ( pXX / aah   + pYY / bbh );
        dh = - F / dF;
        
        //fprintf(stderr, "  %i : h %+f  F %+20.16f  dF %+20.16f  dh %e\n", cnt, h, F, dF, dh);
        
        if ( h+dh < hmin )
        {
            h = 0.5 * ( h + hmin );
            continue;
        }

        h += dh;
        
#if ( 0 )
        if ( cnt > 16 )
            fprintf(stderr, "projectEllipse2D fails %u :  h %+f  F %e  dh %e\n", cnt, h, F, dh);
#endif

        if ( ++cnt > 20 )
            break;
    }
    
    //fprintf(stderr, " %2i  >>> h %+12.8f   dh  %+20.16f\n", cnt, h, dh);

    //calculate the projection from h
    pX = wX * aa / ( aa + h );
    pY = wY * bb / ( bb + h );
}






/**
 Calculates the projection P = (pX, pY, pZ) of the point W = (wX, wY, wZ) on the ellipse that
 is aligned with the X and Y axis, and has radii (lenX, lenY, lenZ).
 
 Method:
 
 The normal to the ellipse at position P is N = ( pX / lenX^2, pY / lenY^2, pZ / lenZ^2 ),
 and we can thus write W = P + h * N, for some scalar `h' which leads to:
 @code
 pX = wX / ( 1 + h / lenX^2 );
 pY = wY / ( 1 + h / lenY^2 );
 pZ = wZ / ( 1 + h / lenZ^2 );
 @endcode
 
 Moreover, the projection should be on the ellipse and thus `h` should be a zero of:
 @code
 F(h) = ( pX / lenX )^2 + ( pY / lenY )^2 + ( pZ / lenZ )^2 - 1
 @endcode
 We follow Newton's rule to find the root of F(h), and use the formula above to
 calculate the projection.
 
 In double precision (real=double), `precision` should be greater than 1e-13.
 */
void projectEllipse3D(real  p[3],
                      const real w[3],
                      const real len[3],
                      const real tolerance)
{
    assert_true( len[0] > REAL_EPSILON );
    assert_true( len[1] > REAL_EPSILON );
    assert_true( len[2] > REAL_EPSILON );
    assert_true( tolerance > 1e-13 );

    
    // handle the pathological cases:
    if ( w[0] == 0 )
    {
        p[0] = 0;
        projectEllipse2D(p[1], p[2], w[1], w[2], len[1], len[2], tolerance);
        return;
    }
    if ( w[1] == 0 )
    {
        p[1] = 0;
        projectEllipse2D(p[0], p[2], w[0], w[2], len[0], len[2], tolerance);
        return;
    }
    if ( w[2] == 0 )
    {
        p[2] = 0;
        projectEllipse2D(p[0], p[1], w[0], w[1], len[0], len[1], tolerance);
        return;
    }

    real aa = len[0] * len[0];
    real bb = len[1] * len[1];
    real cc = len[2] * len[2];
    
    real h = 0, dh = INFINITY;

    // we derive a lower limit for 'h' from  pX^2 + pY^2 + pZ^2 < max(lenX,lenY,lenZ)^2
    real RR = ( bb < aa ) ? ( cc < aa ? aa : cc ) : ( cc < bb ? bb : cc );
    // 'hmin' is the minimum value that 'h' can have
    real hmin = sqrt( ( w[0]*w[0]*aa*aa + w[1]*w[1]*bb*bb + w[2]*w[2]*cc*cc ) / RR ) - RR;

    // we derive another lower limit for 'h' from  |pX| < lenX
    real hm = ( fabs(w[0]) - len[0] ) * len[0];
    if ( hm > hmin )
        hmin = hm;

    // we derive another lower limit for 'h' from  |pY| < lenY
    hm = ( fabs(w[1]) - len[1] ) * len[1];
    if ( hm > hmin )
        hmin = hm;
    
    // we derive another lower limit for 'h' from  |pZ| < lenZ
    hm = ( fabs(w[2]) - len[2] ) * len[2];
    if ( hm > hmin )
        hmin = hm;

    // if the point is outside, then 'h' should be positive:
    if ( w[0]*w[0]/aa + w[1]*w[1]/bb + w[2]*w[2]/cc > 1 )
    {
        if ( hmin < 0 )
            hmin = 0;
        h = hmin;
    }
    else
    {
        // we derive an upper limit for 'h' from  pX^2 + pY^2 + pZ^2 > min(lenX,lenY,lenZ)^2
        real rr = ( aa < bb ) ? ( aa < cc ? aa : cc ) : ( bb < cc ? bb : cc );
        hm = sqrt( ( w[0]*w[0]*aa*aa + w[1]*w[1]*bb*bb + w[2]*w[2]*cc*cc ) / rr ) - rr;
        if ( hm < 0 )
            h = hm;
    }

    // follow Newton's iteration to find the largest root
    unsigned cnt = 0;
    while ( fabs(dh) > tolerance )
    {
        real aah = aa + h;
        real bbh = bb + h;
        real cch = cc + h;

        real pX = w[0] * aa / aah;
        real pY = w[1] * bb / bbh;
        real pZ = w[2] * cc / cch;
        
        real pXX = pX * pX / aa;
        real pYY = pY * pY / bb;
        real pZZ = pZ * pZ / cc;

        real   F = 1 - ( pXX         + pYY         + pZZ       );
        real  dF = 2 * ( pXX / aah   + pYY / bbh   + pZZ / cch );

        dh = - F / dF;
        
        if ( h+dh < hmin )
        {
            h = 0.5 * ( h + hmin );
            continue;
        }
        
        h += dh;

#if ( 0 )
        if ( cnt > 16 )
        {
            fprintf(stderr, "projectEllipse3D fails %u :  h %+f  F %.6e  dh %.6e\n", cnt, h, F, dh);
            //fprintf(stderr, "    pos  %+.10f     %+.10f       %+.10f\n", w[0], w[1], w[2]);
            //fprintf(stderr, "    F    %+.10f  dF %+.10f   ddF %+.10f\n", F, dF, ddF);
        }
#endif

        if ( ++cnt > 20 )
            break;
    }

    //fprintf(stderr, " %2i  >>> h %12.8f  dh  %20.16f\n", cnt, h, dh);

    //calculate the projection from h
    p[0] = w[0] * aa / ( aa + h );
    p[1] = w[1] * bb / ( bb + h );
    p[2] = w[2] * cc / ( cc + h );
}


