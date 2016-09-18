// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef BRIDGE_H
#define BRIDGE_H

#include "couple.h"
class BridgeProp;

/// A specialized kind of Couple
/**
 The Bridge can have:
 - specificity (parallel/antiparallel)
 - trans_activated (hand2 is active only if hand1 is bound)
 .
 It must have a non-zero resting length, and uses Meca:interSideLink()
 
 @todo: change Bridge to use Meca::interLongLink()
 
 For zero-resting length, use Crosslink
 @ingroup CoupleGroup
 */
class Bridge : public Couple
{
    /// property
    BridgeProp const* prop;
    
    /// the side (top/bottom) of the interaction
    mutable Torque mArm;
    
    /// used to calculate \a mArm
    static Torque calcArm(const PointInterpolated & pt, Vector const& pos, real len);
    
public:
    
    /// create following the specifications in the CoupleProp
    Bridge(BridgeProp const*, Vector const & w = Vector(0,0,0));

    /// destructor
    virtual ~Bridge();
    
    //--------------------------------------------------------------------------
    
    /// control function for attachements
    bool    allowAttachment(const FiberBinder & fb);
    
    /// simulation step for a free Couple, implementing BridgeProp::trans_activated
    void    stepFF(const FiberGrid&);
    
    /// position on the side of fiber1 used for sideInteractions
    Vector  posSide() const;
 
    /// force between hands, essentially: stiffness * ( cHand2->posHand() - cHand1->posHand() )
    Vector  force1() const;
    
    /// add interactions to the Meca
    void    setInteractions(Meca &) const;
    
};


#endif

