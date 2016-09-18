// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "sim.h"
#include "smath.h"
#include "assert_macro.h"
#include "tubule.h"
#include "tubule_prop.h"
#include "fiber_locus.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"
#include "space.h"
#include "picket.h"

extern Random RNG;

//========================================================================
//  - - - - - - - - - - - - - - CONSTRUCTORS - - - - - - - - - - - - - - -
//========================================================================

void Tubule::reset()
{
    
    mtState[ PLUS_END] = STATE_GREEN;
    mtState[MINUS_END] = STATE_WHITE;
    
    mtGrowth[ PLUS_END] = 0;
    mtGrowth[MINUS_END] = 0;
}


Tubule::Tubule(TubuleProp const* p) : Fiber(p), prop(p)
{
    reset();
}


Tubule::~Tubule()
{
    //std::cerr << "destroying " << reference() << "\n";
    prop = 0;
}


//------------------------------------------------------------------------------
#pragma mark -

void Tubule::setDynamicState(const FiberEnd which, const int state)
{
    assert_true( which==PLUS_END || which==MINUS_END );

    if ( state!=STATE_WHITE && state!=STATE_GREEN && state!=STATE_RED )
        throw InvalidParameter("fiber: invalid AssemblyState");
    
    if ( state != mtState[which] )
    {
        mtState[which] = state;
    }
}

int  Tubule::dynParamIndex(const FiberEnd which) const
{
    assert_true( mtState[which]==STATE_GREEN || mtState[which]==STATE_RED );
    
    if ( which==PLUS_END )
    {
        if ( mtState[which] == STATE_GREEN )
            return 0;
        else
            return 1;
    }
    else
    {
        assert_true( which==MINUS_END );
        if ( mtState[which] == STATE_GREEN )
            return 2;
        else
            return 3;
    }
}


real Tubule::givenGrowthRate(const FiberEnd which, const int alt) const
{
    if ( mtState[which] == STATE_WHITE ) 
        return 0;
    
    if ( alt )
        return prop->dynamic_speed2[ dynParamIndex(which) ];
    else
        return prop->dynamic_speed1[ dynParamIndex(which) ];
}

real Tubule::givenTransitionRate(const FiberEnd which, const int alt) const
{
    if ( mtState[which] == STATE_WHITE )
        return 0;
    
    if ( alt )
        return prop->dynamic_trans2[ dynParamIndex(which) ];
    else
        return prop->dynamic_trans1[ dynParamIndex(which) ];
}


/**
 growthRate() returns the average assembly rate, in um/s
 
 - assembly (rate>0) is proportional to monomer concentration
 and slowed down by opposing forces on the tip of the MT.
 
 - disassembly (rate<0) is unaffected by monomer concentration or force.
 */
real Tubule::growthRate(const real speed0, const real speed1, const real force) const
{
    if ( speed0 > 0 )
    {    
        // antagonistic force (projectedForceOnEnd < 0) decrease assembly exponentially
        if ( force < 0  &&  prop->growing_force < INFINITY )
            return speed0 * prop->free_polymer * exp(force/prop->growing_force) + speed1;
        else
            return speed0 * prop->free_polymer + speed1;
    }
    else
    {
        // disassembly is constant
        return speed0 + speed1;
    }
}



/**
 - model = 2:
 transitions depend on the MT-length,
 there is no rescue beyond L=13 microns, and no catastrophy at L=0
 the rates given for rescue/catastrophies are those at L=10 microns.
 Dogterom, Felix, Guet, Leibler, J. Cell Biol. 1996 (mitotic X.Eggs extracts)
 Typical rates for this option should be:  cata=0.03 /sec. resc=0.01 /sec.
 - model = 3:
 Transitions depend on the growth rate of the MT tip, itself reduced by
 antagonistic force. the correspondance is: 1/rate = a + b * growthSpeed
 cf. `Dynamic instability of MTs is regulated by force`
 M.Janson, M. de Dood, M. Dogterom. JCB 2003, Figure 2 C.

 */
