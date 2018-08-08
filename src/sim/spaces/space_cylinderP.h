// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SPACE_CYLINDERP_H
#define SPACE_CYLINDERP_H

#include "space.h"
#include "modulo.h"

///a cylinder of axis X that is periodic along X
/**
 Space `cylinderP' is a cylinder with periodic boundary conditions
 along the X-axis. It has no ends and loops on itself like a thorus,
 but without the curvature.

 @code
    cylinderP length radius
 @endcode

 With:
 - length = half-length of the cylinder along X
 - radius = radius of the cylinder
 .
 

 @ingroup SpaceGroup
 */
class SpaceCylinderP : public Space, public Modulo
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
    SpaceCylinderP(const SpaceProp*);
    
    /// check number and validity of specified lengths
    void        resize() { Space::checkLengths(2, true); }
    
    /// true if the Space is periodic in dimension ii
    bool isPeriodic(int ii) const { return ( ii == 0 ); }
        
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
    
    /// the i-th direction of periodicity
    Vector      period(int d) const;
    
    /// set x to its periodic representation closest to origin by removing periodic repeats
    void        fold(real x[]) const;
    
    /// remove from x (1st arg) the periodic repeats, around given reference
    void        fold(real x[], const real ref[]) const;
    
    /// calculate the periodic offset between x and y
    void        foldOffset(real x[], real off[]) const;
    
    /// apply a force directed towards the edge of the Space
    void        setInteraction(Vector const& pos, PointExact const&, Meca &, real stiff) const;
    
    /// apply a force directed towards the edge of the Space
    void        setInteraction(Vector const& pos, PointExact const&, real rad, Meca &, real stiff) const;

    
    /// OpenGL display function, return true is display was done
    bool        display() const;
    
};

#endif

