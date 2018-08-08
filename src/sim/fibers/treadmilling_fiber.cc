// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "smath.h"
#include "assert_macro.h"
#include "treadmilling_fiber.h"
#include "treadmilling_fiber_prop.h"
#include "fiber_locus.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"
#include "space.h"

extern Random RNG;

//------------------------------------------------------------------------------

TreadmillingFiber::TreadmillingFiber(TreadmillingFiberProp const* p) : Fiber(p), prop(p)
{
    stateM   = STATE_WHITE;
    mGrowthM = 0;
    
    stateP   = STATE_WHITE;
    mGrowthP = 0;
}


TreadmillingFiber::~TreadmillingFiber()
{
    prop = 0;
}


//------------------------------------------------------------------------------
#pragma mark -

int TreadmillingFiber::dynamicState(const FiberEnd which) const
{
    assert_true( which==PLUS_END || which==MINUS_END );
    
    if ( which == PLUS_END )
        return stateP;
    else if ( which == MINUS_END )
        return stateM;
    
    return 0;
}


void TreadmillingFiber::setDynamicState(const FiberEnd which, const int state)
{
    assert_true( which==PLUS_END || which==MINUS_END );

    if ( state!=STATE_GREEN && state!=STATE_RED && state!=STATE_WHITE )
        throw InvalidParameter("fiber:treadmilling invalid AssemblyState");
    
    if ( which == PLUS_END )
        stateP = state;
    else if ( which == MINUS_END )
        stateM = state;
}


real TreadmillingFiber::freshAssembly(const FiberEnd which) const
{
    assert_true( which==PLUS_END || which==MINUS_END );

    if ( which == PLUS_END )
        return mGrowthP;
    else
        return mGrowthM;
}

//------------------------------------------------------------------------------

void TreadmillingFiber::step()
{    
    //we start by Fiber::step(), which may cut this fiber, but not destroy it!
    Fiber::step();


    if ( stateP == STATE_GREEN )
    {
        // calculate the force acting on the point at the end:
        real forceP = projectedForceOnEnd(PLUS_END);
        
        // growth is reduced if free monomers are scarce:
        mGrowthP = prop->growing_speed_dt[0] * prop->free_polymer;
        
        // antagonistic force (< 0) decreases assembly rate exponentially
        if ( forceP < 0  &&  prop->growing_force[0] < INFINITY )
            mGrowthP *= exp(forceP/prop->growing_force[0]);
    }
    else if ( stateP == STATE_RED )
    {
        mGrowthP = prop->shrinking_speed_dt[0];
    }
    else
    {
        mGrowthP = 0;
    }
    
    
    if ( stateM == STATE_GREEN )
    {
        // calculate the force acting on the point at the end:
        real forceM = projectedForceOnEnd(MINUS_END);
        
        // growth is reduced if free monomers are scarce:
        mGrowthM = prop->growing_speed_dt[1] * prop->free_polymer;
        
        // antagonistic force (< 0) decreases assembly rate exponentially
        if ( forceM < 0 )
            mGrowthM *= exp(forceM/prop->growing_force[1]);
    }
    else if ( stateM == STATE_RED )
    {
        mGrowthM = prop->shrinking_speed_dt[1];
    }
    else
    {
        mGrowthM = 0;
    }
    
    
    if ( length() + mGrowthP + mGrowthM < prop->min_length )
    {
        // the fiber is too short, we delete it:
        objset()->erase(this);
    }
    else
    {
        growM(mGrowthM);
        growP(mGrowthP);
        
        /*
         FiberNaked::adjustSegmentation and Fiber::updateBinder
         should be called every time as needed from growP or growM,
         but it is more efficient to call them here once per time-step.
         */
        adjustSegmentation();
        updateBinders();
    }
}



//------------------------------------------------------------------------------
#pragma mark -

/**
 Calls Fiber::severM(abs),
 Set the dynamic state of newly created fiber tips:
 - PLUS_END to STATE_RED
 - MINUS_END to STATE_GREEN
 .
 */
Fiber * TreadmillingFiber::severM(real abs)
{
    // the new part 'fib' will have the PLUS_END section
    Fiber* fib = Fiber::severM(abs);
    
    if ( fib )
    {
        assert_true( fib->prop == prop );
        
        // new MINUS_END is stable
        fib->setDynamicState(MINUS_END, STATE_GREEN);
        
        // old PLUS_END is transfered with the same state:
        fib->setDynamicState(PLUS_END, dynamicState(PLUS_END));
        
        // new PLUS_END is unstable (shrinking state)
        setDynamicState(PLUS_END, STATE_RED);
    }
    
    return fib;
}


void TreadmillingFiber::join(Fiber * fib)
{
    assert_true( fib->prop == Fiber::prop );
    Fiber::join(fib);
    //transfer dynamic state of MINUS_END:
    setDynamicState(MINUS_END, fib->dynamicState(MINUS_END));
}
                  
//------------------------------------------------------------------------------
#pragma mark -


void TreadmillingFiber::write(OutputWrapper& out) const
{
    out.writeUInt16(stateM);
    out.writeUInt16(stateP);
    Fiber::write(out);
}


void TreadmillingFiber::read(InputWrapper & in, Simul& sim)
{
    try {
        stateM = in.readUInt16();
        stateP = in.readUInt16();
    }
    catch( Exception & e ) {
        e << ", while importing " << reference();
        throw;
    }
    
    Fiber::read(in, sim);
}

