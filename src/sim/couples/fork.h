// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FORK_H
#define FORK_H

#include "couple.h"
class ForkProp;

/// A specialized kind of Couple
/**
 The Fork is unfinished and should not be used.
 
 The plan is to use Meca:interTorque()
 @ingroup CoupleGroup
 */
class Fork : public Couple
{
    
    /// property
    ForkProp const* prop;
        
public:
    
    /// create following the specifications in the CoupleProp
    Fork(ForkProp const*, Vector const & w = Vector(0,0,0));

    /// destructor
    virtual ~Fork();
    
    //--------------------------------------------------------------------------
    
    /// add interactions to the Meca
    void    setInteractions(Meca &) const;
    
    /// simulation step for a free Couple, implementing CrosslinkProp::trans_activated
    void    stepFF(const FiberGrid&);

};


#endif

