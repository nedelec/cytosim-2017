// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "nucleator.h"
#include "nucleator_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "fiber_prop.h"
#include "fiber_set.h"
#include "hand_monitor.h"

extern Random RNG;

//------------------------------------------------------------------------------

Nucleator::Nucleator(NucleatorProp const* p, HandMonitor* h)
: Hand(p,h), prop(p)
{
    gspTime = RNG.exponential();
}


//------------------------------------------------------------------------------

void Nucleator::nucleate(Vector pos)
{
    Glossary opt(prop->nucleation_spec);
    Fiber * fib = prop->nucleated_fiber_prop->newFiber(opt);
    assert_true(fib);
    
    // register the new Fiber:
    prop->nucleated_fiber_set->add(fib);
    
    // indicate the origin of nucleation:
    int mk = 0;
    if ( opt.set(mk, "mark") )
        fib->mark(mk);
    else
        fib->mark(haMonitor->objNumber());

    // the Fiber will be oriented depending on specificity:
    Rotation rot;
    
    switch( prop->specificity )
    {            
        case NucleatorProp::NUCLEATE_PARALLEL:
        {
            Vector dir = haMonitor->otherDirection(this);
            rot = Rotation::rotationToVector(dir, RNG);
        } break;
            
        case NucleatorProp::NUCLEATE_ANTIPARALLEL:
        {
            Vector dir = -haMonitor->otherDirection(this);
            rot = Rotation::rotationToVector(dir, RNG);
        } break;

        case NucleatorProp::NUCLEATE_PARALLEL_IF:
            if ( haMonitor->otherHand(this)->attached() )
            {
                Vector dir = haMonitor->otherDirection(this);
                rot = Rotation::rotationToVector(dir, RNG);
                fib->mark(1);
                // we read 'orientation' to avoid the 'unused' warning
                std::string str;
                opt.set(str, "orientation");
                break;
            }
            fib->mark(3);
            //there is an intentional fallback on the next case:
            
        case NucleatorProp::NUCLEATE_ORIENTATED:
        {
            std::string str;
            if ( opt.set(str, "orientation") )
            {
                std::istringstream iss(str);
                rot = Movable::readRotation(iss, pos, prop->nucleated_fiber_prop->confine_space_ptr);
            }
            else {
                rot = Rotation::randomRotation(RNG);
            }
        } break;

        default:
            throw InvalidParameter("unknown nucleator:specificity");
    }
    
    fib->rotate(rot);
    
    
    // shift position by the length of the interaction:
    if ( haMonitor->interactionLength() > 0 )
    {
        Vector dir = haMonitor->otherDirection(this);
        pos += dir.randPerp(haMonitor->interactionLength());
    }

    /*
     We translate Fiber to match the Nucleator's position,
     and if prop->track_end, the Hand is attached to the new fiber
     */
    if ( prop->track_end == PLUS_END )
    {
        attachToEnd(fib, PLUS_END);
        fib->translate(pos-fib->posEnd(PLUS_END));
    }
    else
    {
        if ( prop->track_end != NOT_END )
            attachToEnd(fib, MINUS_END);
        fib->translate(pos-fib->posEnd(MINUS_END));
    }
    
    if ( opt.warnings(std::cerr) )
    {
        std::cerr << "in hand:nucleation_spec" << std::endl;
        std::cerr << prop->nucleation_spec << std::endl;
    }
    //MSG("Nucleation at %.2fs,  X = %.2f\n", sim.simTime(), pos.XX);
}


//------------------------------------------------------------------------------
/**
 Does not attach nearby Fiber, but can nucleate
 */
void Nucleator::stepFree(const FiberGrid&, Vector const & pos)
{
    assert_true( !attached() );
    
    gspTime -= prop->nucleation_rate_dt;
    
    if ( gspTime < 0 )
    {
        gspTime = RNG.exponential();
        try {
            nucleate(pos);
        }
        catch( Exception & e )
        {
            e << "\nException occured while executing nucleator:code";
            throw;
        }
    }
}



void Nucleator::stepUnloaded()
{
    assert_true( attached() );
    
    if ( testDetachment() )
        return;
    
    // may track the end of the Fiber:
    switch ( prop->track_end )
    {
        case NOT_END:
            break;
            
        case PLUS_END:
            moveToEnd(PLUS_END);
            return;
            
        case MINUS_END:
            moveToEnd(MINUS_END);
            return;
            
        case NEAREST_END:
            moveToEnd(nearestEnd());
            return;
            
        default:
            throw InvalidParameter("invalid hand:track_end");
    }
}



void Nucleator::stepLoaded(Vector const& force)
{
    assert_true( attached() );
    
    // detachment depends on force:
    if ( testKramersDetachment(force.norm()) )
        return;
    
    
    // may track the end of the Fiber:
    switch ( prop->track_end )
    {
        case NOT_END:
            break;
            
        case PLUS_END:
            moveToEnd(PLUS_END);
            return;
            
        case MINUS_END:
            moveToEnd(MINUS_END);
            return;
            
        case NEAREST_END:
            moveToEnd(nearestEnd());
            return;
            
        default:
            throw InvalidParameter("invalid hand:track_end");
    }
}


//------------------------------------------------------------------------------
/**
 If pro->addictive, this gives a poisonous goodbye-kiss to the fiber
 */
void Nucleator::detach()
{
    if ( prop->addictive )
        fiber()->setDynamicState(nearestEnd(), STATE_RED);
        
    Hand::detach();    
}



