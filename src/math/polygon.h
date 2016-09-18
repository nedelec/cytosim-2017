// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef POLYGON_H
#define POLYGON_H

#include <fstream>

#ifndef REAL
    #include "real.h"
#endif


namespace Polygon
{
    /// Structure to hold coordinate of point in 2D + misc
    struct Point2D 
    {
        real x,  y;
        real dx, dy;
        real len;
        
        Point2D() {}
        
        Point2D(real sx, real sy)
        {
            x = sx;
            y = sy;
        }
        
        bool operator == (const Point2D& p)
        {
            return ( x == p.x  &&  y == p.y );
        }
    };
    
    /// read a polygon from file
    unsigned read(std::istream&, Point2D *pts, unsigned npts);

    /// read a polygon from file
    void     write(std::ostream&, Point2D const* pts, unsigned npts);

    /// tell if a point is inside a polygon
    int      inside(Point2D const* pts, unsigned npts, real x, real y, int bound);
    
    /// calculate the projection (pX, pY) of the point (x,y) on a polygon
    int      project(Point2D const* pts, unsigned npts, real x, real y, real& pX, real& pY, real& nX, real& nY);
    
    /// calculate the bounding box of a polygon
    void     boundingBox(Point2D const* pts, unsigned npts, real box[4]);
    
    /// calculate the surface of a polygon
    real     surface(Point2D const* pts, unsigned npts);
    
    /// calculate the offsets necessary for the other functions. Return 0 if OK
    int      prepare(Point2D* pts, unsigned npts);
 
};

#endif

