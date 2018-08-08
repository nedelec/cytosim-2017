// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#ifndef SPACE_BANANA_H
#define SPACE_BANANA_H

#include "space.h"

/// a bent cylinder of constant diameter terminated by hemispheric caps
/**
 Space `banana` is comprised from a section of a torus,
 terminated by two hemispheres. It is defined by three parameters:
 @code
    banana length width radius
 @endcode
 
 With:
 - `length` = the overall length minus 2*width
 - `width`  = the diameter of the torus in its crosssections.
 - `radius` = the main radius of the torus, which defines curvature
 .
 
 This class was first conceived by Dietrich Foethke, to simulate S. pombe.
 
 */
class SpaceBanana : public Space
{
private:
    
    /// dimensions
    real  bLength;
    real  bWidth, bWidthSqr;
    real  bRadius;

    /// angle covered by torus section
    real bAngle;
    
    /// X and Y coordinates of the right end
    real bEnd[2];

    /// coordinates of the center of the torus
    real bCenter[3];
    
    /// project on the backbone circle
    void project0(const real point[], real proj[]) const;
    
public:
        
    /// constructor
    SpaceBanana(const SpaceProp* p);
        
    /// check number and validity of specified lengths
    void        resize();

    /// maximum extension along each axis
    Vector      extension() const;
    
    /// the volume inside
    real        volume() const;
    
    /// true if the point is inside the Space
    bool        inside(const real point[]) const;
    
    /// project point on the closest edge of the Space
    void        project(const real point[], real proj[]) const;
    
    /// OpenGL display function, return true is display was done
    bool        display() const;
    
};

#endif
