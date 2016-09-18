// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef COUPLE_LONG_H
#define COUPLE_LONG_H

#include "couple.h"

/// A Couple with a non-zero resting length
/**
 The CoupleLong adds a non-zero resting length to Couple,
 using Meca:interSideLink()

 CoupleLong is automatically selected if ( prop:length > 0 )
 @ingroup CoupleGroup
 */
class CoupleLong : public Couple
{
    /// the side (top/bottom) of the interaction
    mutable Torque mArm;
    
    /// used to calculate \a mArm
    static Torque calcArm(const PointInterpolated & pt, Vector const& pos, real len);
    
public:
    
    /// create following the specifications in the CoupleProp
    CoupleLong(CoupleProp const*, Vector const & w = Vector(0,0,0));

    /// destructor
    virtual ~CoupleLong();
    
    /// position on the side of fiber1 used for sideInteractions
    Vector  posSide() const;
 
    /// force between hands, essentially: stiffness * ( cHand2->posHand() - cHand1->posHand() )
    Vector  force1() const;
    
    /// add interactions to the Meca
    void    setInteractions(Meca &) const;
    
};


#endif

