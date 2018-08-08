// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "space_square.h"
#include "exceptions.h"
#include "point_exact.h"
#include "meca.h"


SpaceSquare::SpaceSquare(const SpaceProp* p)
: Space(p)
{
}


Vector SpaceSquare::extension() const
{
    return Vector(length(0), length(1), length(2));
}


//------------------------------------------------------------------------------
#pragma mark -
#pragma mark DIM=1


#if (DIM == 1)

real SpaceSquare::volume() const
{
    return 2 * length(0);
}

bool  SpaceSquare::inside( const real w[] ) const
{
    return ( w[0] >= -length(0)  &&  w[0] <=  length(0) );
}

bool  SpaceSquare::allInside( const real w[], const real rad ) const
{
    return ( w[0] >= rad-length(0)  &&  w[0] <=  length(0)-rad );
}

bool  SpaceSquare::allOutside( const real w[], const real rad ) const
{
    return ( w[0] < -rad-length(0)  ||  w[0] >  length(0)+rad );
}

void SpaceSquare::project( const real w[], real p[] ) const
{
    if ( w[0] > 0 )
        p[0] =  length(0);
    else
        p[0] = -length(0);
}

#endif


//------------------------------------------------------------------------------
#pragma mark -
#pragma mark DIM=2


#if (DIM == 2)

real SpaceSquare::volume() const
{
    return 4 * length(0) * length(1);
}


bool  SpaceSquare::inside( const real w[] ) const
{
    return (w[0] >= -length(0)  &&  w[0] <=  length(0) &&  
            w[1] >= -length(1)  &&  w[1] <=  length(1) );
}

bool  SpaceSquare::allInside( const real w[], const real rad ) const
{
    return (w[0] >= rad-length(0)  &&  w[0] <=  length(0)-rad  &&  
            w[1] >= rad-length(1)  &&  w[1] <=  length(1)-rad );
}

bool  SpaceSquare::allOutside( const real w[], const real rad ) const
{
    return (w[0] < -rad-length(0)  ||  w[0] > length(0)+rad  ||  
            w[1] < -rad-length(1)  ||  w[1] > length(1)+rad );
}


void SpaceSquare::project( const real w[], real p[] ) const
{
    bool in = true;
    
    for ( int d = 0; d < 2; ++d )
    {
        p[d] = w[d];
        
        if ( p[d] >  length(d) )
        {
            p[d] =  length(d);
            in = false;
        }
        else if ( p[d] < -length(d) )
        {
            p[d] = -length(d);
            in = false;
        }
    }
    
    if ( in )
    {
        int dip = ( length(1) - fabs(w[1]) ) < ( length(0) - fabs(w[0]) );
        
        if ( w[dip] > 0 )
            p[dip] =  length(dip);
        else
            p[dip] = -length(dip);
    }
}


#endif

//------------------------------------------------------------------------------
#pragma mark -
#pragma mark DIM=3


#if (DIM == 3)

real SpaceSquare::volume() const
{
    return 8 * length(0) * length(1) * length(2);
}



bool  SpaceSquare::inside( const real w[] ) const
{
    return (w[0] >= -length(0)  &&  w[0] <=  length(0)  &&
            w[1] >= -length(1)  &&  w[1] <=  length(1)  && 
            w[2] >= -length(2)  &&  w[2] <=  length(2)  );
}

bool  SpaceSquare::allInside( const real w[], const real rad ) const
{
    return (w[0] >= rad-length(0)  &&  w[0] <=  length(0)-rad  &&
            w[1] >= rad-length(1)  &&  w[1] <=  length(1)-rad  && 
            w[2] >= rad-length(2)  &&  w[2] <=  length(2)-rad  );
}

bool  SpaceSquare::allOutside( const real w[], const real rad ) const
{
    return (w[0] < -rad-length(0)  ||  w[0] > length(0)+rad ||  
            w[1] < -rad-length(1)  ||  w[1] > length(1)+rad ||
            w[2] < -rad-length(2)  ||  w[2] > length(2)+rad );
}


void SpaceSquare::project( const real w[], real p[] ) const
{
    bool in = true;
    
    for ( int d = 0; d < 3; ++d )
    {
        p[d] = w[d];
        if ( p[d] >  length(d) )
        {
            p[d] =  length(d);
            in = false;
        }
        else
        {
            if ( p[d] < -length(d) )
            {
                p[d] = -length(d);
                in = false;
            }
        }
    }
    
    if ( in )
    {
        // find the dimensionality 'dip' corresponding to the closest face
        int dip = 0;
        
        real l = ( w[0] > 0 ) ? length(0) - w[0] : length(0) + w[0];
        real u = ( w[1] > 0 ) ? length(1) - w[1] : length(1) + w[1];
        
        if ( u < l ) { dip = 1; l = u; };
        
        u = ( w[2] > 0 ) ? length(2) - w[2] : length(2) + w[2];
       
        if ( u < l )   dip = 2;
        
        if ( w[dip] > 0 )
            p[dip] =  length(dip);
        else
            p[dip] = -length(dip);
    }
}

