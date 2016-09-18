// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef POINT_INTERPOLATED_H
#define POINT_INTERPOLATED_H

#include "real.h"
#include "vector.h"
#include "matrix.h"
#include "point_set.h"
#include "point_exact.h"

class FiberLocus;

/// Indicates an intermediate position between two points of a PointSet
/**
 The PointSet is designated by a pointer, and the two points by their indices.
 The interpolation coefficient is in [0,1], 
 such that the intermediate position is between Point1 and Point2:
 
 @code
 pos = (1-coef) * mPoint1 + coef * mPoint2
 @endcode
 
 @todo: PointInterpolated could be extended to refer to a Mecable, by using virtual functions
 */
class PointInterpolated
{
private:
    
    /// Mecable from which the points are interpolated 
    PointSet const* mPS;

    /// index of interpolated point 1 in mPS
    unsigned int    mPoint1;

    /// index of interpolated point 2 in mPS
    unsigned int    mPoint2;
    
    /// interpolation coefficient: pos = (1-coef) * mPoint1 + coef * mPoint2
    real            mCoef;
    
public:

    /// reset member variables
    PointInterpolated()
    {  mPS = 0;  mPoint1 = 0;  mPoint2 = 0;  mCoef = 0; }
    
    /// set to interpolate p1 and p2 on ps, with coefficient c
    PointInterpolated(const PointSet * ps, unsigned int p1, unsigned int p2, real c)
    {  mPS = ps;  mPoint1 = p1;  mPoint2 = p2;  mCoef = c; }

    /// set to interpolate given fiber segment, with abscissa \a c
    PointInterpolated(FiberLocus const&, real abs);
    
    
    /// Reset member variables (mPS=0)
    void clear()
    {  mPS = 0;  mPoint1 = 0;  mPoint2 = 0;  mCoef = 0; }
    
    /// Set to interpolate p1 and p2 on ps, with coefficient c, on the same Mecable
    void set(const unsigned int p1, const unsigned int p2, const real c)
    {  mPoint1 = p1;  mPoint2 = p2;  mCoef = c; }
    
    /// Destructor
    virtual ~PointInterpolated() {}
    
    /// Index of point 1 in the matrix of dynamics (Meca)
    Matrix::index_type matIndex1()  const { return mPS->matIndex() + mPoint1; }
    
    /// Index of point 2 in the matrix of dynamics (Meca)
    Matrix::index_type matIndex2()  const { return mPS->matIndex() + mPoint2; }
    
    
    /// Constant pointer to the PointSet
    PointSet const* object()   const { return mPS; }

    /// PointExact corresponding to first point
    PointExact      exact1()   const { return PointExact(mPS, mPoint1); }
    
    /// PointExact corresponding to second point
    PointExact      exact2()   const { return PointExact(mPS, mPoint2); }
    
    
    /// Constant pointer to the Mecable
    Mecable const*  mecable()  const { return mPS; }
    
    /// true if the pointer seems to be valid.
    bool            valid()    const { return mPS == 0 || ( mPoint1 < mPS->nbPoints() && mPoint2 < mPS->nbPoints() ); }

    /// Index of point 1 in object
    unsigned int    point1()   const { return mPoint1; }
  
    /// Index of point 2 in object
    unsigned int    point2()   const { return mPoint2; }

    /// First interpolation coefficient
    real            coef1()    const { return mCoef; }

    /// Second interpolation coefficient
    real            coef2()    const { return 1.0-mCoef; }

    /// Set interpolation coefficient
    void            coef(real c)     { mCoef = c; }
    
    /// Interpolated position in space
    Vector          pos()     const  { return mPS->interpolatePoints(mPoint1, mPoint2, mCoef); }
    
    /// position of first point
    Vector          pos1()     const { return mPS->posPoint(mPoint1); }
    
    /// position of second point 
    Vector          pos2()     const { return mPS->posPoint(mPoint2); }
    
    /// that is pos2() - pos1()
    Vector          diff()     const { return mPS->diffPoints(mPoint1, mPoint2); }
    
    /// distance between point1 and point2
    real            len()      const { return diff().norm(); }

    /// distance between point1 and point2
    real            lenSqr()   const { return diff().normSqr(); }

    /// (pos2() - pos1()).normalized()
    Vector          dir()      const { return diff().normalized(); }

    /// true if the coefficient is in [0, 1]
    bool            inside() const { return ( 0 <= mCoef ) && ( mCoef <= 1.0 ); }

    /// test if \a this has a common point with argument
    bool            overlapping(const PointExact &) const;
    
    /// test if \a this has a common point with argument
    bool            overlapping(const PointInterpolated &) const;
    
    /// print for debug purpose
    friend std::ostream & operator << (std::ostream &, PointInterpolated const&);
};

#endif
