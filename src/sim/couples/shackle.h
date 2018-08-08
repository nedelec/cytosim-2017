// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SHACKLE_H
#define SHACKLE_H

#include "couple.h"
#include "shackle_prop.h"

/// A specialized kind of Couple
/**
 The Shackle creates a connection that is slipery on Hand1
 by using Meca:interSlidingLink().
 
 Note: this is highly experimental!
 @ingroup CoupleGroup
 */
class Shackle : public Couple
{
    /// the side (top/bottom) of the interaction
    mutable Torque mArm;
    
    /// used to calculate \a mArm
    static Torque calcArm(const PointInterpolated & pt, Vector const& pos, real len);
   
protected:
    
    /// property
    ShackleProp const* prop;
        
public:
    
    /// create following the specifications in the CoupleProp
    Shackle(ShackleProp const* p, Vector const & w = Vector(0,0,0));

    /// destructor
    virtual ~Shackle();
    
    //--------------------------------------------------------------------------
    
    /// simulation step if doubly attached
    void    stepAA();
    
    /// add interactions to the Meca
    void    setInteractions(Meca &) const;
    
};


#endif

