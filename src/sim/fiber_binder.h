// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FIBER_BINDER_H
#define FIBER_BINDER_H

#include "assert_macro.h"
#include "point_interpolated.h"
#include "fiber.h"
#include "sim.h"

class FiberLocus;

/// a location on a Fiber represented by an abscissa
/**
 A FiberBinder has a pointer to a Fiber, which is:
 - zero if the Binder is not attached,
 - the corresponding Fiber when the Binder is attached.
 .
 
 When the FiberBinder is attached, its location on the Fiber
 is stored as a curvilinear abscissa (fbAbs) taken along the fiber, 
 from a fixed reference on the fiber.
 The abscissa is independent from the Fiber's model points.
*/
class FiberBinder: public Node
{
private:
    
    /// the corresponding interpolation, which is kept up-to-date
    PointInterpolated inter;
    
protected:
    
    ///the Fiber on which it is attached, or 0 if not attached
    /* it should always be equal to PointInterpolated::object() */
    Fiber*     fbFiber;
    
    ///the abscissa from the Fiber origin
    real       fbAbs;
    
public:
    
    /// construct as unattached
    FiberBinder() : fbFiber(0), fbAbs(0) {}
    
    /// construct at the given distance from the origin
    FiberBinder(Fiber* f, real a);

    //--------------------------------------------------------------------------
    
    /// move to a different fiber, at same abscissa
    void relocate(Fiber* f);
    
    /// move to a different fiber, at given position
    void relocate(Fiber* f, real a);
    
    /// move to the specified FiberEnd (CENTER is also a valid argument)
    void moveToEnd(FiberEnd end);
    
    /// move along the Fiber to specified abscissa, or detach
    void moveTo(real abs);
    
    /// move along the Fiber by the abscissa offset dabs, or detach
    void moveBy(real dabs);

    //--------------------------------------------------------------------------
    
    /// true if attached
    bool         attached()    const { return fbFiber != 0; }
    
    /// Fiber to which this is attached, or zero if not attached
    Fiber*       fiber()       const { return fbFiber; }
    
    /// position in space
    Vector       pos()         const { return inter.pos(); }
    
    /// direction of Fiber obtained by normalization
    Vector       dir()         const { return inter.dir(); }
    
    /// the direction of the Fiber at the point of attachment
    Vector       dirFiber()    const { return fbFiber->dirPoint(inter.point1()); }
    
    /// the abscissa, from the origin of the Fiber
    real         abscissa()    const { return fbAbs; }

    /// abscissa, counted from the specified FiberEnd (in reversed direction for the PLUS_END)
    real         abscissaFrom(FiberEnd from) const;
            
    ///nearest end to the point specified by the abscissa from the origin
    FiberEnd     nearestEnd() const;
    
    /// true if abscissa is below abscissaP
    bool         withinP()     const { return fbFiber->withinP(fbAbs); }
    
    /// true if abscissa is above abscissaM
    bool         withinM()     const { return fbFiber->withinM(fbAbs); }
    
    /// true if abscissa is within the fiber boundaries
    bool         within()      const { return fbFiber->within(fbAbs); }

    //--------------------------------------------------------------------------
    
    /// the interpolation
    const PointInterpolated& interpolation() const { assert_false(bad()); return inter; }
    
    /// set a valid PointInterpolated
    void         updateBinder()                    { inter = fbFiber->interpolate(fbAbs); }
  
    /// attach at position `fb`
    virtual void attach(FiberBinder const& fb);
    
    /// detach from Fiber (can be changed in derived Classes to allow updating)
    virtual void detach();

    /// check the abscissa against the edges of the fiber, calling handleOutOfRange() if necessary.
    void         checkFiberRange();

    /// called when the Hand abscissa lies outside the range of the Fiber
    virtual void handleOutOfRange(FiberEnd) { detach(); }
    
    //--------------------------------------------------------------------------
    
    /// a static_cast<> of Node::next()
    FiberBinder *  next()  const  { return static_cast<FiberBinder*>(nNext); }
    
    /// a static_cast<> of Node::prev()
    FiberBinder *  prev()  const  { return static_cast<FiberBinder*>(nPrev); }
    
    //--------------------------------------------------------------------------
    
    /// read
    virtual void read(InputWrapper&, Simul&);
    
    /// write
    virtual void write(OutputWrapper&) const;
 
    //--------------------------------------------------------------------------
    
    /// check that fbAbs is within Fiber::abscissaM() and Fiber::abscissaP()
    void         checkAbscissa() const;
    
    /// check validity of the interpolation (debuging purposes)
    int          bad() const;
};

#endif

