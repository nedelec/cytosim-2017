// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SPACE_PERIODIC_H
#define SPACE_PERIODIC_H

#include "space.h"
#include "modulo.h"

/// a rectangular Space with periodic boundary conditions
/**
 Space `periodic` implements periodic boundary condition in all dimensions.
 The volume has no edge and wraps on itself.
 
 @code
    periodic sizeX sizeY sizeZ
 @endcode
 
 With:
 - sizeX = half-width along X
 - sizeY = half-width along Y
 - sizeZ = half-width along Z
 .
 
 */
class SpacePeriodic : public Space, public Modulo
{
public:
    
    /// creator
    SpacePeriodic(const SpaceProp*);

    /// check number and validity of specified lengths
    void       resize();

    /// true if the Space is periodic in dimension ii
    bool       isPeriodic(int ii) const { return true; }

    /// maximum extension along each axis
    Vector     extension()        const;
    
    /// the volume inside
    real       volume()           const;
    
    /// true if the point is inside the Space
    bool       inside(const real point[]) const;
    
    /// project point on the closest edge of the Space
    void       project(const real point[], real proj[]) const;
        
    
    /// the d-th direction of periodicity
    Vector     period(int d)   const;
    
    /// set vector to its periodic representation closest to origin
    void       fold(real[])    const;
    
    /// remove periodic repeats in pos[], to bring it closest to ref[]
    void       fold(real pos[], const real ref[]) const;
    
    /// bring ref[] closest to origin, returning translation done in trans[]
    void       foldOffset(real pos[], real trans[])  const;
    
    
    /// OpenGL display function, return true is display was done
    bool       display() const;
    
};

#endif

