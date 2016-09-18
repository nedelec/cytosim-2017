// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef TUBULE_PROP
#define TUBULE_PROP

#include "fiber_prop.h"

class Tubule;
class Glossary;


/// additional Property for Tubule
/**
 @ingroup Properties
 */
class TubuleProp : public FiberProp
{
    friend class Tubule;
    
public:
    
    /**
     @defgroup TubulePar Parameters of Tubule
     @ingroup Parameters
     Inherits @ref FiberPar.
     @{
     */
    
    /// see @ref Tubule

    /// Model for dynamic assembly [0]=PLUS_END, [1]=MINUS_END
    int     dynamic_model[2];
    
    /// Characteristic force for polymer assembly
    real    growing_force;
    
    /// Minimum authorized length (used only for dynamic fibers)
    /**
     When the fiber becomes shorter than \a min_length,
     the action specified by \a fate is applied.
     */
    real    min_length;
    
    /// Action that is taken when the fiber shrinks below min_length
    Fate    fate;
    
    real    dynamic_speed1[4];
    real    dynamic_speed2[4];
    real    dynamic_trans1[4];
    real    dynamic_trans2[4];
    
    /// @}
    //------------------ derived variables below ----------------
    
private:
    

public:
    
    /// constructor
    TubuleProp(const std::string& n) : FiberProp(n) { clear(); }

    /// destructor
    ~TubuleProp() { }
    
    /// return a Fiber with this property
    Fiber* newFiber() const;
    
    /// set default values
    void clear();
       
    /// set using a Glossary
    void read(Glossary&);
   
    /// check and derive parameter values
    void complete(SimulProp const*, PropertyList*);
    
    /// return a carbon copy of object
    Property* clone() const { return new TubuleProp(*this); }

    /// write
    void write_data(std::ostream &) const;

};

#endif

