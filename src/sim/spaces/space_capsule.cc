// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "space_capsule.h"
#include "point_exact.h"
#include "exceptions.h"
#include "smath.h"
#include "meca.h"

inline real sqr(const real x) { return x*x; }


SpaceCapsule::SpaceCapsule(const SpaceProp* p)
: Space(p)
{
    if ( DIM == 1 )
        throw InvalidParameter("capsule is only defined for DIM = 2 or 3");
}

//------------------------------------------------------------------------------

Vector SpaceCapsule::extension() const
{
    return Vector(radius()+length(), radius(), radius());
}


real SpaceCapsule::volume() const
{
#if (DIM == 3)
    return ( length() + (2/3.0) * radius() ) * radiusSqr() * ( 2 * M_PI );
#else
    return 4 * length() * radius() + M_PI * radiusSqr();
#endif
}


//------------------------------------------------------------------------------
bool SpaceCapsule::inside( const real w[] ) const
{
    real nrm, x = fabs( w[0] );
    
    if ( x > length() )
        nrm = sqr( x - length() );
    else
        nrm = 0;
    
#if ( DIM == 2 )
    nrm += w[1] * w[1];
#else
    nrm += w[1] * w[1] + w[2] * w[2];
#endif
    
    return ( nrm <= radiusSqr() );
}


bool SpaceCapsule::allInside( const real w[], const real rad ) const
{
    real nrm, x = fabs( w[0] );
    
    if ( x > length() )
        nrm = sqr( x - length() );
    else
        nrm = 0;
    
#if ( DIM == 2 )
    nrm += w[1] * w[1];
#else
    nrm += w[1] * w[1] + w[2] * w[2];
#endif
    
    return ( nrm <= sqr(radius()-rad) );
}

//------------------------------------------------------------------------------
void SpaceCapsule::project( const real w[], real p[] ) const
{
    real nrm;
#if ( DIM == 2 )
    nrm = w[1] * w[1];
#else
    nrm = w[1] * w[1] + w[2] * w[2];
#endif
    
    //calculate the projection on the axis, within boundaries:
    if ( w[0] >  length() )
    {
        nrm  += sqr( w[0] - length() );
        //normalize from this point on the axis
        if ( nrm > 0 ) nrm = radius() / sqrt( nrm );
        
        p[0] = length() + nrm * ( w[0] - length() );
    }
    else
    {
        if ( w[0] < -length() )
        {
            nrm  += sqr( length() + w[0] );
            //normalize from this point on the axis
            if ( nrm > 0 ) nrm = radius() / sqrt( nrm );
            
            p[0]  = -length() + nrm * ( w[0] + length() );
        }
        else
        {
            //normalize from this point on the axis
            if ( nrm > 0 ) nrm = radius() / sqrt( nrm );
            
            p[0] = w[0];
        }
    }
    
    if ( nrm > 0 )
    {
        p[1] = nrm * w[1];
#if ( DIM == 3 )
        p[2] = nrm * w[2];
#endif
    }
    else
    {
        //we project on a arbitrary point on the cylinder
        p[1] = radius();
#if ( DIM == 3 )
        p[2] = 0;
#endif
    }
}


//------------------------------------------------------------------------------

/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCapsule::setInteraction(Vector const& pos, PointExact const& pe, Meca & meca, real stiff, const real len, const real rad)
{
    if ( pos.XX > len )
        meca.interLongClamp( pe, Vector( len,0,0), rad, stiff );
    else if ( pos.XX < -len )
        meca.interLongClamp( pe, Vector(-len,0,0), rad, stiff );
    else
    {
        const Matrix::index_type inx = DIM * pe.matIndex();
        
#if ( DIM == 2 )
        if ( pos.YY > 0 )
        {
            meca.mC(inx+1, inx+1) -= stiff;
            meca.base(inx+1)      += stiff * rad;
        }
        else
        {
            meca.mC(inx+1, inx+1) -= stiff;
            meca.base(inx+1)      -= stiff * rad;
        }
#elif ( DIM == 3 )
        Vector axis(0, pos.YY, pos.ZZ);
        real axis_n = axis.norm();
        axis /= axis_n;
        
        if ( rad < axis_n )
        {
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
            meca.mC(inx+1, inx+1) -= stiff * axis[1] * axis[1];
            meca.mC(inx+1, inx+2) -= stiff * axis[1] * axis[2];
            meca.mC(inx+2, inx+2) -= stiff * axis[2] * axis[2];
            
            real facX = stiff * rad;
            meca.base(inx+1) += facX * axis[1];
            meca.base(inx+2) += facX * axis[2];
        }
#endif
    }
}


