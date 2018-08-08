// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#ifndef SPACE_TORUS_H
#define SPACE_TORUS_H

#include "space.h"

///a torus of constant diameter centered on the origin
/**
 Space `torus` is defined by two parameters: 
 @code
    torus radius width
 @endcode
 
 With:
 - `radius` = the main radius of the torus
 - `width`  = the diameter of the torus in its crosssections.
  .
 
 If 'length` is not specified, the torus is endless.
 
 
 */
class SpaceTorus : public Space
{
private:
    
    /// main radius
    real  bRadius;
    
    /// thickness
    real  bWidth, bWidthSqr;
    
    /// project on the backbone
    void project0(const real point[], real proj[]) const;
    
public:
        
    /// constructor
    SpaceTorus(const SpaceProp* p);
        
    /// update following changed dimensions
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
