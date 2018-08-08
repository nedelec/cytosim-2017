// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "smath.h"
#include "assert_macro.h"
#include "classic_fiber.h"
#include "classic_fiber_prop.h"
#include "fiber_locus.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"
#include "space.h"

extern Random RNG;

//------------------------------------------------------------------------------

ClassicFiber::ClassicFiber(ClassicFiberProp const* p) : Fiber(p), prop(p)
{
    mState = STATE_GREEN;
}

ClassicFiber::~ClassicFiber()
{
    prop = 0;
}


//------------------------------------------------------------------------------
#pragma mark -

int ClassicFiber::dynamicState(const FiberEnd which) const
{
    assert_true( which==PLUS_END || which==MINUS_END );

    if ( which == PLUS_END )
        return mState;
    else
        return 0;
}

void ClassicFiber::setDynamicState(FiberEnd which, const int state)
{
    assert_true( which==PLUS_END || which==MINUS_END );
    
    if ( state!=STATE_GREEN && state!=STATE_RED )
        throw InvalidParameter("fiber:classic invalid AssemblyState ", state);
    
    if ( which == PLUS_END )
    {
        if ( state==STATE_GREEN || state==STATE_RED )
            mState = (AssemblyState)state;
    }
}


real ClassicFiber::freshAssembly(const FiberEnd which) const
{
    assert_true( which==PLUS_END || which==MINUS_END );

    if ( which == PLUS_END )
        return mGrowth;
    else
        return 0;
}

//------------------------------------------------------------------------------
#pragma mark -

/** 
 The catastrophe rate depends on the growth rate of the corresponding tip,
 which is itself reduced by antagonistic force. 
 The correspondance is : 1/rate = a + b * growthSpeed.
 For no force on the growing tip: rate = catastrophe_rate[0]*dt
 For very large forces          : rate = catastrophe_rate[1]*dt
 cf. `Dynamic instability of MTs is regulated by force`
 M.Janson, M. de Dood, M. Dogterom. JCB 2003, Figure 2 C.
 */
void ClassicFiber::step()
{
    //we start by Fiber::step(), which may cut this fiber, but not destroy it!
    Fiber::step();
    

    if ( mState == STATE_GREEN )
    {
        // calculate the force acting on the point at the end:
        real force = projectedForceOnEnd(PLUS_END);
        
        // growth is reduced if free monomers are scarce:
        real spd = prop->growing_speed_dt[0] * prop->free_polymer;
        
        // antagonistic force (< 0) decreases assembly rate exponentially
        if ( force < 0  &&  prop->growing_force < INFINITY )
            mGrowth = spd * exp(force/prop->growing_force) + prop->growing_speed_dt[1];
        else
            mGrowth = spd + prop->growing_speed_dt[1];
        
        // grow at PLUS_END
        growP(mGrowth);
        
        // 1 / catastrophe_rate depends linearly on growing speed
        real cata = prop->catastrophe_rate_dt / ( 1 + prop->cata_coef * mGrowth );

        
#ifdef NEW_LENGTH_DEPENDENT_CATASTROPHE
        /*
         Ad-hoc length dependence, used to simulate S. pombe with catastrophe_length=5
         Foethke et al. MSB 5:241 - 2009
         */
        if ( prop->catastrophe_length > 0 )
        {
            MSG_ONCE("Using ad-hoc length-dependent catastrophe rate\n");
            cata *= length() / prop->catastrophe_length;
        }
#endif
        
        if ( RNG.test(cata) )
            mState = STATE_RED;
        
    }
    else if ( mState == STATE_RED )
    {
        mGrowth = prop->shrinking_speed_dt;
        
        if ( length() + mGrowth <= prop->min_length )
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
            // shrink at PLUS_END ( shrinking_speed < 0 )
            growP(mGrowth);
        }
        
        if ( RNG.test(prop->rescue_rate_prob) )
            mState = STATE_GREEN;
    }
    
    /*
     FiberNaked::adjustSegmentation and Fiber::updateBinder
     should be called every time as needed from growP or growM,
     but it is more efficient to call them here once per time-step.
     */
    adjustSegmentation();
    updateBinders();
}



//------------------------------------------------------------------------------

/**
 Calls Fiber::severM(abs)
 Set the dynamic state of newly created fiber tips:
 - PLUS_END to STATE_RED
 - MINUS_END to STATE_GREEN
 .
 */
Fiber * ClassicFiber::severM(real abs)
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


//------------------------------------------------------------------------------
#pragma mark -


void ClassicFiber::write(OutputWrapper& out) const
{
    if ( mState!=STATE_GREEN && mState!=STATE_RED )
        throw InvalidParameter("fiber:classic invalid AssemblyState ", mState);
    
    out.writeUInt8(mState);
    Fiber::write(out);
}


void ClassicFiber::read(InputWrapper & in, Simul& sim)
{
    try {
        setDynamicState(PLUS_END, in.readUInt8());
    }
    catch( Exception & e ) {
        e << ", while importing " << reference();
        throw;
    }
    
    Fiber::read(in, sim);
}

