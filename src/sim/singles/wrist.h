// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef WRIST_H
#define WRIST_H

#include "single.h"
#include "point_exact.h"


/// a Single anchored to a Mecable.
/**
 The anchorage is described by PointExact sBase:
 - the Mecable is sBase.object()
 - the index of the model-point on this Mecable is sBase.index()
 .

 @ingroup SingleGroup
 */
class Wrist : public Single
{
protected:
        
    /// describes the anchorage: Mecable and index of model-point
    PointExact    sBase;
    
public:
     
    /// constructor
    Wrist(SingleProp const*, Mecable const*, unsigned);

    /// destructor
    ~Wrist();
    
    //--------------------------------------------------------------------------
    
    /// return the position in space of the object
    Vector  position()                   const  { return sBase.pos(); }
    
    /// true if object accepts translations
    bool    translatable()               const  { return false; }
    
    /// translate object's position by the given vector
    void    translate(Vector const& T)          { }
    
    /// modulo the position of the grafted
    void    foldPosition(const Modulo * s)      { }

    //--------------------------------------------------------------------------
    
    /// Object to which this is attached
    Mecable const* foot()                 const { return sBase.mecable(); }

    /// the position of what is holding the Hand
    Vector  posFoot()                     const { return sBase.pos(); }
    
    /// force = stiffness * ( posFoot() - posHand() )
    Vector  force() const;
    
    
    /// Monte-Carlo step for a free Single
    void    stepFree(const FiberGrid&);
    
    /// Monte-Carlo step for a bound Single
    void    stepAttached();
    
    
    /// true if Single creates an interaction
    bool    hasInteraction() const { return true; }

    /// add interactions to the Meca
    void    setInteractions(Meca &) const;

    //--------------------------------------------------------------------------
    
    //The Wrist uses a specific TAG to distinguish itself from the Single
    static const Tag TAG = 'w';
    
    /// return unique character identifying the class
    Tag     tag() const { return TAG; }
    
    /// read from file
    void    read(InputWrapper&, Simul&);
    
    /// write to file
    void    write(OutputWrapper&) const;
    
};


#endif
