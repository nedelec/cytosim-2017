// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SHACKLE_LONG_H
#define SHACKLE_LONG_H

#include "shackle.h"

/// A Shackle with a non-zero resting length
/**
 The ShackleLong adds a non-zero resting length to Duo,
 using Meca:interSideSlidingLink()
 
 For zero-resting length, use Shackle
 
 ShackleLong is automatically selected if ( prop:length > 0 )
 
 This is highly experimental!
 @ingroup CoupleGroup
 */
class ShackleLong : public Shackle
{
    
    /// the side (top/bottom) of the interaction
    mutable Torque mArm;
    
    /// used to calculate \a mArm
    static Torque calcArm(const PointInterpolated & pt, Vector const& pos, real len);
    
public:
    
    /// create following the specifications in the CoupleProp
    ShackleLong(ShackleProp const*, Vector const & w = Vector(0,0,0));

    /// destructor
    virtual ~ShackleLong() {}
    
    //--------------------------------------------------------------------------
    
    /// position on the side of fiber1 used for sideInteractions
    Vector  posSide() const;

    /// force between hands
    Vector  force1() const;
    
    /// add interactions to the Meca
    void    setInteractions(Meca &) const;
    
};


#endif

