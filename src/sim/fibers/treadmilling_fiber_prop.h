// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef TREADMILLING_FIBER_PROP
#define TREADMILLING_FIBER_PROP

#include "fiber_prop.h"


class Glossary;


/// additional Property for TreadmillingFiber
/**
 @ingroup Properties
 Assembly is continuous, and can occur at both ends.
 */
class TreadmillingFiberProp : public FiberProp
{
    friend class TreadmillingFiber;
    
public:
    
    /**
     @defgroup TreadmillingFiberPar Parameters of TreadmillingFiber
     @ingroup Parameters
     Inherits @ref FiberPar.
     @{
     */
    
    /// see @ref TreadmillingFiber

    /// Characteristic force for polymer assembly
    real    growing_force[2];
    
    /// Speed of assembly
    real    growing_speed[2];
    
    /// Speed of disassembly
    real    shrinking_speed[2];
 
    /// @}
    //------------------ derived variables below ----------------
    
private:
    
    real    growing_speed_dt[2];
    real    shrinking_speed_dt[2];
    
public:
    
    /// constructor
    TreadmillingFiberProp(const std::string& n) : FiberProp(n) { clear(); }

    /// destructor
    ~TreadmillingFiberProp() { }
    
    /// return a Fiber with this property
    Fiber* newFiber() const;
    
    /// set default values
    void clear();
       
    /// set using a Glossary
    void read(Glossary&);
   
    /// check and derive parameter values
    void complete(SimulProp const*, PropertyList*);
    
    /// return a carbon copy of object
    Property* clone() const { return new TreadmillingFiberProp(*this); }

    /// write
    void write_data(std::ostream &) const;

};

#endif

