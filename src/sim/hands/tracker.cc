// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "tracker.h"
#include "tracker_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"
extern Random RNG;


//------------------------------------------------------------------------------

Tracker::Tracker(TrackerProp const* p, HandMonitor* h)
: Hand(p, h), prop(p)
{
}
//------------------------------------------------------------------------------
bool Tracker::attachmentAllowed(FiberBinder & fb)
{
    assert_true( !attached() && !linked() );
    assert_true( fb.attached() );
    
    
    /*
     Check that the two binding keys match:
     Allow binding if the BITWISE-AND of the two keys is true
     */
    if ( ! ( prop->binding_key & fb.fiber()->prop->binding_key ) )
        return false;

    
    FiberEnd end = NOT_END;
    
    // check minus-end:
    if ( fb.abscissaFrom(MINUS_END) <= prop->bind_end_range )
    {
        if ( prop->bind_only_growing_end && !fb.fiber()->isGrowing(MINUS_END) )
            return false;
        
        if ( prop->bind_end==MINUS_END  ||  prop->bind_end==NEAREST_END )
        {
            if ( fb.abscissaFrom(MINUS_END) <= 0 )
                fb.moveToEnd(MINUS_END);
            end = MINUS_END;
        }
        else
            return false;
    }
    
    // check plus-end:
    if ( fb.abscissaFrom(PLUS_END) <= prop->bind_end_range )
    {
        if ( prop->bind_only_growing_end && !fb.fiber()->isGrowing(PLUS_END) )
            return false;
        
        if ( prop->bind_end==PLUS_END  ||  prop->bind_end==NEAREST_END )
        {
            if ( fb.abscissaFrom(PLUS_END) <= 0 )
                fb.moveToEnd(PLUS_END);
            end = PLUS_END;
        }
        else
            return false;
    }
    
    
    // only allow binding to end if bind_end is true
    if ( end == NOT_END  &&  prop->bind_end )
        return false;
    
    
    // check occupancy:
    if ( prop->bind_only_free_end &&
        0 < fb.fiber()->nbBindersInRange(0, prop->bind_end_range, end))
        return false;
    
    
    // allowAttachment(fb) will return false if the Hand cannot bind
    return haMonitor->allowAttachment(fb);
}

//------------------------------------------------------------------------------
#pragma mark -


void Tracker::stepUnloaded()
{
    assert_true( attached() );
    
    // detachment
    if ( testDetachment() )
        return;

    
    switch ( prop->track_end )
    {
        case NOT_END:
            break;
            
        case PLUS_END:
            moveToEnd(PLUS_END);
            break;
            
        case MINUS_END:
            moveToEnd(MINUS_END);
            break;
            
        case NEAREST_END:
            moveToEnd(nearestEnd());
            break;
            
        default:
            throw InvalidParameter("invalid value of tracker:track_end");
    }
}



void Tracker::stepLoaded(Vector const& force)
{
    assert_true( attached() );
    
    if ( testKramersDetachment(force.norm()) )
        return;

    switch ( prop->track_end )
    {
        case NOT_END:
            break;
            
        case PLUS_END:
            moveToEnd(PLUS_END);
            break;
            
        case MINUS_END:
            moveToEnd(MINUS_END);
            break;
            
        case NEAREST_END:
            moveToEnd(nearestEnd());
            break;
            
        default:
            throw InvalidParameter("invalid value of tracker:track_end");
    }
}


