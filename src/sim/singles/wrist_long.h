// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef WRIST_LONG_H
#define WRIST_LONG_H

#include "wrist.h"
#include "point_exact.h"


/// a Wrist with a non-zero resting length.
/**
 The anchorage is described by PointExact sBase:
 - the Mecable is sBase.object()
 - the index of the model-point on this Mecable is sBase.index()
 .
 It has a non-zero resting length.
 
 @ingroup SingleGroup
 */
class WristLong : public Wrist
{
    /// the side (top/bottom) of the interaction
    mutable Torque  mArm;
    
    /// used to calculate \a mArm
    static Torque calcArm(const PointInterpolated & pt, Vector const& pos, real len);
    
public:
     
    /// constructor
    WristLong(SingleProp const*, Mecable const*, unsigned);

    /// destructor
    ~WristLong();

    //--------------------------------------------------------------------------
    
    /// position on the side of fiber used for sideInteractions
    Vector  posSide() const;
    
    /// force = stiffness * ( posFoot() - posHand() )
    Vector  force() const;
        
    /// add interactions to the Meca
    void    setInteractions(Meca &) const;
    
};


#endif
