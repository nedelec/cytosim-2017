// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "space_cylinder.h"
#include "point_exact.h"
#include "exceptions.h"
#include "smath.h"
#include "meca.h"


SpaceCylinder::SpaceCylinder(const SpaceProp* p)
: Space(p)
{
    if ( DIM != 3 )
        throw InvalidParameter("cylinder is only valid in 3D: use rectangle instead");
}

//------------------------------------------------------------------------------
Vector SpaceCylinder::extension() const
{
    return Vector(length(), radius(), radius());
}


real  SpaceCylinder::volume() const
{
    return 2 * M_PI * length() * radius() * radius();
}

//------------------------------------------------------------------------------
bool  SpaceCylinder::inside( const real w[] ) const
{
    if ( fabs(w[0]) > length() )
        return false;
    return ( w[1]*w[1]+ w[2]*w[2] <= radiusSqr() );
}

bool  SpaceCylinder::allInside( const real w[], const real rad ) const
{
    if ( fabs(w[0]) > length()-rad )
        return false;
    return ( sqrt( w[1]*w[1]+ w[2]*w[2] ) + rad <= radius() );
}

//------------------------------------------------------------------------------
void SpaceCylinder::project( const real w[], real p[] ) const
{    
    bool inX = 1;
    
    p[0] = w[0];
    p[1] = w[1];
    p[2] = w[2];
    
    if ( w[0] >  length() )
    {
        p[0] =  length();
        inX = 0;
    }
    else if ( w[0] < -length() )
    {
        p[0] = -length();
        inX = 0;
    }
    
    real n = sqrt( w[1]*w[1]+ w[2]*w[2] );
    
    if ( n > radius() )
    {
        n = radius() / n;
        p[1] = n * w[1];
        p[2] = n * w[2];            
    }
    else
    {
        if ( inX )
        {
            if ( length() - fabs(w[0]) < radius() - n )
            {
                if ( w[0] > 0 )
                    p[0] =  length();
                else
                    p[0] = -length();
            }
            else
            {
                n = radius() / n;
                p[1] = n * w[1];
                p[2] = n * w[2];
            }
        }
    }
}

//------------------------------------------------------------------------------

/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCylinder::setInteraction(Vector const& pos, PointExact const& pe, Meca & meca, real stiff, const real len, const real rad)
{
    const Matrix::index_type inx = DIM * pe.matIndex();

    if ( pos.XX > len )
    {
        meca.mC(inx, inx) -= stiff;
        meca.base(inx)    += stiff * len;
    }
    else if ( pos.XX < -len )
    {
        meca.mC(inx, inx) -= stiff;
        meca.base(inx)    -= stiff * len;
    }
    
#if ( DIM == 3 )
    
    Vector axis(0, pos.YY, pos.ZZ);
    real axis_n = axis.norm();
    axis /= axis_n;
    
    if ( rad < axis_n )
    {
        // outside cylinder radius
        real len = rad / axis_n;
        
        meca.mC(inx+1, inx+1) += stiff * ( len * ( 1.0 - axis[1] * axis[1] ) - 1.0 );
        meca.mC(inx+1, inx+2) -= stiff * len * axis[1] * axis[2];
        meca.mC(inx+2, inx+2) += stiff * ( len * ( 1.0 - axis[2] * axis[2] ) - 1.0 );
        
        real facX = stiff * len * axis_n;            
        meca.base(inx+1) += facX * axis[1];
        meca.base(inx+2) += facX * axis[2];
    }
    else
    {
        // inside cylinder radius
        real p, d;
        if ( pos.XX > 0 )
        {
            p = len;
            d = len - pos.XX;
        }
        else
        {
            p = -len;
            d = len + pos.XX;
        }
        
        if ( d > rad - axis_n )
        {
            meca.mC(inx+1, inx+1) -= stiff * axis[1] * axis[1];
            meca.mC(inx+1, inx+2) -= stiff * axis[1] * axis[2];
            meca.mC(inx+2, inx+2) -= stiff * axis[2] * axis[2];
            
            real facX = stiff * rad;
            meca.base(inx+1) += facX * axis[1];
            meca.base(inx+2) += facX * axis[2];
        }
        else
        {
            meca.mC(inx, inx) -= stiff;
            meca.base(inx)    += stiff * p;
        }
    }
    
#endif
}


/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCylinder::setInteraction(Vector const& pos, PointExact const& pe, Meca & meca, real stiff) const
{
    setInteraction(pos, pe, meca, stiff, length(), radius());
}

/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCylinder::setInteraction(Vector const& pos, PointExact const& pe, real rad, Meca & meca, real stiff) const
{
    real eRadius = radius() - rad;
    if ( eRadius < 0 ) eRadius = 0;
    real eLength = length() - rad;
    if ( eLength < 0 ) eLength = 0;
    
    setInteraction(pos, pe, meca, stiff, eLength, eRadius);
}

//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY
#include "opengl.h"

bool SpaceCylinder::display() const
{
#if ( DIM == 3 )

    const int  fin = 512;
    GLfloat L = length();
    GLfloat R = radius();
    
    GLfloat c[fin+1], s[fin+1];
    for ( int ii = 0; ii <= fin; ++ii )
    {
        GLfloat ang = ii * 2 * M_PI / (GLfloat) fin;
        c[ii] = cosf(ang);
        s[ii] = sinf(ang);
    }
    
    glBegin(GL_TRIANGLE_STRIP);
    for ( int sc = 0; sc <= fin; ++sc )
    {
        GLfloat ca = c[sc], sa = s[sc];
        glNormal3f( 0, ca, sa );
        glVertex3f( +L, R*ca, R*sa );
        glVertex3f( -L, R*ca, R*sa );
    }
    glEnd();
    
    // draw the cap:
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f( +1, 0, 0 );
    glVertex3f( +L, 0, 0 );
    for ( int sc = 0; sc <= fin; ++sc )
        glVertex3f( +L, R*c[sc], R*s[sc] );
    glEnd();
    
    // draw the cap:
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f( -1, 0, 0 );
    glVertex3f( -L, 0, 0 );
    for ( int sc = 0; sc <= fin; ++sc )
        glVertex3f( -L,-R*c[sc], R*s[sc] );
    glEnd();
    
#endif
    return true;
}

#else

bool SpaceCylinder::display() const
{
    return false;
}

#endif

