// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "hand_prop.h"
#include "smath.h"
#include "messages.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "sim.h"
#include "property_list.h"
#include "simul_prop.h"
#include "hand.h"
#include "hand_monitor.h"


///maximum binding range of all Hands
real HandProp::binding_range_max = 0;


//------------------------------------------------------------------------------

#include "hand_prop.h"
#include "motor_prop.h"
#include "nucleator_prop.h"
#include "slider_prop.h"
#include "tracker_prop.h"
#include "rescuer_prop.h"
#include "cutter_prop.h"
#include "mighty_prop.h"
#include "actor_prop.h"

//------------------------------------------------------------------------------
/**
 @defgroup HandGroup Hand and Derived Activities
 @ingroup ObjectGroup
 @ingroup NewObject
 @brief A Hand can bind to a Fiber, and derived class can do more things.

 A plain Hand can only bind and unbind from a Fiber.
 Derived classes are available that implement more complex functionalities,
 for example molecular motors or severing enzymes.
 
 List of classes accessible by specifying `hand:activity`:

 `activity`    |   Class       | Parameters
 --------------|---------------|-----------------------------------
 `bind`        | Hand          | @ref HandPar (default)
 `move`        | Motor         | @ref MotorPar
 `nucleate`    | Nucleator     | @ref NucleatorPar
 `slide`       | Slider        | @ref SliderPar
 `track`       | Tracker       | @ref TrackerPar
 `rescue`      | Rescuer       | @ref RescuerPar
 `cut`         | Cutter        | @ref CutterPar
 `mighty`      | Mighty        | @ref MightyPar
 `act`         | Actor         | @ref ActorPar
 
 Example:
 @code
 set hand motor
 {
   binding = 10, 0.05
   unbinding = 0.2, 3
 
   activity = move
   max_speed = 1
   stall_force = 5
 } 
 @endcode
 */
HandProp * HandProp::newProperty(const std::string& name, Glossary& glos)
{
    HandProp * hp = 0;
    
    std::string a;
    if ( glos.peek(a, "activity") )
    {
        if ( a == "move" || a == "motor" )
            hp = new MotorProp(name);
        else if ( a == "nucleate" )
            hp = new NucleatorProp(name);
        else if ( a == "slide" )
            hp = new SliderProp(name);
        else if ( a == "track" )
            hp = new TrackerProp(name);
        else if ( a == "rescue" )
            hp = new RescuerProp(name);
        else if ( a == "cut" )
            hp = new CutterProp(name);
        else if ( a == "mighty" )
            hp = new MightyProp(name);
        else if ( a == "act" )
            hp = new ActorProp(name);
        else if ( a == "bind" )
            hp = new HandProp(name);
        else
            throw InvalidParameter("unknown hand:activity `"+a+"'");
    }
    else
        hp = new HandProp(name);
    
    return hp;
}




//------------------------------------------------------------------------------

Hand * HandProp::newHand(HandMonitor* h) const
{
    return new Hand(this, h);
}

//------------------------------------------------------------------------------
void HandProp::clear()
{
    binding_rate       = 0;
    binding_range      = 0;
    binding_key        = (~0);  //all bits at 1
    
    unbinding_rate     = 0;
#if NEW_END_DEPENDENT_DETACHMENT
    unbinding_rate_end = 0;
#endif
    unbinding_force    = 0;
    unbinding_force_inv = 0;

    bind_also_ends     = false;
    hold_growing_end   = false;
    hold_shrinking_end = false;
    
    activity           = "bind";
    display            = "";
}

//------------------------------------------------------------------------------
void HandProp::read(Glossary& glos)
{
    glos.set(binding_rate,       "binding_rate");
    glos.set(binding_range,      "binding_range");
    glos.set(binding_key,        "binding_key");
    //alternative syntax:
    glos.set(binding_rate,       "binding", 0);
    glos.set(binding_range,      "binding", 1);
    glos.set(binding_key,        "binding", 2);
    
    glos.set(unbinding_rate,     "unbinding_rate");
    glos.set(unbinding_force,    "unbinding_force");
    //alternative syntax:
    glos.set(unbinding_rate,     "unbinding", 0);
#if NEW_END_DEPENDENT_DETACHMENT
    glos.set(unbinding_rate_end, "unbinding_rate_end");
#endif
    glos.set(unbinding_force,    "unbinding", 1);
    
    glos.set(bind_also_ends, "bind_also_ends") || glos.set(bind_also_ends, "bind_also_end");

    glos.set(hold_growing_end,   "hold_growing_end");
    glos.set(hold_shrinking_end, "hold_shrinking_end");
    
    glos.set(activity,           "activity");
    glos.set(display,            "display");
    
#ifdef BACKWARD_COMPATIBILITY
    if ( glos.set(hold_growing_end, "hold_growing_ends") )
        Cytosim::warning("hand:hold_growing_ends was renamed hold_growing_end\n");
#endif
}

