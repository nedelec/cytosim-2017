// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "space_periodic.h"
#include "exceptions.h"



SpacePeriodic::SpacePeriodic(const SpaceProp* p)
: Space(p)
{
    assert_true(p->shape=="periodic");
}

void SpacePeriodic::resize()
{
    checkLengths(DIM, true);
    
    for ( int d = 0; d < DIM; ++d )
    {
        if ( length(d) <= 0 )
            throw InvalidParameter("periodic:dimension must be > 0");
    }
}


Vector SpacePeriodic::extension() const
{
    return Vector(length(0), length(1), length(2));
}


//------------------------------------------------------------------------------
#pragma mark -

#if (DIM == 1)

real SpacePeriodic::volume() const
{
    return length2(0);
}

bool  SpacePeriodic::inside( const real point[] ) const
{
    return true;
}


void SpacePeriodic::project( const real point[], real proj[] ) const
{
    throw InvalidParameter("A periodic space has no edge!");
}

#endif


//------------------------------------------------------------------------------

#if (DIM == 2)

real SpacePeriodic::volume() const
{
    return length2(0) * length2(1);
}

bool  SpacePeriodic::inside( const real point[] ) const
{
    return true;
}


void SpacePeriodic::project( const real point[], real proj[] ) const
{
    throw InvalidParameter("A periodic space has no edge!");
}

#endif

//------------------------------------------------------------------------------

#if (DIM == 3)

real SpacePeriodic::volume() const
{
    return length2(0) * length2(1) * length2(2);
}

bool  SpacePeriodic::inside( const real point[] ) const
{
    return true;
}

void SpacePeriodic::project( const real point[], real proj[] ) const
{
    throw InvalidParameter("A periodic space has no edge!");
}

#endif


//------------------------------------------------------------------------------
#pragma mark -

Vector SpacePeriodic::period(int d)   const
{
    Vector off(0, 0, 0);
    if ( d < DIM )
        off[d] = length2(d);
    return off;
}


//------------------------------------------------------------------------------
void  SpacePeriodic::fold( real point[] ) const
{
    //periodic in all dimensions
    point[0] = remainder( point[0], length2(0) );
#if ( DIM > 1 )
    point[1] = remainder( point[1], length2(1) );
#endif
#if ( DIM > 2 )
    point[2] = remainder( point[2], length2(2) );
#endif
}


//------------------------------------------------------------------------------
void SpacePeriodic::fold( real x[], const real o[] ) const
{
    for ( int dd = 0; dd < DIM; ++dd )
        x[dd] -= o[dd];
    
    fold(x);
    
    for ( int dd = 0; dd < DIM; ++dd )
        x[dd] += o[dd];
}

//------------------------------------------------------------------------------
void SpacePeriodic::foldOffset( real x[], real div[] ) const
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

bool SpacePeriodic::display() const
{
    const real X = length(0);
    const real Y = ( DIM > 1 ) ? length(1) : 1;
    const real Z = ( DIM > 2 ) ? length(2) : 0;
    
    glLineStipple(1, 0x0303);
    glEnable(GL_LINE_STIPPLE);

#if ( DIM == 1 )
    glBegin(GL_LINES);
    gleVertex(  X, -Y, 0 );
    gleVertex(  X,  Y, 0 );
    gleVertex( -X,  Y, 0 );
    gleVertex( -X, -Y, 0 );
    glEnd();    
    return true;
#endif
    
    glBegin(GL_LINE_LOOP);
    gleVertex(  X,  Y, Z );
    gleVertex(  X, -Y, Z );
    gleVertex( -X, -Y, Z );
    gleVertex( -X,  Y, Z );
    glEnd();
    
#if ( DIM > 2 )
    glBegin(GL_LINE_LOOP);
    gleVertex(  X,  Y, -Z );
    gleVertex(  X, -Y, -Z );
    gleVertex( -X, -Y, -Z );
    gleVertex( -X,  Y, -Z );
    glEnd();
#endif

    glDisable(GL_LINE_STIPPLE);
    return true;
}

#else

bool SpacePeriodic::display() const
{
    return false;
}

#endif

