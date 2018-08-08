// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef BRIDGE_PROP_H
#define BRIDGE_PROP_H

#include "couple_prop.h"


/// Additional Property for Bridge
/**
 @ingroup Properties
*/
class BridgeProp : public CoupleProp
{
    
    friend class Bridge;
    
public:
    
    /**
     @defgroup BridgePar Parameters of Bridge
     @ingroup Parameters
     Inherits @ref CouplePar.
     @{
     */
    

    /// Specificity of binding to a pair a Fiber
    /**
     Could be:
     - `none` (default)
     - `parallel`
     - `antiparallel`
     */    
    Specificity  specificity;
   
    /// if ( trans_activated == 1 ), hand2 is active only if hand1 is bound
    bool         trans_activated;
    
    /// @}
    //------------------ derived variables below ----------------

public:
    
    /// constructor
    BridgeProp(const std::string& n) : CoupleProp(n)  { clear(); }
    
    /// destructor
    ~BridgeProp() { }
    
    /// return a Hand with this property
    Couple * newCouple(Glossary * opt) const;
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute values derived from the parameters
    void complete(SimulProp const*, PropertyList*);
    
    /// return a carbon copy of object
    Property* clone() const { return new BridgeProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
    
};

#endif