//------------------------------------------------------------------------------
void HandProp::complete(SimulProp const* sp, PropertyList*)
{    
    if ( sp==0 || sp->time_step < REAL_EPSILON )
        throw InvalidParameter("simul:time_step is not defined");
    
    if ( binding_range_max < binding_range )
        binding_range_max = binding_range;
    
    binding_range_sqr = binding_range * binding_range;
    binding_rate_dt   = binding_rate * sp->time_step;
    unbinding_rate_dt = unbinding_rate * sp->time_step;
#if NEW_END_DEPENDENT_DETACHMENT
    if ( unbinding_rate_end > 0 )
        unbinding_rate_end_dt = unbinding_rate_end * sp->time_step;
    else
        unbinding_rate_end_dt = unbinding_rate * sp->time_step;
#endif
    
    if ( binding_range < 0 )
        throw InvalidParameter("hand:binding_range must be >= 0");
    
    if ( binding_rate < 0 )
        throw InvalidParameter("hand:binding_rate must be positive");
    if ( binding_rate_dt > sp->acceptable_rate )
        Cytosim::warning("hand:binding_rate is too high: decrease time_step\n");
    
    if ( unbinding_rate < 0 )
        throw InvalidParameter("hand:unbinding_rate must be positive");
    if ( unbinding_rate_dt > sp->acceptable_rate )
        Cytosim::warning("hand:unbinding_rate is too high: decrease time_step\n");
    
    if ( unbinding_force < 0 )
        throw InvalidParameter("hand:unbinding_force must be specified and > 0");
    
    // if ( unbinding_force == 0 ), there will be no force-dependence:
    if ( unbinding_force > 0 )
        unbinding_force_inv = 1.0 / unbinding_force;
    else
        unbinding_force_inv = 0;
}



/**
 Compare the energy in a link when it binds at its maximum distance,
 with the Thermal energy
 
 @todo the warning may not be relevant for long Links
 */
void HandProp::checkStiffness(real stiff, real len, real mul, real kT) const
{
    real dis = binding_range - len;
    real en = ( stiff * dis * dis ) / kT;
    
    if ( en > 10.0 )
    {
        std::ostringstream oss;
        oss << "hand `" << name() << "' overcomes high energy when binding:" << std::endl;
        oss << PREF << "stiffness * binding_range^2 = " << en << " kT" << std::endl;
        //oss << PREF << "you could decrease stiffness or binding_range" << std::endl;
        Cytosim::warning(oss.str().c_str());
    }
    
    
    real ap = exp( stiff * dis * unbinding_force_inv );
    
    if ( ap > 10.0 )
    {
        std::ostringstream oss;
        oss << "hand `" << name() << "' may unbind immediately after binding:" << std::endl;
        oss << PREF << "exp( stiffness * binding_range / unbinding_force ) = " << ap << std::endl;
        //oss << PREF << "you could decrease stiffness * binding_range / unbinding_force" << std::endl;
        Cytosim::warning(oss.str().c_str());
    }
}

//------------------------------------------------------------------------------

void HandProp::write_data(std::ostream & os) const
{
    write_param(os, "binding",            binding_rate, binding_range);
    write_param(os, "binding_key",        binding_key);
    write_param(os, "unbinding",          unbinding_rate, unbinding_force);
#if NEW_END_DEPENDENT_DETACHMENT
    write_param(os, "unbinding_rate_end", unbinding_rate_end);
#endif
    write_param(os, "bind_also_ends",     bind_also_ends);
    write_param(os, "hold_growing_end",   hold_growing_end);
    write_param(os, "hold_shrinking_end", hold_shrinking_end);

    write_param(os, "display",            "("+display+")");
    write_param(os, "activity",           activity);
}

