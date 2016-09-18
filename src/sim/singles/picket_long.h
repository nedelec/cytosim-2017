// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef PICKET_LONG_H
#define PICKET_LONG_H

#include "picket.h"
class FiberGrid;

/// a Picket with a non-zero resting length.
/**
 This single is fixed at its foot position in absolute space.
 It has a non-zero resting length.

 @ingroup SingleGroup
 */
class PicketLong : public Picket
{
    
    /// the side (top/bottom) of the interaction
    mutable Torque mArm;
    
    /// used to recalculate \a arm
    static Torque calcArm(const PointInterpolated & pt, Vector const& pos, real len);
    
public:

    /// constructor
    PicketLong(SingleProp const*, Vector const& = Vector(0,0,0));

    /// destructor
    ~PicketLong();
        
    /// position on the side of fiber used for sideInteractions
    Vector  posSide() const;
    
    /// force = stiffness * ( posFoot() - posHand() )
    Vector  force() const;
    
    /// add interactions to the Meca
    void    setInteractions(Meca &) const;
    
};


#endif
