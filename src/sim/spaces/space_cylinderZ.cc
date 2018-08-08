// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "space_cylinderZ.h"
#include "point_exact.h"
#include "exceptions.h"
#include "smath.h"
#include "meca.h"


SpaceCylinderZ::SpaceCylinderZ(const SpaceProp* p)
: Space(p)
{
    if ( DIM != 3 )
        throw InvalidParameter("cylinderZ is only valid in 3D: use sphere instead");
}

//------------------------------------------------------------------------------

Vector SpaceCylinderZ::extension() const
{
    return Vector(radius(), radius(), length()); 
}


real  SpaceCylinderZ::volume() const
{
    return 2 * M_PI * length() * radius() * radius();
}

//------------------------------------------------------------------------------
bool  SpaceCylinderZ::inside( const real w[] ) const
{
    if ( fabs(w[2]) > length() )
        return false;
    return ( w[0]*w[0] + w[1]*w[1] <= radiusSqr() );
}

bool  SpaceCylinderZ::allInside( const real w[], const real rad ) const
{
    if ( fabs(w[2]) > length()-rad )
        return false;
    return ( sqrt( w[0]*w[0]+ w[1]*w[1] ) + rad <= radius() );
}


//------------------------------------------------------------------------------
void SpaceCylinderZ::project( const real w[], real p[] ) const
{
    int inZ = 1;
    
    p[0] = w[0];
    p[1] = w[1];
    p[2] = w[2];
    
    if ( w[2] >  length() )
    {
        p[2] =  length();
        inZ = 0;
    }
    else if ( w[2] < -length() )
    {
        p[2] = -length();
        inZ = 0;
    }
    
    real n = sqrt( w[0]*w[0]+ w[1]*w[1] );
    
    if ( n > radius() )
    {
        n = radius() / n;
        p[0] = n * w[0];
        p[1] = n * w[1];            
    }
    else
    {
        if ( inZ )
        {
            if ( length() - fabs(w[2]) < radius() - n )
            {
                if ( w[2] > 0 )
                    p[2] =  length();
                else
                    p[2] = -length();
            }
            else
            {
                n = radius() / n;
                p[0] = n * w[0];
                p[1] = n * w[1];
            }
        }
    }
}

//------------------------------------------------------------------------------

/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCylinderZ::setInteraction(Vector const& pos, PointExact const& pe, Meca & meca, real stiff, const real len, const real rad)
{
#if ( DIM == 3 )
    const Matrix::index_type inx = DIM * pe.matIndex();

    
    if ( pos.ZZ > len )
    {
        meca.mC(inx+2, inx+2) -= stiff;
        meca.base(inx+2)      += stiff * len;
    }
    else if ( pos.ZZ < -len )
    {
        meca.mC(inx+2, inx+2) -= stiff;
        meca.base(inx+2)      -= stiff * len;
    }
    
    Vector axis(pos.XX, pos.YY, 0);
    real axis_n = axis.norm();
    axis /= axis_n;
    
    if ( rad < axis_n )
    {
        // outside cylinder radius
        real len = rad / axis_n;
        
        meca.mC(inx  , inx  ) += stiff * ( len * ( 1.0 - axis[0] * axis[0] ) - 1.0 );
        meca.mC(inx  , inx+1) -= stiff * len * axis[0] * axis[1];
        meca.mC(inx+1, inx+1) += stiff * ( len * ( 1.0 - axis[1] * axis[1] ) - 1.0 );
        
        real facX = stiff * len * axis_n;            
        meca.base(inx  ) += facX * axis[0];
        meca.base(inx+1) += facX * axis[1];
    }
    else
    {
        // inside cylinder radius
        real p, d;
        if ( pos.ZZ > 0 )
        {
            p = len;
            d = len - pos.ZZ;
        }
        else
        {
            p = -len;
            d = len + pos.ZZ;
        }
        
        if ( d > rad - axis_n )
        {
            meca.mC(inx  , inx  ) -= stiff * axis[0] * axis[0];
            meca.mC(inx  , inx+1) -= stiff * axis[0] * axis[1];
            meca.mC(inx+1, inx+1) -= stiff * axis[1] * axis[1];
            
            real facX = stiff * rad;
            meca.base(inx  ) += facX * axis[0];
            meca.base(inx+1) += facX * axis[1];
        }
        else
        {
            meca.mC(inx+2, inx+2) -= stiff;
            meca.base(inx+2)      += stiff * p;
        }
    }
#endif
}


/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCylinderZ::setInteraction(Vector const& pos, PointExact const& pe, Meca & meca, real stiff) const
{
    setInteraction(pos, pe, meca, stiff, length(), radius());
}

/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCylinderZ::setInteraction(Vector const& pos, PointExact const& pe, real rad, Meca & meca, real stiff) const
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

bool SpaceCylinderZ::display() const
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
    //display strips along the side of the volume:
    for ( int sc = 0; sc <= fin; ++sc )
    {
        GLfloat ca = c[sc], sa = s[sc];
        glNormal3f( ca, sa, 0 );
        glVertex3f( R*ca, R*sa, +L );
        glVertex3f( R*ca, R*sa, -L );
    }
    glEnd();
    
    // draw the cap:
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, +1);
    glVertex3f(0, 0, +L);
    for ( int sc = 0; sc <= fin; ++sc )
        glVertex3f(R*c[sc], R*s[sc],  +L);
    glEnd();
    
    // draw the cap:
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, -1);
    glVertex3f(0, 0, -L);
    for ( int sc = 0; sc <= fin; ++sc )
        glVertex3f(-R*c[sc], R*s[sc], -L);
    glEnd();
    
#endif
    return true;
}

#else

bool SpaceCylinderZ::display() const
{
    return false;
}

#endif
