// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SINGLE_H
#define SINGLE_H

#include "dim.h"
#include "vector.h"
#include "movable.h"
#include "object.h"
#include "hand_monitor.h"
#include "point_exact.h"
#include "single_prop.h"
#include "hand.h"

class Meca;
class Modulo;
class Glossary;
class FiberGrid;
class Fiber;


/// A point-like object containing one Hand.
/**
 A Single contains one pointer to Hand, and consequently
 inherit the 2 possible states: \a attached or \a free.
 
 By default:
 - Free Single are diffusing, and try to bind to nearby Fibers,
 - Attached Singles are moving along the Fiber to which their Hand is attached.
 .
 
 However, two derived classes change this behavior:
 -# a Picket is fixed in position and do not diffuse,
 -# a Wrist is attached to one model point of a Mecable.
 .
 
 Attached Wrist and Picket exert a force on the Fiber to which the Hand is attached.
 
 For WristLong and PicketLong, this force can have a non-zero resting length.
 
 Wrist and Picket can be distinguished with Single::foot():
 - Single::foot() and Picket::foot() return zero, 
 - Wrist::foot() returns the Mecable on which the Wrist is attached.
 .

 @ingroup SingleGroup
 */

class Single : public Object, public HandMonitor
{
public:
    
    /// property
    SingleProp const* prop;
    
protected:    
    
    /// the motor domain
    Hand *        sHand;    
    
    /// the position of the foot
    Vector        sPos;

private:
    
    /// specialization of HandMonitor
    void      afterAttachment();
    /// specialization of HandMonitor
    void      afterDetachment();
    /// specialization of HandMonitor
    void      beforeDetachment();
    /// = number() of the foot() or Single::number()
    Number    objNumber() const { if ( foot() ) return foot()->number(); else return Object::number(); }
    /// specialization of HandMonitor
    real      interactionLength() const;

public:

    /// constructor at specified position
    Single(SingleProp const*, Vector const& = Vector(0,0,0));

    ///destructor
    virtual ~Single();
    
    //--------------------------------------------------------------------------
    
    ///a reference to the Hand
    Hand *  hand()                                      { return sHand; }
    
    ///position of the Hand
    Vector  posHand()                            const  { return sHand->pos(); }
    
    /// sHand->attached()
    bool    attached()                           const  { return sHand->attached(); }
    
    /// attach the hand at the given position
    void    attach(FiberBinder& fb)                     { return sHand->attach(fb); }

    /// attach the hand at the given position 
    void    attachTo(Fiber * f, real ab, FiberEnd from) { return sHand->attachTo(f, ab, from); }
    
    /// attach Hand at the given end
    void    attachToEnd(Fiber * f, FiberEnd end)        { return sHand->attachToEnd(f, end); }
    
    /// detach
    void    detach()                                    { return sHand->detach(); }
    
    /// Fiber to which this is attached
    Fiber*  fiber()                                     { return sHand->fiber(); }
    
    /// direction of Fiber at attachment point
    Vector  dirFiber()                           const  { return sHand->dirFiber(); }

    //--------------------------------------------------------------------------
    
    ///return the position in space of the object
    virtual Vector  position() const;
    
    /// Single can be translated only if it is not attached
    virtual bool    translatable()               const  { return !sHand->attached(); }
    
    ///translate object's position by the given vector
    virtual void    translate(Vector const& w)          { assert_false(sHand->attached()); sPos += w; }
    
    /// move object to specified position
    virtual void    setPosition(Vector const& w)        { sPos = w; }

    /// modulo the position of the grafted
    virtual void    foldPosition(const Modulo * s);
    
    //--------------------------------------------------------------------------
    
    /// the Mecable to which this is attached, or zero
    virtual Mecable const* foot()                const  { return 0; }
    
    /// the position of the foot holding the Hand
    virtual Vector  posFoot()                    const  { return sPos; }
    
    /// position on the side of fiber used for sideInteractions
    virtual Vector  posSide()                    const  { return sHand->pos(); }
    
    /// force = zero for a diffusible Single
    virtual Vector  force()                      const  { return Vector(0,0,0); }

    /// Monte-Carlo step for a free Single
    virtual void    stepFree(const FiberGrid&);
    
    /// Monte-Carlo step for a bound Single
    virtual void    stepAttached();
    
    /// true if Single creates an interaction
    virtual bool    hasInteraction() const;
    
    /// add interactions to the Meca
    virtual void    setInteractions(Meca &) const;
    
    //--------------------------------------------------------------------------
    
    /// a static_cast<> of Node::next()
    Single*         next()   const  { return static_cast<Single*>(nNext); }
    
    /// a static_cast<> of Node::prev()
    Single*         prev()   const  { return static_cast<Single*>(nPrev); }

    //--------------------------------------------------------------------------

    /// a unique character identifying the class
    static const Tag TAG = 's';
    
    /// return unique character identifying the class
    virtual Tag     tag() const { return TAG; }
    
    /// return Object Property
    const Property* property() const { return prop; }
    
    /// read from file
    virtual void    read(InputWrapper&, Simul&);
    
    /// write to file
    virtual void    write(OutputWrapper&) const;
    
};


#endif
