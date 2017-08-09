// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "smath.h"
#include "assert_macro.h"
#include "dynamic_fiber.h"
#include "dynamic_fiber_prop.h"
#include "fiber_locus.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"
#include "space.h"

extern Random RNG;

//------------------------------------------------------------------------------

DynamicFiber::DynamicFiber(DynamicFiberProp const* p) : Fiber(p), prop(p)
{
    unitM[0] = 1;
    unitM[1] = 1;
    stateM   = calculateStateM();
    mGrowthM = 0;
    
    unitP[0] = 1;
    unitP[1] = 1;
    stateP   = calculateStateP();
    mGrowthP = 0;
    
    nextGrowthP = RNG.exponential();
    nextHydrolP = RNG.exponential();

    nextGrowthM = RNG.exponential();
    nextHydrolM = RNG.exponential();
}


DynamicFiber::~DynamicFiber()
{
    prop = 0;
}


//------------------------------------------------------------------------------
/** 
 The microscopic state correspond to:
 - STATE_GREEN for growth,
 - STATE_RED for shrinkage
 .
 */
int DynamicFiber::calculateStateP() const
{
    return 4 - unitP[0] - 2 * unitP[1];
}


int DynamicFiber::calculateStateM() const
{ 
    return 4 - unitM[0] - 2 * unitM[1];
}


int DynamicFiber::dynamicState(const FiberEnd which) const
{
    assert_true( which==PLUS_END || which==MINUS_END );
    
    if ( which == PLUS_END )
    {
        assert_true( stateP == calculateStateP() );
        return stateP;
    }
    else if ( which == MINUS_END )
    {
        return STATE_WHITE; //unfinished: MINUS_END is not dynamic
        assert_true( stateM == calculateStateM() );
        return stateM;
    }
    
    return 0;
}


void DynamicFiber::setDynamicState(const FiberEnd which, const int state)
{
    assert_true( which==PLUS_END || which==MINUS_END );

    if ( state < 1 || 4 < state )
        throw InvalidParameter("fiber:dynamic invalid AssemblyState");
    
    if ( which == PLUS_END )
    {
        stateP   = state;
        unitP[1] = ( 4 - state ) / 2;
        unitP[0] = ( 4 - state - 2*unitP[1] );
        assert_true( 0==unitP[0] || unitP[0]==1 );
        assert_true( 0==unitP[1] || unitP[1]==1 );
        assert_true( stateP == calculateStateP() );
    }
    else if ( which == MINUS_END )
    {
        stateM   = state;
        unitM[1] = ( 4 - state ) / 2;
        unitM[0] = ( 4 - state - 2*unitM[1] );
        assert_true( 0==unitP[0] || unitP[0]==1 );
        assert_true( 0==unitP[1] || unitP[1]==1 );
        assert_true( stateM == calculateStateM() );
    }
}

real DynamicFiber::freshAssembly(const FiberEnd which) const
{
    assert_true( which==PLUS_END || which==MINUS_END );

    if ( which == PLUS_END )
        return mGrowthP;
    else
        return mGrowthM;
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 Using a modified Gillespie scheme with a variable rate.
 
 returns the number of units added (if result > 0) or removed (if < 0)
 */
int DynamicFiber::stepPlusEnd(real growth_rate_dt)
{
    int res = 0;
    
    const real growthR = ( stateP==STATE_RED ? prop->shrinking_rate_dt : growth_rate_dt );
    
    const real hydrolR = prop->hydrolysis_rate_2dt;
    
    nextGrowthP -= growthR;
    nextHydrolP -= hydrolR;
    
    while ( nextGrowthP <= 0  ||  nextHydrolP <= 0 )
    {
        /*
         Select the earliest event. In units of time_step,
         - the next growth event is scheduled at time = nextGrowthP / growthR
         - the next hydrolysis event is scheduled at time = nextHydrolP / hydrolR
         */
        
        if ( nextGrowthP * hydrolR < nextHydrolP * growthR )
        {
            if ( stateP == STATE_RED )
            {
                // remove one unit
                --res;
            }
            else
            {
                // add fresh unit, shifting old terminal to penultimate position
                unitP[1] = unitP[0];
                unitP[0] = 1;
                stateP   = calculateStateP();
                ++res;
            }
            nextGrowthP += RNG.exponential();
        }
        else
        {
            //the two units have equal probability to hydrolyze:
            if ( RNG.flip() )
                unitP[0] = 0;
            else
                unitP[1] = 0;
            
            stateP = calculateStateP();
            nextHydrolP += RNG.exponential();
        }
    }
    return res;
}


int DynamicFiber::stepMinusEnd(real growth_rate_dt)
{
    ABORT_NOW("unfinished");
    return 0;
}


void DynamicFiber::step()
{
    //we start by Fiber::step(), which may cut this fiber, but not destroy it!
    Fiber::step();

    
    // calculate the force acting on the point at the end:
    real force = projectedForceOnEnd(PLUS_END);
    
    // growth is reduced if free monomers are scarce:
    real rate = prop->growing_rate_dt[0] * prop->free_polymer;

    // estimate growing rate in continuous approximation:
    real rateP;
    // antagonistic force (< 0) decreases assembly rate exponentially
    if ( force < 0  &&  prop->growing_force < INFINITY )
        rateP = rate * exp(force/prop->growing_force) + prop->growing_rate_dt[1];
    else
        rateP = rate + prop->growing_rate_dt[1];
    
    // perform stochastic simulation:
    int addP = stepPlusEnd(rateP);
    
    mGrowthP = addP * prop->unit_length;
    
    //int addM = stepMinusEnd(0);

    if ( addP )
    {        
        if ( length() + mGrowthP < prop->min_length )
        {
            // do something if the fiber is too short:
            switch ( prop->fate )
            {
                case FATE_NONE:
                    break;
                    
                case FATE_DESTROY:
                    objset()->erase(this);
                    // exit to avoid doing anything with a dead object:
                    return;
                    
                case FATE_RESCUE:
                    setDynamicState(PLUS_END, STATE_GREEN);
                    break;
            }
        }
        else
        {
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
}



//------------------------------------------------------------------------------
#pragma mark -

/**
 Calls Fiber::severM(abs)
 Set the dynamic state of newly created fiber tips:
 - PLUS_END to STATE_RED
 - MINUS_END to STATE_GREEN
 .
 */
Fiber * DynamicFiber::severM(real abs)
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


void DynamicFiber::join(Fiber * fib)
{
    assert_true( fib->prop == Fiber::prop );
    Fiber::join(fib);
    //transfer dynamic state of MINUS_END:
    setDynamicState(MINUS_END, fib->dynamicState(MINUS_END));
}
                  

//------------------------------------------------------------------------------
#pragma mark -


void DynamicFiber::write(OutputWrapper& out) const
{
    out.writeUInt8(unitM[0]);
    out.writeUInt8(unitM[1]);
    out.writeUInt8(unitP[0]);
    out.writeUInt8(unitP[1]);
    Fiber::write(out);
}


void DynamicFiber::read(InputWrapper & in, Simul& sim)
{
    try {
        unitM[0] = in.readUInt8();
        unitM[1] = in.readUInt8();
        stateM   = calculateStateM();

        unitP[0] = in.readUInt8();
        unitP[1] = in.readUInt8();
        stateP   = calculateStateP();
    }
    catch( Exception & e ) {
        e << ", while importing " << reference();
        throw;
    }
    
    Fiber::read(in, sim);
}

