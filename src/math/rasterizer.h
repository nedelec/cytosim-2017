// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef RASTERIZER_H
#define RASTERIZER_H

#include "real.h"


/// 2D and 3D rasterizer
/**
 The different functions in the rasterizer call a given method func() for every point 
 of INTEGER coordinates inside a certain volume.
 
 The volume can be specified in two ways:
 - as a polygon described by a list of points, using paintPolygon?D(polygon) 
 - as a cylinder specified by two points P,Q and a scalar 'radius'.
 In 3D, the functions do not rasterize a cylinder, but rather a rectangular volume
 that contains all the points located at distance radius or less from [PQ].

 Important note:   The points defining the polygon do not need to be integers.

 F.Nedelec, EMBL 2002-2012, nedelec@embl.de

 ///\todo implement a rasterizer in variable precision, keeping arguments as 'real'
*/
namespace Rasterizer 
{
    
    /// a point in 3D, with information for connectivity
    struct Vertex
    {
        /// coordinates of the point
        real x, y, z;
        
        /// bit-field used to describe the connectivity between the points.
        /**
         Two points A and B are connected if ( A.u & B.u ) using the bit-wise AND.
         With a long integer, this limits the number of edges to 64.
         A bigger integer could be used if needed.
        */
        unsigned long u;
    };
    
    //-------------------------- Rasterizer functions in 1D -------------------------
    
    /// Rasterizer function in 1D:
    void paintFatLine1D(void (*paint)(int, int, int, int, void*, void*),
                        void * arg1, void * arg2,
                        const real p[],       ///< first end of the segment [dim=1]
                        const real q[],       ///< second end of the segment [dim=1]
                        const real radius,    ///< width by which the segment [pq] is inflated
                        const real offset[],  ///< phase of the grid [dim=1]
                        const real delta[]    ///< period for the grid [dim=1]
                        );
    
    
    //-------------------------- Rasterizer functions in 2D -------------------------

    /// Calculate the convex hull from a set of 2D-points
    /** 
     - input:   xy[] contains the coordinates of 'nbpts' points: x,y,x,y...
     - output:  xy[] is the anti-clockwise convex hull, starting from the bottom most point

     \return The number of points in the convex hull.
     */
    unsigned int convexHull2D(unsigned int nbpts, ///< number of points
                              real xy[]           ///< coordinates of the points
                              );
    
    
    /// Paint a polygon in 2D
    /**
     paintPolygon2D() calls paintPoint(x,y,zz) for every point (x,y) of
     integral coordinates, which are inside the polygon given in xy[]. 
     The polygon should be convex, and ordered anti-clockwise.
     */  
    void paintPolygon2D(void (*paint)(int, int, int, int, void*, void*),
                        void * arg1, void * arg2,
                        unsigned int nbpts,   ///< number of points
                        const real xy[],      ///< coordinates of the points ( x y, x y...)
                        const int zz = 0      ///< third coordinate, passed as argument to paint()
                        );
    
    
    /// Paint the inside of a rectangle with edges parallel to the segment [pq]
    void paintFatLine2D(void (*paint)(int, int, int, int, void*, void*),
                        void * arg1, void * arg2,
                        const real p[],       ///< first end of the segment [dim=2]
                        const real q[],       ///< second end of the segment [dim=2]
                        const real radius     ///< width by which [pq] is inflated
                        );
    
    
    /// Paint the inside of a rectangle with edges parallel to the segment [pq]
    void paintFatLine2D(void (*paint)(int, int, int, int, void*, void*),
                        void * arg1, void * arg2,
                        const real p[],       ///< first end of the segment [dim=2]
                        const real q[],       ///< second end of the segment [dim=2]
                        const real width,     ///< width by which the line [pq] is extended, to make a round cylinder
                        const real offset[],  ///< phase of the grid [dim=2]
                        const real delta[],   ///< period for the grid [dim=2]
                        real lengthPQ = 0     ///< length of segment PQ, or zero if unknown    
                        );
    
    /// Paint a 2D rectangular volume with edges parallel to the main axes
    /**
     The painted volume is square and aligned with the principal axes (X, Y, Z)
     It contains all the points at a distance 'radius' or less from the segment [p,q].
     This is the fastest rasterizer, but the volume can be much greater than that of the cylinder.
     However, the volume is nearly optimal if PQ is aligned with one of the main axis, 
     and paintBox3D is then the best choice.
     */
    void paintBox2D(void (*paint)(int, int, int, int, void*, void*),
                    void * arg1, void * arg2,
                    const real p[],       ///< first end of the segment [dim=3]
                    const real q[],       ///< second end of the segment [dim=3]
                    const real radius,    ///< radius of cylinder contained in the volume
                    const real offset[],  ///< phase of the grid [dim=3]
                    const real delta[]    ///< period for the grid [dim=3]
                    );
    
    //-------------------------- 2D-functions useful for 3D -------------------------

