// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef TREADMILLING_FIBER_H
#define TREADMILLING_FIBER_H

#include "sim.h"
#include "vector.h"
#include "node_list.h"
#include "fiber.h"

class TreadmillingFiberProp;


/// A Fiber with assembly at both ends 
/**
 This is not documented yet!

 See the @ref TreadmillingFiberPar.

 @todo Document TreadmillingFiber
 @ingroup FiberGroup
 */
class TreadmillingFiber : public Fiber
{
public:
    
    /// the Property of this object
    TreadmillingFiberProp const* prop;
   
private:
    
    /// state of PLUS_END
    int        stateP;
    
    /// assembly during last time-step
    real       mGrowthP;
    
    /// state of MINUS_END
    int        stateM;
    
    /// assembly during last time-step
    real       mGrowthM;
    
public:
  
    /// constructor
    TreadmillingFiber(TreadmillingFiberProp const*);

    /// destructor
    virtual ~TreadmillingFiber();
        
    //--------------------------------------------------------------------------
    
    /// return assembly/disassembly state of the end \a which
    int         dynamicState(FiberEnd which) const;
    
    /// set state of FiberEnd \a which to \a new_state
    void        setDynamicState(FiberEnd which, int new_state);
    
    /// the amount of freshly assembled polymer during the last time step
    real        freshAssembly(FiberEnd which) const;
    
    //--------------------------------------------------------------------------
    
    /// cut fiber at distance \a abs from MINUS_END
    Fiber *     severM(real abs);
    
    /// join two fibers
    void        join(Fiber * fib);
    
    /// simulate dynamic instability of PLUS_END
    int         stepPlusEnd();
    
    /// simulate dynamic instability of MINUS_END
    int         stepMinusEnd();
    
    /// monte-carlo step
    void        step();
    
    //--------------------------------------------------------------------------
    
    /// write to OutputWrapper
    void        write(OutputWrapper&) const;

    /// read from InputWrapper
    void        read(InputWrapper&, Simul&);
    
};


#endif
