// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SLIDER_PROP_H
#define SLIDER_PROP_H

#include "hand_prop.h"


/// Additional Property for Slider
/**
 @ingroup Properties
 */
class SliderProp : public HandProp
{
    friend class Slider;
    
public:
    
    /**
     @defgroup SliderPar Parameters of Slider
     @ingroup Parameters
     Inherits @ref HandPar.
     Check the examples!
     @{
     */
    
    /// mobility coefficient
    real    mobility;
    
    /// link stiffness used to calculate implicit motion (default=unset)
    /**
     If this parameter is set, the mobility used is 
     @code
     mobility = 1.0 / ( stiffness * sp->time_step );
     @endcode
     Which effectively correspond to the maximum mobility possible, given times_step.
     
     The stiffness needs to be set from the link stiffness,
     and from the mobility of the other link. Generally:
     -# = link-stiffness if the Hand is part of a Single, or if it is the only slider in a Couple
     -# = 2 * link-stiffness if the Hand is part of a Couple made of two Slider
     .
     */
    real    stiffness;
    
    /// @}
    //------------------ derived variables below ----------------
    
private:
    
    real    mobility_dt;

public:
    
    /// constructor
    SliderProp(const std::string& n) : HandProp(n)  { clear(); }
    
    /// destructor
    ~SliderProp() { }
    
    /// return a Hand with this property
    virtual Hand * newHand(HandMonitor* h) const;
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute values derived from the parameters
    void complete(SimulProp const*, PropertyList*);
    
    /// return a carbon copy of object
    Property* clone() const { return new SliderProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
    
};

#endif

