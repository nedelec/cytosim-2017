// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef DYNAMIC_FIBER_PROP
#define DYNAMIC_FIBER_PROP

#include "fiber_prop.h"


class Glossary;


/// additional Property for DynamicFiber
/**
 @ingroup Properties
 */
class DynamicFiberProp : public FiberProp
{
    friend class DynamicFiber;
    
public:
    
    /**
     @defgroup DynamicFiberPar Parameters of DynamicFiber
     @ingroup Parameters
     Inherits @ref FiberPar.
     @{
     */
    
    /// see @ref DynamicFiber
    
    /// Length of discrete units of assembly/disassembly
    real    unit_length;
    
    /// Characteristic force for polymer assembly
    real    growing_force;
    
    /// Speed of assembly
    real    growing_speed[2];
    
    /// Hydrolysis rate of G-units, which defines the catastrophe rate
    real    hydrolysis_rate;
    
    /// Speed of disassembly
    real    shrinking_speed;
    
    /// Action that is taken when the fiber shrinks below min_length
    Fate    fate;

    /// @}
    //------------------ derived variables below ----------------
    
private:
    
    real    growing_rate_dt[2];
    real    hydrolysis_rate_2dt;
    real    shrinking_rate_dt;
    
public:
    
    /// constructor
    DynamicFiberProp(const std::string& n) : FiberProp(n) { clear(); }

    /// destructor
    ~DynamicFiberProp() { }
    
    /// return a Fiber with this property
    Fiber* newFiber() const;
    
    /// set default values
    void clear();
       
    /// set using a Glossary
    void read(Glossary&);
   
    /// check and derive parameter values
    void complete(SimulProp const*, PropertyList*);
    
    /// return a carbon copy of object
    Property* clone() const { return new DynamicFiberProp(*this); }

    /// write
    void write_data(std::ostream &) const;

};

#endif

