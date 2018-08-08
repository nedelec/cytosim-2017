// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "nucleator.h"
#include "nucleator_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "fiber_prop.h"
#include "fiber_set.h"
#include "hand_monitor.h"
#include "simul.h"

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
    Glossary opt(prop->spec);
    
    ObjectList objs = prop->simul->fibers.newObjects("fiber", prop->fiber, opt);
    if ( objs.empty() )
        return;
    
    Fiber * fib = static_cast<Fiber*>(objs[0]);
    
    // register the new objects:
    prop->simul->add(objs);
    
    // indicate the origin of nucleation:
    int mk = 0;
    if ( opt.set(mk, "mark") )
        Simul::mark(objs, mk);
    else
        Simul::mark(objs, haMonitor->objNumber());
    
    // the Fiber will be oriented depending on specificity:
    Rotation rot;
    
    real ang = 0;
    if ( opt.set(ang, "nucleation_angle") )
    {
        Vector dir = haMonitor->otherDirection(this);
        rot = Rotation::rotationToVector(dir);
#if ( DIM == 2 )
        rot = rot * Rotation::rotationFromEulerAngles(RNG.sflip()*ang);
#elif ( DIM == 3 )
        rot = rot * Rotation::rotationAroundX(RNG.sreal()*M_PI) * Rotation::rotationAroundZ(ang);
#endif
    }
    else switch( prop->specificity )
    {
        case NucleatorProp::NUCLEATE_PARALLEL:
        {
            Vector dir = haMonitor->otherDirection(this);
            rot = Rotation::rotationToVector(dir, RNG);
        }
        break;
            
        case NucleatorProp::NUCLEATE_ANTIPARALLEL:
        {
            Vector dir = -haMonitor->otherDirection(this);
            rot = Rotation::rotationToVector(dir, RNG);
        }
        break;
            
        case NucleatorProp::NUCLEATE_ORIENTATED:
        {
            std::string str;
            if ( opt.set(str, "orientation") )
            {
                std::istringstream iss(str);
                rot = Movable::readRotation(iss, pos, fib->prop->confine_space_ptr);
            }
            else {
                rot = Rotation::randomRotation(RNG);
            }
        }
        break;
            
        default:
            throw InvalidParameter("unknown nucleator:specificity");
    }
    
    ObjectSet::rotateObjects(objs, rot);
    
    
    // shift position by the length of the interaction:
    if ( haMonitor->interactionLength() > 0 )
    {
        Vector dir = haMonitor->otherDirection(this);
        pos += dir.randPerp(haMonitor->interactionLength());
    }
    
    /*
     We translate Fiber to match the Nucleator's position,
     and if prop->hold_end, the Hand is attached to the new fiber
     */
    if ( prop->track_end == MINUS_END )
    {
        attachToEnd(fib, MINUS_END);
        ObjectSet::translateObjects(objs, pos-fib->posEnd(MINUS_END));
    }
    else if ( prop->track_end == PLUS_END )
    {
        attachToEnd(fib, PLUS_END);
        ObjectSet::translateObjects(objs, pos-fib->posEnd(PLUS_END));
    }
    else
        ObjectSet::translateObjects(objs, pos-fib->position());
    
    // report unused options:
    if ( opt.warnings(std::cerr) )
    {
        std::cerr << "in nucleation:spec" << std::endl;
        std::cerr << prop->spec << std::endl;
    }
    //Cytosim::MSG("Nucleation at %.2fs,  X = %.2f\n", sim.simTime(), pos.XX);
}


//------------------------------------------------------------------------------
/**
 Does not attach nearby Fiber, but can nucleate
 */
void Nucleator::stepFree(const FiberGrid&, Vector const & pos)
{
    assert_true( !attached() );
    
    gspTime -= prop->rate_dt;
    
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



