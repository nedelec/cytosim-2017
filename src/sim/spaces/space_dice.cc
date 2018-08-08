// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "space_dice.h"
#include "exceptions.h"


SpaceDice::SpaceDice(const SpaceProp* p)
: Space(p)
{
    if ( DIM == 1 )
        throw InvalidParameter("dice is only valid in DIM=2 or 3");
}

/**
 The `dice' is included in the rectangle
 */
Vector SpaceDice::extension() const
{
    return Vector( length(0), length(1), length(2) );
}

/**
 If `radius==0`, the volume should be the volume of a rectangle
 */
real SpaceDice::volume() const
{
#if ( DIM == 1 )
    return 2 * length(0);
#elif ( DIM == 2 )
    return 4 * length(0)*length(1) + (M_PI-4)*radius()*radius();
#else
    return 8 * length(0)*length(1)*length(2)
    + 2 * (M_PI-4) * ( length(0) + length(1) + length(2) - 3 * radius() ) * radius() * radius()
    + (4/3.0 * M_PI - 8) * radius()*radius()*radius();
#endif
}


//------------------------------------------------------------------------------

bool  SpaceDice::inside( const real w[] ) const
{
    int d;
    real sw[3];
    
    for ( d = 0; d < DIM; ++d )
    {
        sw[d] = fabs( w[d] );
        if ( sw[d] > length(d) )
            return false;
    }
    
    real n = 0;
    for ( d = 0; d < DIM; ++d )
    {
        sw[d] += radius() - length(d);
        if ( sw[d] > 0 )
            n += sw[d]*sw[d];
    }
    
    return ( n <= radiusSqr() );
}



//------------------------------------------------------------------------------

void SpaceDice::project( const real w[], real p[] ) const
{
    bool in = true;
    
    //calculate the projection on the inner cube disminished from radius()
    for ( int d = 0; d < DIM; ++d )
    {
        p[d] = w[d];
        
        if ( p[d] >  length(d) - radius() )
        {
            p[d] = length(d) - radius();
            in = false;
        }
        
        if ( p[d] <  radius() - length(d) )
        {
            p[d] = radius() - length(d);
            in = false;
        }
    }
    
    real dis = 0;
    
    if ( in )
    {
        int dip = 0;
        dis = length(0) - fabs(w[0]);
        
        for ( int d = 1; d < DIM; ++d )
        {
            real test = length(d) - fabs(w[d]);
            if ( test < dis )
            {
                dip = d;
                dis = test;
            }
        }
        
        if ( w[dip] > 0 )
            p[dip] =  length(dip);
        else
            p[dip] = -length(dip);
    }
    else
    {
        //calculate the distance to the projection:
        for ( int d = 0; d < DIM; ++d )
            dis += ( w[d] - p[d] ) * ( w[d] - p[d] );
        
        //normalize to radius(), and add to p to get the real projection
        dis = radius() / sqrt(dis);
        for ( int d = 0; d < DIM; ++d )
            p[d] += dis * ( w[d] - p[d] );
        
    }
}

//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY
#include "opengl.h"
#include "gle.h"
using namespace gle;

bool SpaceDice::display() const
{
#if ( DIM == 3 )
    
    const real X = length(0) - radius();
    const real Y = length(1) - radius();
    const real Z = length(2) - radius();
 
    const real XR = length(0);
    const real YR = length(1);
    const real ZR = length(2);

    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  XR,  Y, -Z );
    gleVertex(  XR,  Y,  Z );
    gleVertex(  XR, -Y, -Z );
    gleVertex(  XR, -Y,  Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex( -XR, -Y, -Z );
    gleVertex( -XR, -Y,  Z );
    gleVertex( -XR,  Y, -Z );
    gleVertex( -XR,  Y,  Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X,  YR, -Z );
    gleVertex( -X,  YR, -Z );
    gleVertex(  X,  YR,  Z );
    gleVertex( -X,  YR,  Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X, -YR,  Z );
    gleVertex( -X, -YR,  Z );
    gleVertex(  X, -YR, -Z );
    gleVertex( -X, -YR, -Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X,  Y,  ZR );
    gleVertex( -X,  Y,  ZR );
    gleVertex(  X, -Y,  ZR );
    gleVertex( -X, -Y,  ZR );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X,  Y, -ZR );
    gleVertex(  X, -Y, -ZR );
    gleVertex( -X,  Y, -ZR );
    gleVertex( -X, -Y, -ZR );
    glEnd();
    
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    
    glLineStipple(1, 0x0303);
    glEnable(GL_LINE_STIPPLE);
    displaySection( 0, -X, 0.01 );
    displaySection( 0,  X, 0.01 );
    displaySection( 1, -Y, 0.01 );
    displaySection( 1,  Y, 0.01 );
    displaySection( 2, -Z, 0.01 );
    displaySection( 2,  Z, 0.01 );
    glDisable(GL_LINE_STIPPLE);
    glPopAttrib();
    
#else

    displaySection( 2, 0, 0.01 );

#endif
    
    return true;
}

#else

bool SpaceDice::display() const
{
    return false;
}

#endif


