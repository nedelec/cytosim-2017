// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "space_cylinderP.h"
#include "point_exact.h"
#include "exceptions.h"
#include "smath.h"
#include "meca.h"


SpaceCylinderP::SpaceCylinderP(const SpaceProp* p)
: Space(p)
{
    if ( DIM != 3 )
        throw InvalidParameter("cylinderP is only valid in 3D: use strip instead");
}

//------------------------------------------------------------------------------
Vector SpaceCylinderP::extension() const
{
    return Vector(length(), radius(), radius());
}


real  SpaceCylinderP::volume() const
{
    return 2 * M_PI * length() * radius() * radius();
}

//------------------------------------------------------------------------------
bool  SpaceCylinderP::inside( const real w[] ) const
{
    return ( w[1]*w[1]+ w[2]*w[2] <= radiusSqr() );
}

bool  SpaceCylinderP::allInside( const real w[], const real rad ) const
{
    return ( sqrt( w[1]*w[1]+ w[2]*w[2] ) + rad <= radius() );
}

//------------------------------------------------------------------------------
void SpaceCylinderP::project( const real w[], real p[] ) const
{    
    
    p[0] = w[0];
    p[1] = w[1];
    p[2] = w[2];
    
    real n = sqrt( w[1]*w[1]+ w[2]*w[2] );
    n = radius() / n;
    p[1] = n * w[1];
    p[2] = n * w[2]; 
    
}

//------------------------------------------------------------------------------

/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCylinderP::setInteraction(Vector const& pos, PointExact const& pe, Meca & meca, real stiff, const real len, const real rad)
{

#if ( DIM == 3 )

    const Matrix::index_type inx = DIM * pe.matIndex();
    
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
void SpaceCylinderP::setInteraction(Vector const& pos, PointExact const& pe, Meca & meca, real stiff) const
{
    setInteraction(pos, pe, meca, stiff, length(), radius());
}

/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCylinderP::setInteraction(Vector const& pos, PointExact const& pe, real rad, Meca & meca, real stiff) const
{
    real eRadius = radius() - rad;
    if ( eRadius < 0 ) eRadius = 0;
    real eLength = length() - rad;
    if ( eLength < 0 ) eLength = 0;
    
    setInteraction(pos, pe, meca, stiff, eLength, eRadius);
}

//------------------------------------------------------------------------------
#pragma mark -

Vector SpaceCylinderP::period(int d)   const
{
    Vector off(0, 0, 0);
    if ( d == 0  )
        off[d] = length2(d);
    return off;
}


//------------------------------------------------------------------------------

/**
 Apply periodicity, only in first dimension
 */
void  SpaceCylinderP::fold( real point[] ) const
{

    point[0] = remainder( point[0], length2(0) );
}


//------------------------------------------------------------------------------
//this makes modulo around the center o
void SpaceCylinderP::fold( real x[], const real o[] ) const
{
    for ( int dd = 0; dd < DIM; ++dd )
        x[dd] -= o[dd];
    
    fold(x);
    
    for ( int dd = 0; dd < DIM; ++dd )
        x[dd] += o[dd];
}


//------------------------------------------------------------------------------
//calculate both the integral part (in div) and the reminder (in x)
void SpaceCylinderP::foldOffset( real x[], real div[] ) const
{
    for ( int dd = 0; dd < DIM; ++dd )
        div[dd] = x[dd];
    
    fold(x);
    
    for ( int dd = 0; dd < DIM; ++dd )
        div[dd] -= x[dd];
}

//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY
#include "opengl.h"

bool SpaceCylinderP::display() const
{
#if ( DIM == 3 )

    const int  fin = 512;
    
    GLfloat L = length();
    GLfloat R = radius();
    
    glBegin(GL_TRIANGLE_STRIP);
    for ( int ii = 0; ii <= fin; ++ii )
    {
        GLfloat ang = ii * 2 * M_PI / (GLfloat) fin;
        GLfloat ca = cosf(ang), sa = sinf(ang);
        glNormal3f( 0, ca, sa );
        glVertex3f( +L, R*ca, R*sa );
        glVertex3f( -L, R*ca, R*sa );
    }
    glEnd();
    
#endif
    return true;
}

#else

bool SpaceCylinderP::display() const
{
    return false;
}

#endif

