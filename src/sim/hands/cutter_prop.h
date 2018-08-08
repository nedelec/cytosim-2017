// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef CUTTER_PROP_H
#define CUTTER_PROP_H

#include "hand_prop.h"


/// Additional Property for Cutter
/**
 @ingroup Properties
 */
class CutterProp : public HandProp
{
    friend class Cutter;
    
public:
    
    /**
     @defgroup CutterPar Parameters of Cutter
     @ingroup Parameters
     Inherits @ref HandPar.
     Check the examples!
     @{
     */
    
    /// rate of cutting event
    real    cutting_rate;
    
    
    /// dynamic state of newly created Fiber ends
    /**
      This defines the dynamic state of the new ends that are created by a cut:
      - new_end_state[0] is for the new PLUS_END,
      - new_end_state[1] is for the new MINUS_END
      .
     */
    int     new_end_state[2];
    
    /// @}
    //------------------ derived variables below ----------------
    
private:
    
    real cutting_rate_prob;
    
public:
    
    /// constructor
    CutterProp(const std::string& n) : HandProp(n)  { clear(); }
    
    /// destructor
    ~CutterProp() { }
    
    /// return a Hand with this property
    virtual Hand * newHand(HandMonitor* h) const;
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute values derived from the parameters
    void complete(SimulProp const*, PropertyList*);
    
    /// return a carbon copy of object
    Property* clone() const { return new CutterProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
    
};

#endif

