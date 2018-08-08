// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef PICKET_H
#define PICKET_H

#include "single.h"
class FiberGrid;

/// a Single attached at a fixed position.
/**
 This Single is fixed at its foot position in absolute space.
 A link is created if the Hand is attached.

 @ingroup SingleGroup
 */
class Picket : public Single
{
        
public:

    /// constructor
    Picket(SingleProp const*, Vector const& = Vector(0,0,0));

    /// destructor
    ~Picket();
    
    ///return the position in space of the object
    Vector  position()           const  { return sPos; }

    /// true if object accepts translations
    bool    translatable()       const  { return true; }
    
    /// translate object's position by the given vector
    void    translate(Vector const& w)  { sPos += w; }

    /// sPos should never changed
    void    beforeDetachment() {}

    /// tension in the link = stiffness * ( posFoot() - posHand() )
    Vector  force() const;

    /// Monte-Carlo step for a free Single
    void    stepFree(const FiberGrid&);
    
    /// Monte-Carlo step for a bound Single
    void    stepAttached();
    
    /// true if Single creates an interaction
    bool    hasInteraction() const { return true; }
    
    /// add interactions to the Meca
    void    setInteractions(Meca &) const;
    
};


#endif
