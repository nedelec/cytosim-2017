// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SPACE_POLYGON_H
#define SPACE_POLYGON_H

#include "space.h"
#include "polygon.h"

/// a polygonal convex region in space
/**
 Space `polygon` implements a polygon. It works best for convex polygon.
 In 3D, and additional HEIGHT can be specified to describe a generalized 
 cylinder of axis Z, that has the 2D polygon as cross-section.
 
 The coordinates of the polygon are read from a file.

 @code
    polygon file_name HEIGHT
 @endcode

 @ingroup SpaceGroup
*/
class SpacePolygon : public Space
{
private:
    
    ///number of points defining the polygon. Must be > 2.
    unsigned          nPoints;
    
    ///pointer to the points defining the polygon in 2D
    Polygon::Point2D *mPoints;
        
    ///pre-calculated bounding box since this is called often
    Vector            boundingBox;
    
    /// Volume calculated from polygon
    real              mVolume;
    
    /// half the total height (alias to mLength[0])
    real &            height;


public:
        
    ///creator
    SpacePolygon(const SpaceProp *, std::string const& file);
    
    ///destructor
    ~SpacePolygon();
    
    /// maximum extension along each axis
    Vector      extension() const { return boundingBox; }
    
    /// the volume inside
    real        volume() const { return mVolume; }
    
    /// true if the point is inside the Space
    bool        inside(const real point[]) const;
    
    /// project point on the closest edge of the Space
    void        project(const real point[], real proj[]) const;

    /// apply a force directed towards the edge of the Space
    void        setInteraction(Vector const& pos, PointExact const&, Meca &, real stiff) const;
    
    /// apply a force directed towards the edge of the Space
    void        setInteraction(Vector const& pos, PointExact const&, real rad, Meca &, real stiff) const;
    
    /// update since length have changed
    void        resize();
    
    /// OpenGL display function, return true is display was done
    bool        display() const;
    
};

#endif

