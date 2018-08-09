// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "hand.h"
#include "hand_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "fiber_prop.h"
#include "simul.h"
#include "sim.h"
extern Random RNG;

//------------------------------------------------------------------------------

Hand::Hand(HandProp const* p, HandMonitor* m) : haMonitor(m), prop(p)
{
    nextAttach = RNG.exponential();
    nextDetach = RNG.exponential();
}

Hand::~Hand()
{
    if ( attached() )
        detach();
    prop = 0;
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 Tests if attachment at site \a fb is possible:
 - site is located within the MINUS_END and PLUS_END
 - binding_key are bitwise compatible: prop->binding_key & fb.fiber()->prop->binding_key 
 - HandMonitor::allowAttachment() returns true
 */
bool Hand::attachmentAllowed(FiberBinder & fb)
{
    assert_true( fb.attached() );    
    
    /*
     Check that the two binding keys match:
     Allow binding if the BITWISE-AND of the two keys is true
     */
    if ( ! ( prop->binding_key & fb.fiber()->prop->binding_key ) )
        return false;

    
    // check minus-end-binding:
    if ( fb.abscissaFrom(MINUS_END) < 0 )
    {
        if ( prop->bind_also_ends )
            fb.moveToEnd(MINUS_END);
        else
            return false;
    }
    
    // check plus-end binding:
    if ( fb.abscissaFrom(PLUS_END) < 0 )
    {
        if ( prop->bind_also_ends )
            fb.moveToEnd(PLUS_END);
        else
            return false;
    }
 
    
    // allowAttachment(fb) will return false if the Hand cannot bind
    return haMonitor->allowAttachment(fb);
}


/**
 This also sets the Gillespie detachment time \a nextDetach
 */
void Hand::attach(FiberBinder const & fb)
{
    assert_true( !attached() && !linked() );
    assert_true( fb.attached() );    

    FiberBinder::attach(fb);
    haMonitor->afterAttachment();
}


void Hand::attachTo(Fiber * f, const real ab, const FiberEnd from)
{
    assert_true(f);
    FiberBinder fb(f, f->abscissa(ab, from));
    attach(fb);
}


void Hand::attachToEnd(Fiber * f, const FiberEnd end)
{
    assert_true(f);
    FiberBinder fb(f, f->abscissa(end));
    attach(fb);
}


void Hand::handleOutOfRange(FiberEnd end)
{
    assert_true( attached() );
    assert_true( fbAbs < fbFiber->abscissaM() ||  fbAbs > fbFiber->abscissaP() );
    
    if ( fiber()->isShrinking(end) )
    {
        if ( prop->hold_shrinking_end )
            moveToEnd(end);
        else
            detach();
    }
    else
    {
        // if the FiberEnd is not shrinking, it can be growing or static:
        if ( prop->hold_growing_end )
            moveToEnd(end);
        else
            detach();
    }
}


//------------------------------------------------------------------------------
#pragma mark -

void Hand::detach()
{
    assert_true( attached() );
    
    haMonitor->beforeDetachment();
    FiberBinder::detach();
    haMonitor->afterDetachment();
}

//------------------------------------------------------------------------------
/** 
 Test for spontaneous detachment using Gillespie approach.
 
 @return true if the test has passed, and detach() was called.
 
 see @ref Stochastic
 */
bool Hand::testDetachment()
{
    assert_true( nextDetach >= 0 );
    /* 
     Attention: nextDetach should be set at each attachement.
     */
    
#if NEW_END_DEPENDENT_DETACHMENT
    // Hands within 10nm can hold onto the plus end
    if ( abscissaFrom(PLUS_END) < 0.010 )
        nextDetach -= prop->unbinding_rate_end_dt;
    else
#endif
        
    nextDetach -= prop->unbinding_rate_dt;
    
    if ( nextDetach <= 0 )
    {
        nextDetach = RNG.exponential();
        detach();
        return true;
    }
    
    return false;
}


/**
 Test for spontaneous detachment using Gillespie approach.
 
 @return true if the test has passed, and detach() was called.
 
 see @ref Stochastic
 */
bool Hand::testKramersDetachment(const real force)
{
    /*
     mul can be infinite, because it is the exponential of force,
     and we avoid the case ( rate==0 ) since zero * infinite is undefined
     */
    real rate = prop->unbinding_rate_dt;
    
#if NEW_END_DEPENDENT_DETACHMENT
    // Hands within 10nm can hold onto the plus end
    if ( abscissaFrom(PLUS_END) < 0.010 )
        rate = prop->unbinding_rate_end_dt;
#endif

    
    if ( rate > 0 )
    {
        nextDetach -= rate * exp(force*prop->unbinding_force_inv);
        if ( nextDetach <= 0 )
        {
            nextDetach = RNG.exponential();
            detach();
            return true;
        }
    }
    return false;
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 Test for attachment to nearby Fibers, using the Gillespie time nextAttach
 */
void Hand::stepFree(const FiberGrid& grid, Vector const & pos)
{
    assert_true( !attached() );
    assert_true( nextAttach >= 0 );

    nextAttach -= prop->binding_rate_dt;

    if ( nextAttach <= 0 )
    {
        nextAttach = RNG.exponential();
        if ( grid.tryToAttach(pos, *this) )
            return;
    }
}


//------------------------------------------------------------------------------
/**
 Test for spontaneous detachment at rate HandProp::unbinding_rate, 
 */
void Hand::stepUnloaded()
{
    assert_true( attached() );
    
    testDetachment();
}

//------------------------------------------------------------------------------
/**
 Test for force-induced detachment following Kramers' law,
 vith basal rate HandProp::unbinding_rate, 
 and characteristic force HandProp::unbinding_force
 */
void Hand::stepLoaded(Vector const& force)
{
    assert_true( attached() );
    
    testKramersDetachment(force.norm());
}



//------------------------------------------------------------------------------
#pragma mark -

void Hand::write(OutputWrapper& out) const
{
    /*
     it is not necessary to write the property index here,
     since it is set only when Hand is created in class Single or Couple.
     */
    FiberBinder::write(out);
}


void Hand::read(InputWrapper & in, Simul& sim)
{
#ifdef BACKWARD_COMPATIBILITY
    if ( in.formatID() < 32 )
        prop = static_cast<HandProp*>(sim.properties.find("hand",in.readUInt16()));
#endif
    
    FiberBinder::read(in, sim);
}
