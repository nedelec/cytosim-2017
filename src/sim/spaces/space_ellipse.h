// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SPACE_ELLIPSE_H
#define SPACE_ELLIPSE_H

#include "dim.h"
#include "space.h"

/// ellipse in 2D, ellipsoid or spheroid in 3D 
/**
 The ellipse/ellipsoid is aligned with the principal axes X, Y and Z.
 
 @code
    ellipse sizeX sizeY sizeZ
 @endcode 

 With:
 - sizeX = half length of X axis
 - sizeY = half length of Y axis
 - sizeZ = half length of Z axis
 .
 
 The projection of one point on the surface of the ellipse is done numerically. 
 In 3D this is the only solution if the 3 axes have different length.
 setInteraction() relies on project() and thus uses the tangent plane at the
 projection point to approximate the confinement force.
 */

class SpaceEllipse : public Space
{
public:
        
    /// creator
    SpaceEllipse(const SpaceProp*);
        
    /// check number and validity of specified lengths
    void        resize();
    
    /// maximum extension along each axis
    Vector      extension() const;
    
    /// direct normal direction calculation
    Vector      normalToEdge(const real point[]) const;
    
    /// the volume inside
    real        volume() const;
    
    /// true if the point is inside the Space
    bool        inside(const real point[]) const;
    
    /// project point on the closest edge of the Space
    void        project1D(const real point[], real proj[]) const;
    
    /// project point on the closest edge of the Space
    void        project2D(const real point[], real proj[]) const;
    
    /// project point on the closest edge of the Space
    void        project3D(const real point[], real proj[]) const;
    
    /// project point on the closest edge of the Space
    void        project(const real point[], real proj[]) const
    {
#if ( DIM == 1 )
        project1D( point, proj );
#elif ( DIM == 2 )
        project2D( point, proj );
#else
        project3D( point, proj );
#endif
    }
    
    
    /// openGL display function, return true is display was done
    bool       display() const;
    
};

#endif

