// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FORK_PROP_H
#define FORK_PROP_H

#include "couple_prop.h"


/// Additional Property for Fork
/**
 @ingroup Properties
*/
class ForkProp : public CoupleProp
{
    friend class Fork;
    
public:
    
    /**
     @defgroup ForkPar Parameters of Fork
     @ingroup Parameters
     Inherits @ref CouplePar.
     @{
     */
    
    /// if ( trans_activated == 1 ), hand2 is active only if hand1 is bound
    bool         trans_activated;

    
    /// @}
    //------------------ derived variables below ----------------

public:
    
    /// constructor
    ForkProp(const std::string& n) : CoupleProp(n)  { clear(); }
    
    /// destructor
    ~ForkProp() { }
    
    /// return a Hand with this property
    Couple * newCouple(Glossary * opt) const;
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute values derived from the parameters
    void complete(SimulProp const*, PropertyList*);
    
    /// return a carbon copy of object
    Property* clone() const { return new ForkProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
    
};

#endif

