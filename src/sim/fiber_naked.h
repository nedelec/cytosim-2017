// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FIBER_NAKED_H
#define FIBER_NAKED_H


#include "sim.h"
#include "vector.h"
#include "common.h"
#include "point_set.h"
#include "point_interpolated.h"

class PointExact;


/// Mecable with linear geometry
/**
 This PointSet describes a thin flexible Fiber that is longitudinally incompressible.
 The length of the FiberNaked can be changed by growP(), growM(), cutP() and cutM().
 
 \par Number of points:
 
 The best number of points to describe a FiberNaked is automatically calculated:
 It is the integer \a nb_points that minimizes:
 @code
 fabs( length() / nb_points - FiberProp::segmentation )
 @endcode
 
 where segmentation is a parameter of the fiber class.
 All the segments in a fiber all have the same length
 @code
 FiberNaked::segmentation() = length() / nb_points
 @endcode

 Note that FiberNaked::segmentation() is not always equal to FiberProp::segmentation.
 If the fibers have various length, their segmentation() will be different,
 even though they all share the same value of FiberProp::segmentation.

 See related functions: length(), nbPoints() and segmentation().
 
 \par Longitudinal incompressibility:
 
 Successive model-points are kept at a constant distance via constrained dynamics:
 @code
 ( posPoint(N+1)-posPoint(N) ).norm() == FiberNaked::segmentation()
 @endcode
 
 \par Origin:
 
 An abscissa is a curvilinear distance taken along the Fiber,
 and the FiberNaked provides an origin to make this independent of the model-points. 
 Thus even if the fiber lengthen from its ends, a position described by an abscissa will
 stay associated with the same local lattice site.
 
 Functions are provided in FiberNaked to convert abscissa measured from different references,
 and to obtain positions of the fiber for a given abcissa.

 \par Derived classes:
 
 The class FiberBinder keeps track of its position using an abscissa from the origin,
 and all Hand objects are built from this class.
 The class Fiber keeps track of the FiberBinder that are attached to itself.
*/
class FiberNaked : public PointSet
{
    /// the ideal number of points for ratio = length / segmentation
    static unsigned int  bestNbPoints(real ratio);

private:
        
    /// actual section length: distance between consecutive points
    real         fnCut;
    
    /// target (ideal) segmentation length
    real         fnCutWished;
      
    /// abscissa of the minus-end (zero at creation)
    real         fnAbscissa;
        
    /// seed used to simulate speckles in the display
    unsigned int fnSignature;
    
    /// oldest method to restore the distance between successive model-points
    static void  reshape_sure(unsigned, real*, real cut);
    
    /// restore the distance between two points
    static void  reshape_two(const real*, real*, real cut);
   
    /// iterative method to restore the distance between successive model-points
    static int   reshape_it(unsigned, const real*, real*, real cut);

protected:
    
    /// callback for function that elongates or shorten the Fiber
    virtual void updateRange() = 0;
    
public:
        
    /// Constructor
    FiberNaked();
    
    /// Destructor
    ~FiberNaked() {}

    //---------------------

    /// set position of MINUS_END and direction (length and Nb of points are not modified)
    /** dir does not need to be normalized */
    void         setStraight(Vector const& pos, Vector const& dir);

    /// set position of 'ref' and direction of Fiber
    void         setStraight(Vector const& pos, Vector const& dir, FiberEnd ref);

    /// set position of 'ref', direction and length of Fiber
    void         setStraight(Vector const& pos, Vector const& dir, real len, FiberEnd ref);
    
    /// import shape from the given array of size DIM*n_pts, and create a shape with \a np points
    void         setShape(const real pts[], unsigned n_pts, unsigned np);

    //---------------------
    
    /// exact representation of given end
    PointExact         exactEnd(FiberEnd) const;

    /// interpolation representing a given end (CENTER is also a valid argument)
    PointInterpolated  interpolateEnd(FiberEnd) const;
    
    /// interpolation representing a given end
    PointInterpolated  interpolateCenter() const;

    /// interpolation representing the MINUS_END
    PointInterpolated  interpolateMinusEnd() const { return PointInterpolated(this, 0, 1, 0); }

    /// interpolation representing the PLUS_END
    PointInterpolated  interpolatePlusEnd() const { return PointInterpolated(this, nbPoints()-2, nbPoints()-1, 1); }

    /// interpolation of the site specified by its distance from the ORIGIN
    PointInterpolated  interpolate(real ab) const;
    
    /// interpolation of the site specified from the MINUS_END
    PointInterpolated  interpolateM(real ab) const;
    
    /// interpolation of a site specified by its distance from a FiberEnd
    PointInterpolated  interpolate(real ab, FiberEnd from) const;
    
    //---------------------
    
    /// signed distance from ORIGIN to MINUS_END (abscissa of MINUS_END)
    real         abscissaM()    const { return fnAbscissa; }
    
    /// signed distance from ORIGIN to PLUS_END (abscissa of PLUS_END)
    real         abscissaP()    const { return fnAbscissa + fnCut * nbSegments(); }
    
