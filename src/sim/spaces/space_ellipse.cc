// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "space_ellipse.h"
#include "exceptions.h"
#include "project_ellipse.h"
#include "smath.h"


inline real sqrE(const real x) { return x*x; }


SpaceEllipse::SpaceEllipse(const SpaceProp* p)
: Space(p)
{
}


void SpaceEllipse::resize()
{
    Space::checkLengths(DIM, true);
}


Vector SpaceEllipse::extension() const
{
    return Vector( length(0), length(1), length(2) );
}


Vector SpaceEllipse::normalToEdge(const real point[]) const
{
    return Vector(point[0]/length(0), point[1]/length(1), point[2]/length(2)).normalized();
}

#if (DIM == 1)

real SpaceEllipse::volume() const
{
    return 2 * length(0);
}

bool  SpaceEllipse::inside( const real w[] ) const
{
    return (( w [0] >= -length(0) ) && ( w [0] <=  length(0) ));
}

#elif (DIM == 2)

real SpaceEllipse::volume() const
{
    return M_PI * length(0) * length(1);
}

bool  SpaceEllipse::inside( const real w[] ) const
{
    return ( sqrE( w[0] / length(0) ) + sqrE( w[1] / length(1) ) <= 1 );
}

#else

real SpaceEllipse::volume() const
{
    return 4/3.0 * M_PI * length(0) * length(1) * length(2);
}

bool SpaceEllipse::inside( const real w[] ) const
{
    return  sqrE( w[0]/length(0) ) + sqrE( w[1]/length(1) ) + sqrE( w[2]/length(2)) <= 1;
}

#endif



void SpaceEllipse::project1D( const real w[], real p[] ) const
{
    if ( w[0] >= 0 )
        p[0] =  length(0);
    else
        p[0] = -length(0);
}



void SpaceEllipse::project2D( const real w[], real p[] ) const
{
    projectEllipse2D(p[0], p[1], w[0], w[1], length(0), length(1), REAL_EPSILON);
}


void SpaceEllipse::project3D( const real w[], real p[] ) const
{
    projectEllipse3D(p, w, mLength, REAL_EPSILON);
}


//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

// Modified from sphere by Aastha Mathur, 18th Jan 2013

#ifdef DISPLAY

#include "glut.h"
#include "gle.h"

bool SpaceEllipse::display() const
{
    
#if ( DIM == 2 )
    
    
    GLfloat X = length(0);
    GLfloat Y = length(1);
    glBegin(GL_LINE_LOOP);
    for ( real aa = 0; aa < 6.28; aa += 0.01 )
        glVertex2f( X*cosf(aa), Y*sinf(aa) );
    glEnd();
    
    
#elif ( DIM == 3 )
    
    const unsigned fin = ((DIM==2) ? 32 : 8) * gle::finesse;
    GLfloat X = length(0);
    GLfloat Y = length(1);
    GLfloat Z = length(2);
    
    GLfloat c[2*fin+1], s[2*fin+1];
    for ( unsigned ii = 0; ii <= 2*fin; ++ii )
    {
        GLfloat ang = ii * M_PI / (GLfloat) fin;
        c[ii] = cosf(ang);
        s[ii] = sinf(ang);
    }
    
    for ( unsigned ii = 0; ii < fin; ++ii )
    {
        real uX = s[ii  ]*X, uY = s[ii  ]*Y, uZ = c[ii  ]*Z;
        real lX = s[ii+1]*X, lY = s[ii+1]*Y, lZ = c[ii+1]*Z;
        glBegin(GL_TRIANGLE_STRIP);
        for ( unsigned jj = 0; jj <= 2*fin; ++jj )
        {
            glNormal3f(c[jj]*s[ii], s[jj]*s[ii], c[ii]);
            glVertex3f(c[jj]*uX, s[jj]*uY, uZ);
            glNormal3f(c[jj]*s[ii+1], s[jj]*s[ii+1], c[ii+1]);
            glVertex3f(c[jj]*lX, s[jj]*lY, lZ);
        }
        glEnd();
    }

    glPushMatrix();
    gle::gleScale(length(0), length(1), length(2));
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

bool SpaceEllipse::display() const
{
    return false;
}


#endif
