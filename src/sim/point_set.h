// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#ifndef POINT_SET_H
#define POINT_SET_H

#include "assert_macro.h"
#include "dim.h"
#include "movable.h"
#include "object.h"
#include "mecable.h"
#include "rotation.h"

class PointInterpolated;
class Space;
class Modulo;
class Simul;

/// Array of points describing a physical object
/**
 This implements the interface defined by Mecable,
 and defines an Object with a variable number of points.
 */
class PointSet : public Mecable
{
    
private:
    
    /// Allocation size of arrays psPos[] and psFor[]
    unsigned int  psAllocated;
    
    /// Number of points in the set
    unsigned int  psSize;
    
    /// Constructor base
    void          psConstructor();

protected:
        
    /// psPos[] of size DIM*psAllocated contains DIM*psSize point-coordinates
    real*         psPos;
    
    /// psFor[] of size DIM*psAllocated contains DIM*psSize force-coordinates
    real const*   psFor;
    
    /// Center of gravity calculed in calculatePosition()
    Vector        psCenter;
    
    //--------------------------------------------------------------------------
public:
        
    /// Constructor
    PointSet();
    
    /// Copy constructor
    PointSet(const PointSet &);
    
    /// Assignement operator
    PointSet & operator =(const PointSet &);
    
    /// Destructor
    virtual ~PointSet()  { deallocatePoints(); }
    
    
    /// allocate memory to store 'nbp' points
    virtual unsigned int  allocatePoints(unsigned int nbp);
    
    /// free memory allocated by allocatePoints()
    virtual void  deallocatePoints();
    
    //--------------------------------------------------------------------------
   
    /// Number of points
    unsigned int  nbPoints()             const { return psSize; }
    
    /// Index of the last point = nbPoints - 1
    unsigned int  lastPoint()            const { return psSize - 1; }
    
    /// Number of segments = nbPoints - 1
    unsigned int  nbSegments()           const { return psSize - 1; }
    
    /// Index of the last segment = nbPoints - 2
    unsigned int  lastSegment()          const { return psSize - 2; }

    /// Position of point 'p' of the object
    Vector        posPoint(const unsigned p) const { assert_true(psPos && p<psSize); return Vector::make(psPos+DIM*p); }
    
    /// Address of point \a p
    real const*   addr()                 const { return psPos; }
    
    /// Address of point \a p
    real const*   addrPoint(const unsigned p) const { return psPos + DIM*p; }
        
    /// Set position of point p to w
    void          setPoint(unsigned p, Vector const& w);
    
    /// Set the number of points in the array
    void          nbPoints(const unsigned n)  { allocatePoints(n); psSize = n; }

    /// Add a point, returning the array index that was used
    unsigned int  addPoint(Vector const& w);
    
    /// Remove a point from the array psPos, returns number of points remaining
    void          removePoint(unsigned int p);
    
    /// Remove all points with indices [ 0, p-1 ], keep [ p, nbPoints() ]
    virtual void  truncateM(unsigned int p);
    
    /// Keep points [ 0, p ], remove other points
    virtual void  truncateP(unsigned int p);
    
    /// Remove all points
    void          clearPoints()  { psSize = 0; }
    
    /// Set all coordinates to zero (nicer for debug/testing)
    void          resetPoints();
    
    /// Add random noise uniformly to all coordinate (used for testing purposes)
    void          addNoise(real amount);
    
    /// copy current coordinates to argument
    virtual void  putPoints(real *) const;
    
    /// replace current coordinates by provided ones
    virtual void  getPoints(const real *);
    
    //--------------------------------------------------------------------------
    
    /// replace current forces by the ones provided
    virtual void  getForces(const real * ptr) { psFor = ptr; }
    
    /// Force on point p calculate by Brownian dynamics
    Vector        netForce(const unsigned p) const;
    
    //--------------------------------------------------------------------------
    //These functions are provided here for inlining, which may be faster
    
