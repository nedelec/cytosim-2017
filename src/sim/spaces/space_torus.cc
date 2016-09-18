// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "space_torus.h"
#include "exceptions.h"

SpaceTorus::SpaceTorus(const SpaceProp* p)
: Space(p)

{
    if ( DIM == 1 )
        throw InvalidParameter("torus is not usable in 1D");
}

void SpaceTorus::resize()
{
    checkLengths(2, true);

    bRadius   = mLength[0];
    bWidth    = mLength[1];
    bWidthSqr = bWidth * bWidth;

    if ( bWidth > bRadius )
        throw InvalidParameter("torus:width must be < radius");
}


real SpaceTorus::volume() const
{
#if (DIM == 3)
    return 2 * M_PI * M_PI * bRadius * bWidthSqr;
#else
    return 4 * M_PI * bRadius * bWidth;
#endif
}


Vector SpaceTorus::extension() const
{
    return Vector( bRadius+bWidth, bRadius+bWidth, bWidth );
}


///project on the backbone circle in the XY plane:
void SpaceTorus::project0(const real pos[], real prj[]) const
{
    real n = bRadius / sqrt( pos[0]*pos[0] + pos[1]*pos[1] );

    prj[0] = n * pos[0];
    prj[1] = n * pos[1];
    
    if ( DIM == 3 )
        prj[2] = 0;
}


bool SpaceTorus::inside( const real pos[] ) const
{
    real prj[3];
    project0(pos, prj);
    
    real n = ( pos[0] - prj[0] ) * ( pos[0] - prj[0] );
    for( int d=1; d<DIM; ++d )
        n += ( pos[d] - prj[d] ) * ( pos[d] - prj[d] );
    
    return ( n <= bWidthSqr );
}


void SpaceTorus::project( const real pos[], real prj[]) const
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

bool SpaceTorus::display() const
{
#if ( DIM == 2 )
    
    GLfloat r1 = bRadius - bWidth;
    GLfloat r2 = bRadius + bWidth;
    const GLfloat da = M_PI / 360;
    
    glBegin(GL_LINE_LOOP);
    //inner ring
    for ( GLfloat a = -M_PI; a <= M_PI;  a += da )
        gleVertex(r1*cosf(a), r1*sinf(a), 0);
    glEnd();
    
    //outer ring
    glBegin(GL_LINE_LOOP);
    for ( GLfloat a = -M_PI; a <= M_PI;  a += da )
        gleVertex(r2*cosf(a), r2*sinf(a), 0);
    glEnd();
    return true;
    
#elif ( DIM == 3 )
    
    const int fin = 4 * gle::finesse;
        glutSolidTorus(bWidth, bRadius, fin, fin);
    return true;
    
#else
    return false;
#endif
}

#else

bool SpaceTorus::display() const
{
    return false;
}


#endif
