// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SPACE_CYLINDERZ_H
#define SPACE_CYLINDERZ_H

#include "space.h"

///a cylinder of axis Z
/**
 Space `cylinderZ' is radial symmetric along the Z-axis.
 The crosssection in the XY plane is a disc.

 @code
    cylinderZ length radius
 @endcode
 
 With:
 - length = half-length of cylinder in Z
 - radius = radius of cylinder
 .

 @ingroup SpaceGroup
 */
class SpaceCylinderZ : public Space
{    
    /// apply a force directed towards the edge of the Space
    static void setInteraction(Vector const& pos, PointExact const&, Meca &, real stiff, real len, real rad);

private:
    
    /// half the length of the central cylinder
    real        length() const { return Space::length(0); }
    
    /// the radius of the hemisphere
    real        radius() const { return Space::length(1); }
    
    /// the square of the radius
    real        radiusSqr() const { return Space::lengthSqr(1); }
    
public:
        
    ///creator
    SpaceCylinderZ(const SpaceProp*);
    
    /// check number and validity of specified lengths
    void        resize() { Space::checkLengths(2, true); }
    
    /// maximum extension along each axis
    Vector      extension() const;
    
    /// the volume inside
    real        volume() const;
    
    /// true if the point is inside the Space
    bool        inside(const real point[]) const;
    
    /// true if the bead is inside the Space
    bool        allInside(const real point[], real rad) const;
    
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

