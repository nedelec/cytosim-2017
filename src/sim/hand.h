// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef HAND_H
#define HAND_H

#include "fiber_binder.h"

class HandMonitor;
class FiberGrid;
class FiberProp;
class HandProp;
class Simul;

/// Simulates the stochastic binding/unbinding of a FiberBinder
/**
 A Hand is always part of a larger construct, for example Single or Couple.
 
 Hand is the parent to many class that implement different fiber-related activities.
 Hand provides binding/unbinding capacity to these derived classes.
 
 Attachment occurs with constant rate  @ref HandPar "attach_rate" to any fiber located
 at distance  @ref HandPar "attach_range" or less.
 If attachment occurs, it happens on the closest point of the fiber,
 which is either the projection of the point on the fiber, or one of the fiber end.
 
 Detachment increases exponentially with force:
 @code
 off_rate = unbinding_rate * exp( force.norm() / unbinding_force )
 @endcode

 See @ref HandPar.
 @ingroup HandGroup
 */
class Hand : public FiberBinder
{

private:
    
    /// disabled default constructor
    Hand();

protected:
    
    /// the monitor associated with this Hand
    HandMonitor*   haMonitor;
    
    /// Gillespie normalized time for attachment
    real           nextAttach;
    
    /// Gillespie normalized time for detachment
    real           nextDetach;
    
    /// test for detachment with rate prop->unbinding_rate
    bool           testDetachment();
    
    /// test for detachment with Kramers theory
    bool           testKramersDetachment(real force);
    
public:
    
    /// Property
    HandProp const* prop;
    
    /// constructor
    /**
     To create a new Hand, you need to have a HandProp.
     HandProp is parent to several classes, exactly fiMirroring the hierarchy of Hands.
     
     The correct derived class can be created by its associated HandProp:
     @code
     HandProp * hp = HandProp::newProperty(name, opt);
     Hand * h = hp->newHand(this);
     @endcode
     */
    Hand(HandProp const*, HandMonitor* h);

    /// destructor
    virtual ~Hand();

    /// tell if attachment at given site is possible
    virtual bool   attachmentAllowed(FiberBinder& site);
    
    /// attach the hand at the position described by site
    virtual void   attach(FiberBinder const& site);
    
    /// detach
    virtual void   detach();
    
    /// simulate when the Hand is not attached
    virtual void   stepFree(const FiberGrid&, Vector const & pos);

    /// simulate when the Hand is attached but not under load
    virtual void   stepUnloaded();

    /// simulate when the Hand is attached and under load
    virtual void   stepLoaded(Vector const & force);
    
    /// this is called when the attachment point is outside the Fiber's range
    virtual void   handleOutOfRange(FiberEnd);
    
    
    /// attach to the given position on the given Fiber (calls attach(FiberBinder))
    void           attachTo(Fiber *, real ab, FiberEnd from);
    
    /// attach to the given end on the given Fiber (calls attach(FiberBinder))
    void           attachToEnd(Fiber *, FiberEnd end);
    
    /// read
    void           read(InputWrapper&, Simul&);
    
    /// write
    void           write(OutputWrapper&) const;
    
};

#endif

