// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SPACE_SPHERE_H
#define SPACE_SPHERE_H

#include "space.h"

/// sphere centered at the origin.
/**
 Space `sphere` is a sphere centered around the origin
 
 @code 
    sphere radius
 @endcode
 
With:
 - radius = radius of the sphere
 .
 
 @ingroup SpaceGroup
 */

class SpaceSphere : public Space
{
public:
    
    /// the radius of the sphere
    real        radius() const { return length(0); }
    
    /// the square of the radius
    real        radiusSqr() const { return lengthSqr(0); }
    
public:
    
    /// constructor
    SpaceSphere(const SpaceProp*);
    
    /// check number and validity of specified lengths
    void        resize() { Space::checkLengths(1, true); }

    /// maximum extension along each axis
    Vector      extension() const;
    
    /// the volume inside
    real        volume() const;
    
    /// true if the point is inside the Space
    bool        inside(const real point[]) const;
    
    /// project point on the closest edge of the Space
    void        project(const real point[], real proj[]) const;
    
    /// apply a force directed towards the edge of the Space
    void        setInteraction(Vector const& pos, PointExact const&, Meca &, real stiff) const;

    /// apply a force directed towards the edge of the Space
    void        setInteraction(Vector const& pos, PointExact const&, real rad, Meca &, real stiff) const;

    
    /// OpenGL display function, return true is display was done
    bool        display() const;
    
};

#endif

