// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "space_banana.h"
#include "exceptions.h"

SpaceBanana::SpaceBanana(const SpaceProp* p)
: Space(p)
{
    if ( DIM == 1 )
        throw InvalidParameter("banana is not usable in 1D");
    //resize();
}


void SpaceBanana::resize()
{
    checkLengths(3, true);
    
    bLength   = mLength[0] - 2 * mLength[1];
    bWidth    = mLength[1];
    bWidthSqr = bWidth * bWidth;
    bRadius   = mLength[2];
    
    if ( bLength <= 0 )
        throw InvalidParameter("banana:length must be specified and greater than 2 * width");

    if ( bRadius <= 0 )
        throw InvalidParameter("banana:radius must be specified and > 0");

    if ( bWidth > bRadius )
        throw InvalidParameter("banana:width must be smaller than radius");
    
    bAngle = 0.5 * bLength / bRadius;
    
    if ( bAngle > M_PI )
    {
        bAngle = M_PI;
        std::cerr << "banana:length should not exceed 2*PI*radius" << std::endl;
    }

    bEnd[0] = bRadius * sin(bAngle);
    bEnd[1] = 0.5*bRadius*(1-cos(bAngle));
    
    bCenter[0] = 0;
    bCenter[1] = bRadius - bEnd[1];
}


real SpaceBanana::volume() const
{
#if ( DIM == 3 )
    return 2*bAngle*M_PI*bRadius*bWidthSqr + 4/3.*M_PI*bWidthSqr*bWidth;
#else
    return 4*bAngle*bRadius*bWidth + M_PI*bWidthSqr;
#endif
}


Vector SpaceBanana::extension() const
{
    return Vector(bEnd[0]+bWidth, bEnd[1]+bWidth, bWidth);
}


/// project on the backbone circular arc in the XY plane:
void SpaceBanana::project0(const real pos[], real prj[]) const
{
    real cp[3];
    
    for( int d=0; d<DIM; ++d )
        cp[d] = pos[d] - bCenter[d];
    
    real n = bRadius / sqrt( cp[0]*cp[0] + cp[1]*cp[1] );

    prj[0] = bCenter[0] + n * cp[0];
    prj[1] = bCenter[1] + n * cp[1];
    
    if ( prj[1] > bEnd[1] )
    {
        prj[0] = pos[0] > 0 ? bEnd[0] : -bEnd[0];
        prj[1] = bEnd[1];
    }
    
    if ( DIM == 3 )
        prj[2] = 0;
}


bool SpaceBanana::inside( const real pos[] ) const
{
    real prj[3];
    
    project0(pos, prj);
    
    real n = ( pos[0] - prj[0] )*( pos[0] - prj[0] );
    for( int d=1; d<DIM; ++d )
        n += ( pos[d] - prj[d] )*( pos[d] - prj[d] );
    
    return ( n <= bWidthSqr );
}


void SpaceBanana::project( const real pos[], real prj[]) const
{
    real cen[3], ax[3];
    project0(pos, cen);
    
    for( int d=0; d<DIM; ++d )
        ax[d] = pos[d] - cen[d];

    real n = ax[0] * ax[0];
    for( int d=1; d<DIM; ++d )
        n += ax[d] * ax[d];
    
    n = bWidth / sqrt(n);
    
    for( int d=0; d<DIM; ++d )
        prj[d] = cen[d] + n * ax[d];
}


//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY
#include "glut.h"
#include "gle.h"
using namespace gle;

bool SpaceBanana::display() const
{
#if ( DIM == 2 )
    
    real r1 = bRadius - bWidth;
    real r2 = bRadius + bWidth;
    const real daa = 0.01;
    glBegin(GL_LINE_LOOP);
    //the lower swing
    for ( real aa = -bAngle; aa <= bAngle;  aa += daa )
        gleVertex( bCenter[0]+r2*sin(aa), bCenter[1]-r2*cos(aa), 0 );
    //the right cap
    for ( real aa = bAngle-M_PI/2; aa <= bAngle+M_PI/2; aa += daa )
        gleVertex( bEnd[0] + bWidth*cos(aa), bEnd[1] + bWidth*sin(aa), 0 );
    //the upper swing
    for ( real aa = bAngle;  aa >= -bAngle; aa -= daa )
        gleVertex( bCenter[0]+r1*sin(aa), bCenter[1]-r1*cos(aa), 0 );
    //the left cap
    for ( real aa = M_PI/2-bAngle;  aa <= 1.5*M_PI-bAngle; aa += daa )
        gleVertex( -bEnd[0] + bWidth*cos(aa), bEnd[1] + bWidth*sin(aa), 0 );
    glEnd();
    
#elif ( DIM == 3 )
    
    const int fin = 4 * gle::finesse;
    glMatrixMode(GL_MODELVIEW);

    const GLenum glp1 = GL_CLIP_PLANE4;
    const GLenum glp2 = GL_CLIP_PLANE5;
    GLdouble planeX[] = { +1, 0, 0, 0 };
    GLdouble plane1[] = { -cos(bAngle), -sin(bAngle), 0, 0 };
    GLdouble plane2[] = {  cos(bAngle), -sin(bAngle), 0, 0 };
    glEnable(glp1);
    glEnable(glp2);
    
    //right side:
    glPushMatrix();
    glTranslated(bCenter[0], bCenter[1], 0);
    glClipPlane(glp1, planeX);
    glClipPlane(glp2, plane1);
    glutSolidTorus(bWidth, bRadius, 2*fin, 2*fin);
    
    //right cap:
    glDisable(glp1);
    plane1[0] = -plane1[0];
    plane1[1] = -plane1[1];
    glClipPlane(glp2, plane1);
    glTranslated(bEnd[0]-bCenter[0], bEnd[1]-bCenter[1], 0);
    glutSolidSphere(bWidth, fin, fin);
    glPopMatrix();
    
    glEnable(glp1);
    planeX[0] = -planeX[0];
    //left side:
    glPushMatrix();
    glTranslated(bCenter[0], bCenter[1], 0);
    glClipPlane(glp1, planeX);
    glClipPlane(glp2, plane2);
    glutSolidTorus(bWidth, bRadius, 2*fin, 2*fin);
    
    //left cap:
    glDisable(glp1);
    plane2[0] = -plane2[0];
    plane2[1] = -plane2[1];
    glClipPlane(glp2, plane2);
    glTranslated(-bEnd[0]-bCenter[0], bEnd[1]-bCenter[1], 0);
    glutSolidSphere(bWidth, fin, fin);
    glPopMatrix();
    
    glDisable(glp2);
    
#endif
    return true;
}

#else

bool SpaceBanana::display() const
{
    return false;
}


#endif
