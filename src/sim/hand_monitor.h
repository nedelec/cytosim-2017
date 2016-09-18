// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef HAND_MONITOR
#define HAND_MONITOR

#include "real.h"
#include "vector.h"
#include "inventoried.h"

class Hand;
class Simul;
class FiberBinder;


/// base class to monitor and control Hand's actions
class HandMonitor
{

public:
    
    virtual ~HandMonitor() {}
    
    /// called just before attachement. Returning false prevents the attachment
    virtual bool allowAttachment(const FiberBinder & site) { return true; }
    
    /// called after attachement
    virtual void afterAttachment() {}
    
    /// called before detachment
    virtual void beforeDetachment() {}

    /// called after detachment
    virtual void afterDetachment() {}
    
    /// number() for associated object
    virtual Number objNumber() const { return 0; }
    
    /// return the Hand that is not the argument, in a Couple
    virtual Hand * otherHand(Hand *) const { return 0; }

    /// return the direction of the Fiber for the Hand that is not the argument, in a Couple
    /** If the hand is not part of a Couple, this return a random unit vector */
    virtual Vector otherDirection(Hand *) const { return Vector::randUnit(); }

    /// resting length of the interaction
    virtual real   interactionLength() const { return 0; }
};


#endif
