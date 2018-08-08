// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "space_strip.h"
#include "exceptions.h"


SpaceStrip::SpaceStrip(const SpaceProp* p)
: Space(p)
{
    assert_true(p->shape=="strip");
    
    if ( DIM == 1 )
        throw InvalidParameter("strip is only valid in DIM=2 or 3");
}

void SpaceStrip::resize()
{
    checkLengths(DIM, true);
    
    for ( int d = 0; d < DIM-1; ++d )
    {
        if ( length(d) <= 0 )
            throw InvalidParameter("strip:dimension must be > 0");
    }
}


Vector SpaceStrip::extension() const
{
    return Vector(length(0), length(1), length(2));
}

//------------------------------------------------------------------------------
#pragma mark -


#if (DIM == 1)

real SpaceStrip::volume() const
{
    return length2(0);
}

bool  SpaceStrip::inside( const real point[] ) const
{
    if ( point[0] >  length(0) ) return false;
    if ( point[0] < -length(0) ) return false;
    return true;
}


void SpaceStrip::project( const real point[], real proj[] ) const
{
    if ( point[0] > 0 )
        proj[0] =  length(0);
    else
        proj[0] = -length(0);
}

#endif


//------------------------------------------------------------------------------

#if (DIM == 2)

real SpaceStrip::volume() const
{
    return length2(0) * length2(1);
}

bool  SpaceStrip::inside( const real point[] ) const
{
    if ( point[1] >  length(1) ) return false;
    if ( point[1] < -length(1) ) return false;
    return true;
}


void SpaceStrip::project( const real point[], real proj[] ) const
{
    proj[0] = point[0];
    
    if ( point[1] > 0 )
        proj[1] =  length(1);
    else
        proj[1] = -length(1);
}

#endif

//------------------------------------------------------------------------------

#if (DIM == 3)

real SpaceStrip::volume() const
{
    return length2(0) * length2(1) * length2(2);
}

bool  SpaceStrip::inside( const real point[] ) const
{
    if ( point[2] >  length(2) ) return false;
    if ( point[2] < -length(2) ) return false;
    return true;
}

void SpaceStrip::project( const real point[], real proj[] ) const
{
    proj[0] = point[0];
    proj[1] = point[1];
    
    if ( point[2] > 0 )
        proj[2] =  length(2);
    else
        proj[2] = -length(2);
}

#endif

//------------------------------------------------------------------------------
#pragma mark -

Vector SpaceStrip::period(int d)   const
{
    Vector off(0, 0, 0);
    if ( d < DIM-1 )
        off[d] = length2(d);
    return off;
}

//------------------------------------------------------------------------------

/**
 Apply periodicity, execpt in the last dimension
 */
void  SpaceStrip::fold( real point[] ) const
{
#if ( DIM > 1 )
    point[0] = remainder( point[0], length2(0) );
#endif
#if ( DIM > 2 )
    point[1] = remainder( point[1], length2(1) );
#endif
}


//------------------------------------------------------------------------------
//this makes modulo around the center o
void SpaceStrip::fold( real x[], const real o[] ) const
{
    for ( int dd = 0; dd < DIM; ++dd )
        x[dd] -= o[dd];
    
    fold(x);
    
    for ( int dd = 0; dd < DIM; ++dd )
        x[dd] += o[dd];
}

//------------------------------------------------------------------------------
//calculate both the integral part (in div) and the reminder (in x)
void SpaceStrip::foldOffset( real x[], real div[] ) const
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
#pragma mark -

#ifdef DISPLAY
#include "opengl.h"
#include "gle.h"
using namespace gle;

bool SpaceStrip::display() const
{
    const real X = length(0);
    const real Y = ( DIM > 1 ) ? length(1) : 1;
    const real Z = ( DIM > 2 ) ? length(2) : 0;
    
    glBegin(GL_LINES);
    gleVertex( -X,  Y, Z );
    gleVertex(  X,  Y, Z );
    gleVertex( -X, -Y, Z );
    gleVertex(  X, -Y, Z );
    glEnd();
    
    glLineStipple(1, 0x0303);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINES);
    gleVertex(  X,  Y, Z );
    gleVertex(  X, -Y, Z );
    gleVertex( -X,  Y, Z );
    gleVertex( -X, -Y, Z );
    glEnd();
    glDisable(GL_LINE_STIPPLE);

#if ( DIM > 2 )
    
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINES);
    gleVertex(  X,  Y, -Z );
    gleVertex(  X, -Y, -Z );
    gleVertex( -X,  Y, -Z );
    gleVertex( -X, -Y, -Z );
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    
    glBegin(GL_LINES);
    gleVertex( -X,  Y, -Z );
    gleVertex(  X,  Y, -Z );
    gleVertex( -X, -Y, -Z );
    gleVertex(  X, -Y, -Z );
    glEnd();
#endif

    return true;
}

#else

bool SpaceStrip::display() const
{
    return false;
}

#endif

