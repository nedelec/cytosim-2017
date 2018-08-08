// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SPACE_SQUARE_H
#define SPACE_SQUARE_H

#include "space.h"

///a rectangular region
/**
 Space `square` is a 2D or 3D rectangular volume.
 
 @code
    square sizeX sizeY sizeZ
 @endcode

 With:
 - sizeX = half-width along X
 - sizeY = half-width along Y
 - sizeZ = half-width along Z
 .


 @ingroup SpaceGroup
 */
class SpaceSquare : public Space
{
private:
    
    /// apply a force directed towards the edge of the Space
    static void setInteraction(const real pos[], PointExact const&, Meca &, real stiff, const real dim[]);
    
public:
    
    ///creator
    SpaceSquare(const SpaceProp*);
    
    /// check number and validity of specified lengths
    void        resize() { Space::checkLengths(DIM, true); }
    
    /// maximum extension along each axis
    Vector      extension() const;
    
    /// the volume inside
    real        volume() const;
    
    /// true if the point is inside the Space
    bool        inside(const real point[]) const;

    /// true if a sphere (center w, radius) fits in the space, edges included
    bool        allInside(const real point[], real rad) const;
    
    /// true if a sphere (center w[], radius) is entirely outside
    bool        allOutside(const real point[], real rad) const;
    
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


