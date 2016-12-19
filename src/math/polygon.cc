// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "polygon.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

/**
 Each point should be on its own line: X Y
 This will set coordinates in mPoints[] within the limit given by `alc`.
 but it will return the number of points in the stream, even if this is greater than `alc`.
 Thus two calls to this function should be enough to:
 - first determine the number of points
 - allocate the array
 - read the coordinated
 */
unsigned int Polygon::read(std::istream& in, Point2D* pts, unsigned alc)
{
    unsigned ix = 0;
    std::string line;
    real x, y;

    while ( std::getline(in, line) )
    {
        std::stringstream ls(line);
        
        ls >> x >> y;
        
        if ( ! ls.fail() )
        {
            if ( ix < alc )
            {
                pts[ix].x = x;
                pts[ix].y = y;
            }
            ++ix;
        }
    }

    return ix;
}


void Polygon::write(std::ostream& os, Point2D const* pts, unsigned npts)
{
    os.precision(6);
    for ( unsigned i = 1; i < npts; ++i )
        os << std::setw(12) << pts[i].x << "  " << std::setw(12) << pts[i].y << std::endl;
}


/**
 box[] = { xmin, xmax, ymin, ymax }
 
 result is undefined if ( npts == 0 ).
 */
void Polygon::boundingBox(Point2D const* pts, unsigned npts, real box[4])
{
    if ( npts > 0 )
    {
        box[0] = pts[0].x;
        box[1] = pts[0].x;
        box[2] = pts[0].y;
        box[3] = pts[0].y;
    }
    
    for ( unsigned i = 1; i < npts; ++i )
    {
        if ( pts[i].x < box[0] )  box[0] = pts[i].x;
        if ( pts[i].x > box[1] )  box[1] = pts[i].x;
        if ( pts[i].y < box[2] )  box[2] = pts[i].y;
        if ( pts[i].y > box[3] )  box[3] = pts[i].y;
    }
}


/**
 pre-calculate offset of successive points,
 and length of segments, used in project for efficiency.
 Also copy two points to simplify the calculations:
 - point[nbpts  ] <- point[0]
 - point[nbpts+1] <- point[1]
 .
 
 The array should be allocated to hold (npts+2) Point2D
 */
int Polygon::prepare(Point2D* pts, unsigned npts)
{
    int res = 0;
    // copy the first two points to end of array:
    // this makes some calculations more convenient
    if ( npts > 1 )
    {
        pts[npts  ].x = pts[0].x;
        pts[npts  ].y = pts[0].y;
        pts[npts+1].x = pts[1].x;
        pts[npts+1].y = pts[1].y;
    }
    else
        res = 2;
    
    for ( unsigned i = 0; i <= npts; i++ )
    {
        real dx = pts[i+1].x - pts[i].x;
        real dy = pts[i+1].y - pts[i].y;
        real d = sqrt( dx * dx + dy * dy );
        //normalize the vector:
        if ( d < REAL_EPSILON )
            res = 1;
        else
        {
            pts[i].dx = dx / d;
            pts[i].dy = dy / d;
        }
        pts[i].len = d;
    }
    return res;
}

    
/**
 calculate volume of polygon, using an algorithm that return a negative value
 for a polygon defined clockwise and a positive value for anti-clockwise.
 http://mathworld.wolfram.com/PolygonArea.html
 */
real Polygon::surface(Point2D const* pts, unsigned npts)
{
    if ( npts < 3 )
        return 0;
    
    real S = pts[npts-1].x * ( pts[0].y - pts[npts-2].y );
    for ( unsigned ii = 2; ii < npts; ++ii )
        S += pts[ii-1].x * ( pts[ii].y - pts[ii-2].y );
    
    return S / 2.0;
}


/**
 Count the number of time a ray from (xx, yy) to (infinity, yy) crosses the polygon
 The point is inside if the result is odd.
 
 @return
 0 : ouside
 1 : inside
 bound : on boundary
 .
*/
int Polygon::inside(Point2D const* pts, unsigned npts, real xx, real yy, int bound)
{
    int cross = 0;
    
    Point2D p1, p2 = pts[0];
    
    //check all edges of polygon
    for ( unsigned ii = 1; ii <= npts; ++ii )
    {
        p1 = p2;
        p2 = pts[ii];

        // check if edge cannot interesect with ray
        if (( yy <= p1.y && yy < p2.y ) || ( yy >= p1.y && yy > p2.y ))
            continue;
        
        // ray may go through p2
        if ( yy == p2.y )
        {
            // check for horizontal edge
            if ( p1.y == p2.y )
            {
                if ( xx > p1.x && xx > p2.x )
                    continue;
                if ( xx < p1.x && xx < p2.x )
                    continue;
                return bound;
            }
            
            if ( p2.x < xx )
                continue;
            
            if ( xx == p2.x )
                return bound;
            
            // next vertex
            const Point2D& p3 = pts[ii+1];
         
            // check that p2 is not a corner
            if (( p1.y < yy && yy < p3.y ) || ( p3.y < yy && yy < p1.y ))
                ++cross;
            
            continue;
        }
        
        // xx is left of edge
        if ( xx <= p1.x || xx <= p2.x )
        {
            // intersection of ray with edge
            real xi = ( yy - p1.y ) * ( p2.x - p1.x ) / ( p2.y - p1.y ) + p1.x;
            
            // overlies on an edge
            if ( fabs( xx - xi ) < __DBL_EPSILON__)
                return bound;
                
            // xx left of intersection
            if ( xx < xi )
                ++cross;
        }
    }
    
    //std::cerr << " polygon::inside " << cross << " for " << xx << " " << yy << std::endl;
    if ( cross % 2 == 0 )
        return 0;
    else
        return 1;
}


/**
 Find the closest point on the polygon to (x, y)
 
 @return
 0 : projection is on an edge
 1 : projection is a vertex
 .
 In case 1, the normal to the segment is returned in (nX, nY).
 The normal is of norm = 1.
 In case 2, the values of nX, nY are undefined.

 */
int Polygon::project(Point2D const* pts, unsigned npts, real xx, real yy,
                      real& pX, real& pY, real& nX, real& nY)
{
    int res = 0;
    
    //initialize with one point:
    pX = pts[0].x;
    pY = pts[0].y;
    
    nX = 0;
    nY = 0;

    real dis = ( xx - pX ) * ( xx - pX ) + ( yy - pY ) * ( yy - pY );
    
    for ( unsigned ii = 0; ii < npts; ++ii )
    {
        real x = xx - pts[ii].x;
        real y = yy - pts[ii].y;
        // distance to polygon point:
        real d = x * x + y * y;
        // abscissa of projection on segment [ii, ii+1] of the polygon:
        real a = pts[ii].dx * x + pts[ii].dy * y;
        
        if ( a > 0 )
        {
            if ( a < pts[ii].len )
            {
                // distance from segment to point:
                real da = d - a * a;
                
                if ( da < dis )
                {
                    dis = da;
                    pX =  pts[ii].x + a * pts[ii].dx;
                    pY =  pts[ii].y + a * pts[ii].dy;
                    nX = -pts[ii].dy;
                    nY =  pts[ii].dx;
                    res = 1;
                }
            }
        }
        else
        {
            if ( d < dis )
            {
                dis = d;
                pX = pts[ii].x;
                pY = pts[ii].y;
                res = 0;
            }
        }
    }
    
    return res;
}


