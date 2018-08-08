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
     @{
     */
    
    /// mobility coefficient (default=0)
    /*
     The speed of a slider is proportional to projected force:
     @code
     f_parallel = force_vector . direction_of_fiber
     speed = mobility * f_parallel
     abscissa = abscissa + time_step * speed
     @endcode
     
     The mobility has unit of speed per force = um . s^-1 . pN^-1
     
     In principle, the ‘mobility’ is related to the diffusion constant of the molecule
     along the filament lattice by Einstein’s relation: `diffusion = mobility * kT`.
     So measuring the 1D diffusion constant of the molecules bound to a filament
     may provide an estimate of the mobility.
     */
    real    mobility;
    

    /// @}
    
private:
    
    /// derived variable:
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
    
    /// perform additional tests for the validity of parameters, given the elasticity
    void checkStiffness(real stiff, real len, real mul, real kT) const;

    /// return a carbon copy of object
    Property* clone() const { return new SliderProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
    
};

#endif

