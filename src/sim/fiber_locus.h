// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FIBER_LOCUS_H
#define FIBER_LOCUS_H


#include "real.h"
#include "vector.h"
#include "fiber.h"


/// Represents the segment between two consecutive points of a Fiber
/** 
 FiberLocus is used to refer to the entire segment of a Fiber.

 It is used to calculate the distance to this segment,
 or the intersection of the segment with a plane.
 
 The only mysterious thing is the name: why Locus?
 */
class FiberLocus 
{
private:
    
    /// Fiber to which the segment belongs to
    Fiber const*   mFib;
    
    /// index of segment
    unsigned int   mPoint;
    
public:
    
    /// construct without initialization
    FiberLocus() {}
    
    /// constructor
    FiberLocus(Fiber const* f, int r) : mFib(f), mPoint(r) {}

    /// the Fiber
    Fiber const* fiber()       const { return mFib; }
    
    /// index of segment
    unsigned int point()       const { return mPoint; }
    
    /// the length of the segment
    real         len()         const { return mFib->segmentation(); }
    
    /// position of first point in space
    Vector       pos1()        const { return mFib->posPoint(mPoint); }
    
    /// position of second point in space
    Vector       pos2()        const { return mFib->posPoint(mPoint+1); }
    
    /// that is pos2() - pos1()
    Vector       diff()        const { return mFib->diffPoints(mPoint); }

    /// that is ( pos2() - pos1() ).normalized()
    Vector       dir()         const { return diff() / len(); }

    /// that is [ pos2() + pos1() ] / 2
    Vector       center()      const { return 0.5 * ( pos1() + pos2() ); }
    
    /// PointExact corresponding to first point
    PointExact   exact1()      const { return PointExact(mFib, mPoint); }
    
    /// PointExact corresponding to second point
    PointExact   exact2()      const { return PointExact(mFib, mPoint+1); }

    
    /// true if the segment is the first of the Fiber
    bool         isFirst()     const { return ( mPoint == 0 ); }

    /// true if the segment is not the first of the Fiber
    bool         notFirst()    const { return ( mPoint > 0 ); }
    
    /// true if the segment is the last of the fiber
    bool         isLast()      const { return ( mPoint == mFib->nbPoints()-2 ); }
    
    /// true if the segment is not the last of the fiber
    bool         notLast()     const { return ( mPoint < mFib->nbPoints()-2 ); }

    
    /// calculate the projection of `w` on the line supporting the segment
    void         projectPoint0(Vector const& w, real& abs, real& dist) const;

    /// calculate the projection of `w` on the line supporting the segment
    void         projectPoint(Vector const& w, real& abs, real& dist) const;

    /// faster projectionPoint, but incompatible with periodic boundary conditions
    void         projectPointF(const real[], real& abs, real& dist) const;

    /// find the intersection with the plane defined by <em> n.x + a = 0 </em>
    int          intersectPlane(Vector const& n, real a, real& abs) const;

    /// calculates the closest distance between the two lines supporting the segments
    bool         projectSegment(FiberLocus const& that, real& a, real & b, real& dist) const;

};


#endif

