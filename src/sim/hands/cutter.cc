// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "cutter.h"
#include "cutter_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"
extern Random RNG;

//------------------------------------------------------------------------------

Cutter::Cutter(CutterProp const* p, HandMonitor* h)
: Hand(p,h), prop(p)
{
}

void Cutter::cut()
{
    assert_true( attached() );

    /**
     Cutting the fiber can invalidate the FiberGrid used for attachment,
     and this becomes a problem if the Cutter is part of a Couple,
     because calls for attachments and actions are intermingled.
     */
    Fiber * fib = fiber();
    ObjectSet * set = fib->objset();
 	Fiber * sec = fib->Fiber::severM(abscissa()-fib->abscissaM());
    
    if ( sec )
    {
        assert_true( sec->prop == fib->prop );
        set->add(sec);

        // new MINUS_END set as desired
        sec->setDynamicState(MINUS_END, prop->new_end_state[1]);
        
        // old PLUS_END is transfered with the same state:
        sec->setDynamicState(PLUS_END, fib->dynamicState(PLUS_END));
        
        // new PLUS_END set as desired
        fib->setDynamicState(PLUS_END, prop->new_end_state[0]);
    }

    detach();
}

//------------------------------------------------------------------------------

void Cutter::stepUnloaded()
{
    assert_true( attached() );
    
    // test for detachment
    if ( testDetachment() )
        return;

    if ( RNG.test(prop->cutting_rate_prob) )
        cut();
}


void Cutter::stepLoaded(Vector const& force)
{
    assert_true( attached() );
    
    if ( testKramersDetachment(force.norm()) )
        return;
    
    if ( RNG.test(prop->cutting_rate_prob) )
        cut();
}