real Tubule::transitionRate(FiberEnd which, const int model) const
{
    real growth = mtGrowth[which] / prop->time_step;
    real rate0 = givenTransitionRate(which);
    real rate1 = givenTransitionRate(which, 1);
    
    switch ( model )
    {
        case 0:
            // rate is zero: no transition occurs
            return 0;
        
        case 1:
            // transitions are independent of MT length 
            return rate0;
        
        case 2:
            if ( givenGrowthRate(which) > 0 )
                return rate0 * length() * 0.1;
            else {
                if ( length() < 13.0 )
                    return rate0 * (13.0-length()) * 0.333;
                else
                    return 0;
            }
        break;
        
        
        case 3: {
            // 1 / catastrophe_rate depends linearly on growing speed
            real grow0 = givenGrowthRate(which);
            real grow1 = givenGrowthRate(which, 1);
            real coef = 0;
            if ( rate0 > 0 )
                coef = ( rate1 / rate0 - 1.0 ) / ( grow0 + grow1 );
            return rate1 / ( 1 + coef * growth );
        }
        
        case 7:
            //Transitions are alternate if a Hand is attached
            if ( nbBindersNearEnd(0.1, which) )
                return rate1;
            else
                return rate0;
            
        case 9:
            /** the position of the plus-end in the Space determines its dynamics:
            transitions are trans1[] inside the box and trans2[] outside
            */
            assert_true( prop->confine_space_ptr );
            if ( prop->confine_space_ptr->outside(posEnd(which)) )
                return rate1;
            else
                return rate0;
            
            
        default: {
            throw InvalidParameter("invalid value tubule:model=", model);
        }
    }
    return 0;
}


//------------------------------------------------------------------------------
#pragma mark -

void Tubule::stepPlusEnd()
{
    const FiberEnd which = PLUS_END;
    
    if ( mtState[which] != STATE_WHITE )
    {    
        // get force acting on the point at the end:
        real force = projectedForceOnEnd(which);
        
        mtGrowth[which] = prop->time_step * growthRate(givenGrowthRate(which), givenGrowthRate(which, 1), force);
        
        //switch to the other state with the probability given by transitionRate():
        real rate = transitionRate(which, prop->dynamic_model[0]);
        
        if ( RNG.test( rate * prop->time_step ))
            setDynamicState(which, (mtState[which]==STATE_GREEN) ? STATE_RED : STATE_GREEN);
    }
}


void Tubule::stepMinusEnd()
{
    const FiberEnd which = MINUS_END;
    
    if ( mtState[which] != STATE_WHITE )
    {    
        // get force acting on the point at the end:
        real force = projectedForceOnEnd(which);
        
        mtGrowth[which] = prop->time_step * growthRate(givenGrowthRate(which), givenGrowthRate(which, 1), force);
        
        //switch to the other state with the probability given by transitionRate():
        real rate = transitionRate(which, prop->dynamic_model[1]);
        
        if ( RNG.test( rate * prop->time_step ))
            setDynamicState(which, (mtState[which]==STATE_GREEN) ? STATE_RED : STATE_GREEN);
    }
}


void Tubule::step()
{    
    //we start by Fiber::step(), which may cut this fiber, but not destroy it!
    Fiber::step();

    
    if ( prop->dynamic_model[0] )
        stepPlusEnd();
    
    if ( prop->dynamic_model[1] )
        stepMinusEnd();
    
    
    if ( length() + mtGrowth[MINUS_END] + mtGrowth[PLUS_END] <= prop->min_length )
    {
        // do something if the fiber is too short:
        switch( prop->fate )
        {
            case FATE_NONE:
                break;
                
            case FATE_DESTROY:
                objset()->erase(this);
                // exit to avoid doing anything with the dead object:
                return;
                
            case FATE_RESCUE:
                if ( prop->dynamic_model[0] ) setDynamicState( PLUS_END, STATE_GREEN);
                if ( prop->dynamic_model[1] ) setDynamicState(MINUS_END, STATE_GREEN);
                break;
                
            default:
                throw InvalidParameter("tubule:fate should be in [0...3]");
        }
    }
    else
    {
        
        // grow/shrink if needed
        if ( mtGrowth[PLUS_END] )
            growP( mtGrowth[PLUS_END] );
        
        if ( mtGrowth[MINUS_END] )
            growM( mtGrowth[MINUS_END] );
        
        
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
 Calls Fiber::severM(abs)
 Set the dynamic state of newly created fiber tips:
 - PLUS_END to STATE_RED
 - MINUS_END to STATE_GREEN
 .
 */
Fiber * Tubule::severM(real abs)
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


void Tubule::join(Fiber * fib)
{
    assert_true( fib->prop == Fiber::prop );
    Fiber::join(fib);
    // if the prop are equal, the cast should be safe:
    setDynamicState(MINUS_END, fib->dynamicState(MINUS_END));
}
                  
                  
//------------------------------------------------------------------------------
#pragma mark -

void Tubule::write(OutputWrapper& out) const
{
    out.writeUInt8(mtState[MINUS_END]);
    out.writeUInt8(mtState[PLUS_END]);
    Fiber::write(out);
}


void Tubule::read(InputWrapper & in, Simul& sim)
{
    reset();

    try {
#ifdef BACKWARD_COMPATIBILITY
        if ( in.formatID() > 30 )
#endif
        {
            // current format
            mtState[MINUS_END] = in.readUInt8();
            mtState[PLUS_END]  = in.readUInt8();
        }
    }
    catch( Exception & e ) {
        e << ", while importing " << reference();
        throw;
    }
    
    Fiber::read(in, sim);
}

