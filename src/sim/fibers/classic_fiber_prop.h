// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef CLASSIC_FIBER_PROP
#define CLASSIC_FIBER_PROP

#include "fiber_prop.h"


class Glossary;


// This makes the catastrophe rate dependent on length of fiber
//#define NEW_LENGTH_DEPENDENT_CATASTROPHE


/// additional Property for ClassicFiber
/**
 @ingroup Properties
 */
class ClassicFiberProp : public FiberProp
{
    friend class ClassicFiber;
    
public:
    
    /**
     @defgroup ClassicFiberPar Parameters of ClassicFiber
     @ingroup Parameters
     Inherits @ref FiberPar.
     @{
     */
    
    /// see @ref ClassicFiber
    
    /// Speed of assembly state
    /**
     @code
     growth_speed = monomer_fraction * growing_speed[0] * exp(force/growing_force) + growing_speed[1];
     @endcode
     
     monomer_fraction in [0,1] reflect the amount of free monomers.
     Antagonistic force is negative ( force < 0 ). 
     Force directed in the same direction as assembly has no effect.
     
     The parameters are:
     - growing_speed[0] = force-dependent assembly rate.
     - growing_speed[1] = constant term, which can be negative to represent spontaneous disassembly.
     - growing_force = characteristic force
     .
     */    
    real    growing_speed[2];
    
    
    /// Characteristic force of assembly state
    /**
     Antagonistic force decrease assembly rate exponentially.
     */
    real    growing_force;
    
    
    /// speed of disassembly state
    real    shrinking_speed;
    
    
    /// Rate of stochastic switch from assembly to disassembly
    /**
     The catastrophe rate depends on the growth rate of the corresponding tip,
     which is itself reduced by antagonistic force. \n
     The correspondance is : 
     @code
     1/catastrophe_rate = a + b * growth_speed
     @endcode
     where \a growth_speed is calculated as explained in @ref growing_speed,
     and parameters \a a and \a b are derived from:
     - catastrophe_rate[0] = the catastrophe rate in the absence of force.
     - catastrophe_rate[1] = the catastrophe rate of a stalled tip.
     .
     
     */
    real    catastrophe_rate[2];

    
    /// Rate of stochastic switch from disassembly to assembly
    real    rescue_rate;
    
    
    /// Action that is taken when the fiber shrinks below `min_length`
    Fate    fate;
    
#ifdef NEW_LENGTH_DEPENDENT_CATASTROPHE
    
    /// Switch to enable the length-dependent catastrophe rate
    /**
     If this is defined, the catastrophe rate will depend on the length of the fiber:
     @code
     catastrophe_rate_real = catastrophe_rate * length() / catastrophe_length;
     @endcode
     */
    real    catastrophe_length;
    
#endif

    /// @}
    //------------------ derived variables below ----------------
    
private:
    
    real    shrinking_speed_dt;
    real    growing_speed_dt[2];
    real    catastrophe_rate_dt, cata_coef;
    real    rescue_rate_prob;

public:
    
    /// constructor
    ClassicFiberProp(const std::string& n) : FiberProp(n) { clear(); }

    /// destructor
    ~ClassicFiberProp() { }
    
    /// return a Fiber with this property
    Fiber* newFiber() const;
    
    /// set default values
    void clear();
       
    /// set using a Glossary
    void read(Glossary&);
   
    /// check and derive parameter values
    void complete(SimulProp const*, PropertyList*);
    
    /// return a carbon copy of object
    Property* clone() const { return new ClassicFiberProp(*this); }

    /// write
    void write_data(std::ostream &) const;

};

#endif