    /// Find convex hull from a set of 2D-points given in an array with leading-dimension 4
    /**
     - input:  xy[] contains the coordinates of 'nbpts' points: x,y,a,b,x,y...
               - For each point xy[] has 4 slots (X, Y, A, B)
               - A, B are not used, but are moved together with X,Y as the array is reordered.
     - output: xy[] is the anti-clockwise convex hull, starting from the bottom most point.
     
     \return The number of points in the convex hull.
     */
    unsigned int convexHull2D_4(unsigned int nbpts, ///< number of points
                                real xy[]           ///< coordinates of the points
                                );
    
    
    /// Polygon rasterizer function in 2D, when leading-dimension of xy[] is 4
    void paintPolygon2D_4(void (*paint)(int, int, int, int, void*, void*),
                          void * arg1, void * arg2,
                          unsigned int nbpts,   ///< number of points
                          const real xy[],      ///< coordinates of the points ( x y, *, *, x y...)
                          const int zz = 0      ///< third coordinate, passed as argument to paint()
                          );
    
    
    //-------------------------- Rasterizer functions in 3D -------------------------
    
    /// Paint a polygon in 3D
    /** 
     Rasterize the convex hull of the 'nbpts' points given in xyz[].
     Simplistic algorithm: for each section at integral Z, the intersection of
     all possible lines connecting any two points are calculated, and the 
     convex-hull of all this points is given to paintPolygon2D.
     */
    void paintPolygon3D(void (*paint)(int, int, int, int, void*, void*),
                        void * arg1, void * arg2,
                        unsigned int nbpts,       ///< number of points
                        real xyz[]                ///< coordinates
                        );
    
    
    /// old Rasterizer function in 3D (slower)
    void paintFatLine3D_old(void (*paint)(int, int, int, int, void*, void*),
                            void * arg1, void * arg2,
                            const real p[],       ///< first end of the segment [dim=3]
                            const real q[],       ///< second end of the segment [dim=3]
                            const real radius,    ///< radius of cylinder contained in the volume
                            const real offset[],  ///< phase of the grid [dim=3]
                            const real delta[]    ///< period for the grid [dim=3]
                            );
    
    
    /// Paint a 3D polygon for which the edges of the convex hull are known
    /**
     The polygon is the convex hull of the 'nbpts' vertices given in pts[].
     Each Vertex contains coordinates and information on the connectivity to other points.
     The connections between Vertices are the edge of the 3D polygon.
     */
    void paintPolygon3D(void (*paint)(int, int, int, int, void*, void*),
                        void * arg1, void * arg2,
                        unsigned int  nbpts,  ///< number of points
                        Vertex pts[]          ///< coordinates + connectivity
                        );
    
    
    /// Paint a 3D cylinder with square section, aligned with the segment [P,Q]
    /**
     A volume is painted around the segment [p,q], containing the cylinder of
     all the points located at a distance 'radius' or less from [p,q].
     The volume is a right cylinder with a square section.
     */
    void paintFatLine3D(void (*paint)(int, int, int, int, void*, void*),
                        void * arg1, void * arg2,
                        const real p[],       ///< first end of the segment [dim=3]
                        const real q[],       ///< second end of the segment [dim=3]
                        const real radius,    ///< radius of cylinder contained in the volume
                        const real offset[],  ///< phase of the grid [dim=3]
                        const real delta[],   ///< period for the grid [dim=3]
                        real lengthPQ = 0     ///< length of segment PQ if known    
                        );
    
    
    /// Paint a 3D cylinder with hexagonal section, aligned with the segment [P,Q]
    /**
     A volume is painted around points [p,q], which contains the cylinder of
     all the points at a distance 'radius' or less from the segment [p,q].
     The volume is a right cylinder with hexagonal section.
     This is a tighter approximation of the cylinder than the square cylinder of paintFatLine3D.
     */
    void paintHexLine3D(void (*paint)(int, int, int, int, void*, void*),
                        void * arg1, void * arg2,
                        const real p[],       ///< first end of the segment [dim=3]
                        const real q[],       ///< second end of the segment [dim=3]
                        const real radius,    ///< radius of cylinder contained in the volume
                        const real offset[],  ///< phase of the grid [dim=3]
                        const real delta[],   ///< period for the grid [dim=3]
                        real lengthPQ = 0     ///< length of segment PQ if known    
                        );

    
    /// Paint a 3D rectangular volume with edges parallel to the main axes
    /**
     The painted volume is square and its edges are parallel to the principal axes (X, Y, Z)
     It contains all the points at a distance 'radius' or less from the segment [p,q].
     This is the fastest rasterizer, but the volume can be much greater than that of the cylinder,
     in particular in the case where PQ >> radius, and PQ is oriented along a diagonal.
     However, the volume is nearly optimal if PQ is almost aligned with one of the main axis, 
     and paintBox3D is then a better choice than the rasterizers that paint a cylinder,
     because it is much faster.
     */
    void paintBox3D(void (*paint)(int, int, int, int, void*, void*),
                    void * arg1, void * arg2,
                    const real p[],       ///< first end of the segment [dim=3]
                    const real q[],       ///< second end of the segment [dim=3]
                    const real radius,    ///< radius of cylinder contained in the volume
                    const real offset[],  ///< phase of the grid [dim=3]
                    const real delta[]    ///< period for the grid [dim=3]
                    );
    
};

#endif

