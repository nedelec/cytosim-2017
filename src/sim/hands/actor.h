// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef ACTOR_H
#define ACTOR_H

#include "hand.h"
class ActorProp;

/// A Hand that can act on a Fiber
/**
 The Actor is a Hand, and can thus bind and unbind from fibers.
 
 The Actor currently does nothing else.
 It exists as a template, or as a class that can be used when new functionalities are needed

 See Examples and the @ref ActorPar.
 @ingroup HandGroup 
 */
class Actor : public Hand
{
private:
    
    /// disabled default constructor
    Actor();
    
    /// Property
    ActorProp const* prop;

public:
    
    /// constructor
    Actor(ActorProp const* p, HandMonitor* h);
    
    /// destructor
    ~Actor() {}
    
    
    /// simulate when \a this is attached but not under load
    void   stepUnloaded();
    
    /// simulate when \a this is attached and under load
    void   stepLoaded(Vector const & force);
    
};

#endif

