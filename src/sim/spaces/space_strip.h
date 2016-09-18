// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SPACE_STRIP_H
#define SPACE_STRIP_H

#include "space.h"
#include "modulo.h"

///a rectangular Space with partial periodic boundary conditions
/**
 Space `periodic` implements periodic boundary condition in all but the last dimension.
 The volume only has edge in the last dimension, and otherwise wraps on itself.
 The last dimension is Y in 2D and Z in 3D.
 
 @code
    strip sizeX sizeY sizeZ
 @endcode
 
 With:
 - sizeX = half-width along X
 - sizeY = half-width along Y
 - sizeZ = half-width along Z
 .
 
 */
class SpaceStrip : public Space, public Modulo
{
public:
    
    /// creator
    SpaceStrip(const SpaceProp*);

    /// check number and validity of specified lengths
    void        resize();

    /// true if the Space is periodic in dimension ii
    bool       isPeriodic(int ii) const { return ( ii < DIM-1 ); }

    /// maximum extension along each axis
    Vector     extension()        const;
    
    /// the volume inside
    real       volume()           const;
    
    /// true if the point is inside the Space
    bool       inside(const real point[]) const;
    
    /// project point on the closest edge of the Space
    void       project(const real point[], real proj[]) const;
    
    
    /// the i-th direction of peripodicity
    Vector     period(int d)  const;

    /// set x to its periodic representation closest to origin by removing periodic repeats
    void       fold(real x[]) const;
    
    /// remove from x (1st arg) the periodic repeats, around given reference
    void       fold(real x[], const real ref[]) const;
    
    /// calculate the periodic offset between x and y
    void       foldOffset(real x[], real off[]) const;
    
    
    /// OpenGL display function, return true is display was done
    bool       display() const;
    
};

#endif

