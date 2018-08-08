// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef CROSSLINK_H
#define CROSSLINK_H

#include "couple.h"
class CrosslinkProp;

/// A specialized kind of Couple
/**
 The Crosslink can have:
 - specificity (parallel/antiparallel)
 - trans_activated (hand2 is active only if hand1 is bound)
 .
 It has a zero resting length, and uses Meca:interLink()
 
 CrosslinkLong has a non-zero resting length, and is selected automatically 
 @ingroup CoupleGroup
 */
class Crosslink : public Couple
{
protected:
    
    /// property
    CrosslinkProp const* prop;
    
public:
    
    /// create following the specifications in the CoupleProp
    Crosslink(CrosslinkProp const*, Vector const & w = Vector(0,0,0));

    /// destructor
    virtual ~Crosslink();
    
    /// control function for attachements
    bool allowAttachment(const FiberBinder & fb);
    
    /// simulation step for a free Couple, implementing CrosslinkProp::trans_activated
    void    stepFF(const FiberGrid&);
    
};


#endif