#endif

//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Interaction

/// apply a force directed towards the edge of the box
/**
 When the point is in the center of the box.
 
 When a point is along the edge of the cube, the interaction
 is flat in one direction, and curved in the two others.

 */

void SpaceSquare::setInteraction(const real pos[], PointExact const& pe, Meca & meca, real stiff, const real dim[])
{
    bool in = true;
    
    Matrix::index_type inx = DIM * pe.matIndex();
    
    for ( int d = 0; d < DIM; ++d )
    {
        assert_true( dim[d] >= 0 );
        if ( pos[d] > dim[d] )
        {
            meca.mC(inx+d, inx+d) -= stiff;
            meca.base(inx+d)      += stiff * dim[d];
            in = false;
        }
        else if ( pos[d] < -dim[d] )
        {
            meca.mC(inx+d, inx+d) -= stiff;
            meca.base(inx+d)      -= stiff * dim[d];
            in = false;
        }
    }

    if ( in ) 
    {
        // find the dimensionality 'dip' corresponding to the closest face
        int  dip = 0;
        
        real l = ( pos[0] > 0 ) ? dim[0] - pos[0] : dim[0] + pos[0];
#if ( DIM > 1 )
        real u = ( pos[1] > 0 ) ? dim[1] - pos[1] : dim[1] + pos[1];
        if ( u < l ) { dip = 1; l = u; };
#endif
#if ( DIM > 2 )
        u = ( pos[2] > 0 ) ? dim[2] - pos[2] : dim[2] + pos[2];
        if ( u < l )  dip = 2;
#endif
        meca.mC(inx+dip, inx+dip) -= stiff;
        meca.base(inx+dip) += stiff * ( pos[dip] > 0 ? dim[dip] : -dim[dip]);
    }
}


void SpaceSquare::setInteraction(Vector const& pos, PointExact const& pe, Meca & meca, real stiff) const
{
    setInteraction(pos, pe, meca, stiff, mLength);
}


void SpaceSquare::setInteraction(Vector const& pos, PointExact const& pe, real rad, Meca & meca, real stiff) const
{
    real dim[DIM];
    for ( int d = 0; d < DIM; ++d )
    {
        dim[d] = length(d) - rad;
        if ( dim[d] < 0 )
            dim[d] = 0;
    }

    setInteraction(pos, pe, meca, stiff, dim);
}

//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY
#include "opengl.h"
#include "gle.h"
using namespace gle;

bool SpaceSquare::display() const
{
    const real X = length(0);
    const real Y = ( DIM > 1 ) ? length(1) : 1;
    const real Z = ( DIM > 2 ) ? length(2) : 0;
    
    
#if ( DIM == 3 )

    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X,  Y, -Z );
    gleVertex(  X,  Y,  Z );
    gleVertex(  X, -Y, -Z );
    gleVertex(  X, -Y,  Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex( -X, -Y, -Z );
    gleVertex( -X, -Y,  Z );
    gleVertex( -X,  Y, -Z );
    gleVertex( -X,  Y,  Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X,  Y, -Z );
    gleVertex( -X,  Y, -Z );
    gleVertex(  X,  Y,  Z );
    gleVertex( -X,  Y,  Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X, -Y,  Z );
    gleVertex( -X, -Y,  Z );
    gleVertex(  X, -Y, -Z );
    gleVertex( -X, -Y, -Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X,  Y,  Z );
    gleVertex( -X,  Y,  Z );
    gleVertex(  X, -Y,  Z );
    gleVertex( -X, -Y,  Z );
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex(  X,  Y, -Z );
    gleVertex(  X, -Y, -Z );
    gleVertex( -X,  Y, -Z );
    gleVertex( -X, -Y, -Z );
    glEnd();

    glBegin(GL_LINE_LOOP);
    gleVertex(  X,  Y, Z );
    gleVertex(  X, -Y, Z );
    gleVertex( -X, -Y, Z );
    gleVertex( -X,  Y, Z );
    glEnd();
    
    glBegin(GL_LINES);
    gleVertex(  X,  Y, -Z );
    gleVertex(  X,  Y,  Z );
    gleVertex(  X, -Y, -Z );
    gleVertex(  X, -Y,  Z );
    gleVertex( -X, -Y, -Z );
    gleVertex( -X, -Y,  Z );
    gleVertex( -X,  Y, -Z );
    gleVertex( -X,  Y,  Z );
    glEnd();

#endif
  
    glBegin(GL_LINE_LOOP);
    gleVertex(  X,  Y, -Z );
    gleVertex(  X, -Y, -Z );
    gleVertex( -X, -Y, -Z );
    gleVertex( -X,  Y, -Z );
    glEnd();

    return true;
}

#else

bool SpaceSquare::display() const
{
    return false;
}


#endif