    /// signed distance from ORIGIN to given point or intermediate position
    real         abscissaP(const real n) const { return fnAbscissa + fnCut * n; }

    /// signed distance from the ORIGIN to the specified FiberEnd
    real         abscissa(FiberEnd end) const;
    
    /// converts abscissa from the specified FiberEnd, to abscissa from the ORIGIN
    real         abscissa(real ab, FiberEnd from) const;
    
    //---------------------
    
    /// the total length of the Fiber
    real         length()    const { return nbSegments() * fnCut; }
        
    /// true if ( abscissaM() < ab < abscissaP() )
    bool         within(const real ab)  const { return abscissaM() <= ab && ab <= abscissaP(); }
    
    /// true if abscissa is below abscissaP
    bool         withinP(const real ab) const { return ab <= abscissaP(); }
    
    /// true if abscissa is above abscissaM
    bool         withinM(const real ab) const { return abscissaM() <= ab; }

    /// a random number which is used in speckle display 
    unsigned int signature() const { return fnSignature; }
    
    /// set signature
    void         signature(unsigned int s) { fnSignature = s; }
    
    /// calculate the domain in which ab is located (near a FiberEnd, or central)
    FiberEnd     whichEndDomain(real ab, real lambda) const;
    
    //---------------------
    
    /// position of a point specified by abscissa from the origin
    Vector       pos(real ab) const;

    /// position of a point specified by abscissa from the MINUS_END
    Vector       posM(real ab) const;

    /// position of a point specified by abscissa on the Fiber
    Vector       pos(real ab, FiberEnd from) const;

    /// position of a FiberEnd (CENTER is also a valid argument)
    Vector       posEnd(FiberEnd which) const;
    
    //---------------------
    
#if ( 1 )
    /// normalized tangent vector to the fiber at given model point
    /** We divide by fnCut, which should be the distance between points */
    Vector       dirPoint(unsigned p) const { return diffPoints(p) / fnCut; }
#else
    /** We normalize the difference between points */    
    Vector       dirPoint(unsigned p) const { return diffPoints(p).normalized(); }
#endif

        
    /// normalized tangent vector to the fiber at given abscissa from the origin
    Vector       dir(real ab) const;

    /// normalized tangent vector to the fiber at given abscissa from given reference
    Vector       dir(real ab, FiberEnd from) const;
    
    /// normalized tangent vector to the fiber at given end
    Vector       dirEnd(FiberEnd which) const;
    
    /// dot-product (force/km on the end of the Fiber).(direction of Fiber growth)
    real         projectedForceOnEnd(FiberEnd which) const;

    //--------------------- Segmentation / discrete representation
    
    /// set desired segmentation (the actual segmentation might be different)
    void         segmentation(real c) { assert_true(c>0); fnCutWished = c; }
    
    /// the current segment length (distance between successive model-points)
    real         segmentation() const { return fnCut; }
    
    /// returns segmentation() ^ 3
    real         segmentationCub() const { return fnCut*fnCut*fnCut; }
    
    /// recalculate fiber to have \a nb_points model points
    void         resegment(unsigned int nb_points);
    
    /// set the number of points to minimize abs( fnCut - fnCutWished )
    void         adjustSegmentation();
    
    /// re-establish the distance between successive model-points
    void         reshape();
    
    /// set position
    void         getPoints(const real * x);

    //--------------------- Info
    
    /// calculate the minimum and maximum segment length
    void         minMaxSegments(real&, real&) const;

    /// calculate average and variance of the segment length
    void         infoSegments(real&, real&) const;

    /// the cosine of the maximum segment angle: indicate the errors due to curvature
    real         minCosinus() const;
    
    /// number of joints at which ( cosine(angle) < threshold )
    unsigned     nbKinks(real threshold = 0) const;

    //--------------------- Growing/Shrinking
    
    /// merge two fibers by joining the PLUS_END of \a fib with the MINUS_END of \a this
    void         join(FiberNaked const* fib);

    /// increase/decrease length of Fiber by \a dlen, at the MINUS_END
    void         growM(real dlen);
    
    /// add a segment of length segmentation() at the MINUS_END
    void         addSegmentM();
    
    /// remove a piece of length \a dlen >= 0 at the MINUS_END
    void         cutM(real dlen);
    
    /// increase/decrease length of Fiber by \a dlen, at the PLUS_END
    void         growP(real dlen);
    
    /// add a segment of length segmentation() at the MINUS_END
    void         addSegmentP();
    
    /// remove a piece of length \a dlen >= 0 at the PLUS_END
    void         cutP(real dlen);
    
    /// adjust length without changing the position of \a ref
    void         setLength(real len, FiberEnd ref);

    /// Remove all model points in [ 0, P-1 ], keep [ P, nbPoints() ]
    void         truncateM(unsigned int p);

    /// Keep model points [ 0, p ], remove other points
    void         truncateP(unsigned int p);

    //---------------------
    
    /// write to OutputWrapper
    void         write(OutputWrapper&) const;
    
    /// read from InputWrapper
    void         read(InputWrapper&, Simul&);
    
};


#endif
