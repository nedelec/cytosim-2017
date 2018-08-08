// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FORK_H
#define FORK_H

#include "couple.h"
class ForkProp;

/// A specialized kind of Couple
/**
 The fork connect two fibers at an angle.
 It creates a torque between the two fibers, with a resting angle defined by 'ForkProp::angle',
 and a rotational stiffness which is 'ForkProp::angular_stiffness'.

 @ingroup CoupleGroup
 */
class Fork : public Couple
{
    /// direction for flipping (-1 or +1)
    mutable int     flip;
    
    /// Property
    ForkProp const* prop;
        
public:
    
    /// create following the specifications in the CoupleProp
    Fork(ForkProp const*, Vector const & w = Vector(0,0,0));

    /// destructor
    virtual ~Fork();
    
    /// simulation step for a free Couple, implementing CrosslinkProp::trans_activated
    void    stepFF(const FiberGrid&);
    
    /// add interactions to the Meca
    void    setInteractions(Meca &) const;

};


#endif