/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCapsule::setInteraction(Vector const& pos, PointExact const& pe, Meca & meca, real stiff) const
{
    setInteraction(pos, pe, meca, stiff, length(), radius());
}

/**
 This applies the correct forces in the cylindrical and spherical parts.
 */
void SpaceCapsule::setInteraction(Vector const& pos, PointExact const& pe, real rad, Meca & meca, real stiff) const
{
    if ( rad < radius() )
        setInteraction(pos, pe, meca, stiff, length(), radius()-rad);
    else
        setInteraction(pos, pe, meca, stiff, length(), 0);
}


//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY
#include "opengl.h"
#include "gle.h"

bool SpaceCapsule::display() const
{
    //number of sections in the quarter-circle
    const int  fin = (DIM==2) ? 256 : 32;
    
    GLfloat c[4*fin+1], s[4*fin+1];
    for ( int ii = 0; ii <= 4*fin; ++ii )
    {
        GLfloat ang = ii * 0.5 * M_PI / (GLfloat) fin;
        c[ii] = cosf(ang);
        s[ii] = sinf(ang);
    }
    
    GLfloat L = length();
    GLfloat R = radius();
    
#if ( DIM <= 2 )
    
    //display a loop in X/Y plane
    glBegin(GL_LINE_LOOP);
    for ( int ii = 0;     ii <= 2*fin; ++ii )
        glVertex2f( +L+R*s[ii], R*c[ii] );
    for ( int ii = 2*fin; ii <= 4*fin; ++ii )
        glVertex2f( -L+R*s[ii], R*c[ii] );
    glEnd();
    
#else
    
    //display strips along the side of the volume:
    for ( int sc = 0; sc < 4*fin; ++sc )
    {
        //compute the transverse angles:
        GLfloat ctb  = c[sc  ],   stb  = s[sc  ];
        GLfloat cta  = c[sc+1],   sta  = s[sc+1];
        GLfloat ctbR = R*ctb,     stbR = R*stb;
        GLfloat ctaR = R*cta,     staR = R*sta;
        
        //draw one strip of the oval:
        glBegin(GL_TRIANGLE_STRIP);
        for ( int ii=0; ii <= fin; ++ii )
        {
            GLfloat ca = c[ii], sa = s[ii];
            glNormal3f( ca, cta*sa, sta*sa );
            glVertex3f( +L+R*ca, ctaR*sa, staR*sa );
            glNormal3f( ca, ctb*sa, stb*sa );
            glVertex3f( +L+R*ca, ctbR*sa, stbR*sa );
        }
        for ( int ii=fin; ii >= 0; --ii)
        {
            GLfloat ca = -c[ii], sa = s[ii];
            glNormal3f( ca, cta*sa, sta*sa );
            glVertex3f( -L+R*ca, ctaR*sa, staR*sa );
            glNormal3f( ca, ctb*sa, stb*sa );
            glVertex3f( -L+R*ca, ctbR*sa, stbR*sa );
        }
        glEnd();
    }
    
    if ( 1 )
    {
        //draw 2 rings on the surface
        GLfloat width = 0.01 * R;
        glPushMatrix();
        gle::gleTranslate(L, 0, 0);
        gle::gleScale(R, R, R);
        glRotated(90, 0, 1, 0);
        gle::gleArrowedBand(width);
        gle::gleTranslate(0, 0, -2*L/R);
        glRotated(180, 0, 1, 0);
        gle::gleArrowedBand(width);
        glPopMatrix();
    }

#endif
    return true;
}

#else

bool SpaceCapsule::display() const
{
    return false;
}

#endif
