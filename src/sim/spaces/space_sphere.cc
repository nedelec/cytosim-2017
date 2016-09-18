// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "space_sphere.h"
#include "exceptions.h"
#include "random.h"
#include "smath.h"
#include "meca.h"

extern Random RNG;

SpaceSphere::SpaceSphere(const SpaceProp* p)
: Space(p)
{
    if ( radius() < 0 )
        throw InvalidParameter("sphere:radius should be specified and >= 0");
}


#if (DIM == 1)

Vector SpaceSphere::extension() const
{
    return Vector(radius(), 0, 0);
}

real SpaceSphere::volume() const
{
    return 2 * radius();
}

bool SpaceSphere::inside( const real point[] ) const
{
    return point[0] * point[0] <= radiusSqr();
}

void SpaceSphere::project( const real point[], real proj[] ) const
{
    real n = point[0] * point[0];
    
    if ( n > 0 ) {
        n = radius() / sqrt( n );
        proj[0] = n * point[0];
    }
    else {
        proj[0] = RNG.sflip() * radius();
    }
}

#endif


//------------------------------------------------------------------------------


#if (DIM == 2)

Vector SpaceSphere::extension() const
{
    return Vector(radius(), radius(), 0);
}

real SpaceSphere::volume() const
{
    return M_PI * radius() * radius();
}

bool SpaceSphere::inside( const real point[] ) const
{
    return point[0] * point[0] + point[1] * point[1] <= radiusSqr();
}

void SpaceSphere::project( const real point[], real proj[] ) const
{    
    real n = point[0] * point[0] + point[1] * point[1];
    
    if ( n > 0 ) {
        n = radius() / sqrt( n );
        proj[0] = n * point[0];
        proj[1] = n * point[1];
    }
    else {
        //select a random point on the surface
        real x, y;
        do {
            x = RNG.sreal();
            y = RNG.sreal();
            n = x*x + y*y;
        } while ( n > 1.0  ||  n == 0 );
        n = radius() / sqrt( n );
        proj[0] = n * x;
        proj[1] = n * y;
    }
}

#endif


//------------------------------------------------------------------------------

#if (DIM == 3)

Vector SpaceSphere::extension() const
{
    return Vector(radius(), radius(), radius());
}

real SpaceSphere::volume() const
{
    return 4/3.0 * M_PI * radius() * radius() * radius();
}

bool SpaceSphere::inside( const real point[] ) const
{
    return point[0] * point[0] + point[1] * point[1] + point[2] * point[2] <= radiusSqr();
}

void SpaceSphere::project( const real point[], real proj[] ) const
{    
    real n = point[0] * point[0] + point[1] * point[1] + point[2] * point[2];
    
    if ( n > 0 ) {
        n = radius() / sqrt( n );
        proj[0] = n * point[0];
        proj[1] = n * point[1];
        proj[2] = n * point[2];
    }
    else {
        //select a random point on the surface
        real x, y, z;
        do {
            x = RNG.sreal();
            y = RNG.sreal();
            z = RNG.sreal();
            n = x*x + y*y + z*z;
        } while ( n > 1.0  ||  n == 0 );
        n = radius() / sqrt( n );
        proj[0] = n * x;
        proj[1] = n * y;
        proj[2] = n * z;
    }
}

#endif

//------------------------------------------------------------------------------

void SpaceSphere::setInteraction(Vector const& pos, PointExact const& pe, Meca & meca, real stiff) const
{
    meca.interLongClamp( pe, Vector(0,0,0), radius(), stiff );
}


void SpaceSphere::setInteraction(Vector const& pos, PointExact const& pe, real rad, Meca & meca, real stiff) const
{
    if ( radius() > rad )
        meca.interLongClamp( pe, Vector(0,0,0), radius()-rad, stiff );
    else {
        meca.interClamp( pe, Vector(0,0,0), stiff );
        std::cerr << "object is too big to fit in SpaceSphere\n";
    }
}

//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY

#include "glut.h"
#include "gle.h"

bool SpaceSphere::display() const
{
#if ( DIM <= 2 )
    
    GLfloat R = radius();
    glBegin(GL_LINE_LOOP);
    for ( real aa = 0; aa < 6.28; aa += 0.01 )
        glVertex2f( R*cosf(aa), R*sinf(aa) );
    glEnd();

#else


    const int fin = 8*gle::finesse;
    glPushMatrix();
    gle::gleScale(radius());
    //draw a transparent sphere with GLU
    static GLUquadricObj * qobj = 0;
    if ( qobj == 0 )
    {
        qobj = gluNewQuadric();
        gluQuadricDrawStyle(qobj, GLU_FILL);
        //gluQuadricOrientation(qobj, GLU_OUTSIDE);
        gluQuadricNormals(qobj, GLU_SMOOTH);
    }
    gluSphere(qobj, 1, fin, fin);
    if ( 1 )
    {
        GLfloat width = 0.02;
        gle::gleArrowedBand(width);
        glRotated(-90,1,0,0);
        gle::gleArrowedBand(width);
        glRotated(90,0,1,0);
        gle::gleArrowedBand(width);
    }
    glPopMatrix();
#endif
    
    return true;
}

#else

bool SpaceSphere::display() const
{
    return false;
}


#endif
