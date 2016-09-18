// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef TUBULE_H
#define TUBULE_H

#include "sim.h"
#include "vector.h"
#include "node_list.h"
#include "common.h"
#include "object.h"
#include "fiber.h"


class Single;
class TubuleProp;


/// Adds dynamic instability (growth/shrinkage of the ends) to a Fiber
/**
 This code is outdated, and we discourage you to use it.
 @ingroup FiberGroup
 */
class Tubule : public Fiber
{
public:
    
    /// the Property of this object
    TubuleProp const* prop;
   
private:
    
    /// state of FiberEnd
    int        mtState[3];
    
    /// amount growth by the ends (in length-units) during the time-step
    real       mtGrowth[3];
        
    /// resets the values of member variables
    void       reset();
        
public:
        
    /// returns the index which should be used to get parameters
    int dynParamIndex(const FiberEnd which) const;
    
    //--------------------------------------------------------------------------
  
    /// constructor
    Tubule(TubuleProp const*);

    /// destructor
    virtual ~Tubule();
        
    //--------------------------------------------------------------------------
    
    /// return assembly/disassembly state of the end \a which
    int         dynamicState(FiberEnd which) const { return mtState[which]; }
    
    /// set state of FiberEnd \a which to \a new_state
    void        setDynamicState(FiberEnd which, int new_state);
    
    /// the amount of freshly assembled polymer during the last time step
    real        freshAssembly(FiberEnd which) const { return mtGrowth[which]; }
    
    //--------------------------------------------------------------------------
    
    /// the growth rate set for the specified end
    real        givenGrowthRate(FiberEnd which, int alt=0) const;
    
    /// the transition rate set for the specified end
    real        givenTransitionRate(FiberEnd which, int alt=0) const;
    
    
    /// calculates the growth rate, in units of um/s of added polymer length = speed.
    real        growthRate(real rate0, real rate1, real force) const;
    
    /// calculate the transition rate of a MT end, function of the parameter, growth and position
    real        transitionRate(FiberEnd which, int model) const;
    
    //--------------------------------------------------------------------------
    
    /// cut fiber at distance \a abs from MINUS_END
    Fiber *     severM(real abs);
    
    /// join 'other' at the PLUS_END of this fiber
    void        join(Fiber * fib);
    
    /// simulate dynamic instability at MINUS_END
    void        stepMinusEnd();
    
    /// simulate dynamic instability at PLUS_END
    void        stepPlusEnd();
    
    /// monte-carlo step
    void        step();
    
    //--------------------------------------------------------------------------
    
    /// write Tubule to OutputWrapper
    void        write(OutputWrapper&) const;

    /// read Tubule from InputWrapper
    void        read(InputWrapper&, Simul&);
    
};


#endif