    /// Shift point at index p by vector w
    void   movePoint(const unsigned int p, Vector const& w)
    {
        assert_true( p < psSize );
        psPos[DIM*p  ] += w.XX;
#if ( DIM > 1 )
        psPos[DIM*p+1] += w.YY;
#endif
#if ( DIM > 2 )
        psPos[DIM*p+2] += w.ZZ;
#endif
    }
    
    /// Difference of two points = P+1 - P
    static Vector diffPoints(const real* src, const unsigned p)
    {
#if ( DIM == 1 )
        return Vector(src[p+1]-src[p], 0);
#elif ( DIM == 2 )
        return Vector(src[2*p+2]-src[2*p], src[2*p+3]-src[2*p+1]);
#elif ( DIM == 3 )
        return Vector(src[3*p+3]-src[3*p], src[3*p+4]-src[3*p+1], src[3*p+5]-src[3*p+2]);
#endif
    }

    /// Difference of two points = Q - P = vector PQ
    Vector diffPoints(const unsigned p) const
    {
        assert_true( p+1 < psSize );
#if ( DIM == 1 )
        return Vector(psPos[p+1]-psPos[p], 0);
#elif ( DIM == 2 )
        return Vector(psPos[2*p+2]-psPos[2*p], psPos[2*p+3]-psPos[2*p+1]);
#elif ( DIM == 3 )
        return Vector(psPos[3*p+3]-psPos[3*p], psPos[3*p+4]-psPos[3*p+1], psPos[3*p+5]-psPos[3*p+2]);
#endif
    }

    /// Difference of two points = Q - P = vector PQ
    Vector diffPoints(const unsigned int p, const unsigned int q) const
    {
        assert_true( p < psSize );
        assert_true( q < psSize );
#if ( DIM == 1 )
        return Vector(psPos[q]-psPos[p], 0);
#elif ( DIM == 2 )
        return Vector(psPos[2*q]-psPos[2*p], psPos[2*q+1]-psPos[2*p+1]);
#elif ( DIM == 3 )
        return Vector(psPos[3*q]-psPos[3*p], psPos[3*q+1]-psPos[3*p+1], psPos[3*q+2]-psPos[3*p+2]);
#endif
    }
    
    /// Calculate intermediate position = P + a ( Q - P )
    Vector interpolatePoints(const unsigned int p, const unsigned int q, const real a) const
    {
        assert_true( p < psSize );
        assert_true( q < psSize );
#if ( DIM == 1 )
        return Vector(psPos[p]+a*(psPos[q]-psPos[p]), 0);
#elif ( DIM == 2 )
        return Vector(psPos[2*p]+a*(psPos[2*q]-psPos[2*p]), psPos[2*p+1]+a*(psPos[2*q+1]-psPos[2*p+1]));
#elif ( DIM == 3 )
        return Vector(psPos[3*p]+a*(psPos[3*q]-psPos[3*p]), psPos[3*p+1]+a*(psPos[3*q+1]-psPos[3*p+1]), psPos[3*p+2]+a*(psPos[3*q+2]-psPos[3*p+2]));
#endif
    }
    
    /// calculate first momentum of point coordinates
    void          calculateMomentum(real[], bool divide = false);
    
    /// calculate first and second momentum of point coordinates
    void          calculateMomentum(real[], real[], bool divide = false);
    
    //--------------------------------------------------------------------------
    //                      Position-related functions
    //--------------------------------------------------------------------------
    
    /// Position of center of gravity
    virtual Vector position() const;
    
    /// true if object accepts translations
    virtual bool  translatable()   const { return true; }

    /// Translate object (moves all the points by w)
    virtual void  translate(Vector const&);
    
    
    /// true if object accepts translations
    virtual bool  rotatable()   const { return true; }

    /// Rotate object by given rotation
    virtual void  rotate(Rotation const&);
    
    /// Modulo around the first point
    virtual void  foldPosition(const Modulo *);
    
    //--------------------------------------------------------------------------
    
    /// Write to file
    void          write(OutputWrapper&) const;
    
    /// Read from file
    void          read(InputWrapper&, Simul&);
    
};

#endif
