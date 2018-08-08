// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <cmath>
#include "sim.h"
#include "messages.h"
#include "classic_fiber_prop.h"
#include "classic_fiber.h"
#include "property_list.h"
#include "simul_prop.h"
#include "exceptions.h"
#include "glossary.h"


Fiber* ClassicFiberProp::newFiber() const
{
    return new ClassicFiber(this);
}

//------------------------------------------------------------------------------
void ClassicFiberProp::clear()
{
    FiberProp::clear();
    
    growing_speed[0]    = 0;
    growing_speed[1]    = 0;
    growing_force       = INFINITY;
    shrinking_speed     = 0;
    catastrophe_rate[0] = 0;
    catastrophe_rate[1] = -1;
    rescue_rate         = 0;
    fate                = FATE_DESTROY;
    
#ifdef NEW_LENGTH_DEPENDENT_CATASTROPHE
    catastrophe_length  = 0;
#endif
}

//------------------------------------------------------------------------------
void ClassicFiberProp::read(Glossary& glos)
{
    FiberProp::read(glos);
    
    glos.set(growing_speed, 2,    "growing_speed");
    glos.set(growing_force,       "growing_force");
    glos.set(shrinking_speed,     "shrinking_speed");
    glos.set(catastrophe_rate, 2, "catastrophe_rate");
    glos.set(rescue_rate,         "rescue_rate");
    glos.set(fate,                "fate", KeyList<Fate>("destroy", FATE_DESTROY, "rescue", FATE_RESCUE, "none", FATE_NONE));
    
#ifdef NEW_LENGTH_DEPENDENT_CATASTROPHE
    glos.set(catastrophe_length,       "catastrophe_length");
#endif

#ifdef BACKWARD_COMPATIBILITY
    
    int d = 0;
    if ( glos.set(d, "delete_stub") && d )
        fate = FATE_DESTROY;

    if ( glos.set(growing_force, "dynamic_force") )
        Cytosim::warning("fiber:dynamic_force was renamed growing_force\n");
    
    if ( glos.set(fate, "dynamic_fate", KeyList<Fate>("destroy", FATE_DESTROY, "rescue", FATE_RESCUE, "none", FATE_NONE)) )
        Cytosim::warning("fiber:dynamic_fate was renamed fate\n");
    
    if ( glos.set(fate, "shrinking_fate", KeyList<Fate>("destroy", FATE_DESTROY, "rescue", FATE_RESCUE, "none", FATE_NONE)) )
        Cytosim::warning("fiber:shrinking_fate was renamed fate\n");
    
#endif
}


void ClassicFiberProp::complete(SimulProp const* sp, PropertyList* plist)
{
    FiberProp::complete(sp, plist);
    
    if ( growing_speed[0] < 0 ) 
        throw InvalidParameter("fiber:growing_speed[0] should be >= 0");
    
    if ( growing_force <= 0 )
        throw InvalidParameter("fiber:growing_force should be specified and > 0");

    if ( shrinking_speed > 0 ) 
        throw InvalidParameter("fiber:shrinking_speed should be <= 0");
    
    if ( catastrophe_rate[0] < 0 )
        throw InvalidParameter("fiber:catastrophe_rate should be >= 0");
    
    cata_coef = 0;
    if ( catastrophe_rate[1] <= 0 )
    {
        catastrophe_rate[1] = catastrophe_rate[0];
        Cytosim::warning("fiber:catastrophe_rate is independent of force (catastrophe_rate[1] unspecified)\n");
    }
    else if ( catastrophe_rate[0] == catastrophe_rate[1] )
    {
        Cytosim::warning("fiber:catastrophe_rate is independent of force (catastrophe_rate[0] == catastrophe_rate[1])\n");
    }
    else
    {
        if ( growing_speed[0]+growing_speed[1] <= 0 )
            cata_coef = 0;
        else
            cata_coef = ( catastrophe_rate[1]/catastrophe_rate[0] - 1.0 )
            / ( ( growing_speed[0] + growing_speed[1] ) * sp->time_step );
        
        if ( cata_coef < 0 )
            throw InvalidParameter("inconsistent fiber:dynamic parameters");
    }

    catastrophe_rate_dt = catastrophe_rate[1] * sp->time_step;
    shrinking_speed_dt  = shrinking_speed * sp->time_step;
    growing_speed_dt[0] = growing_speed[0] * sp->time_step;
    growing_speed_dt[1] = growing_speed[1] * sp->time_step;
    rescue_rate_prob    = 1 - exp( -rescue_rate * sp->time_step );
}

//------------------------------------------------------------------------------

void ClassicFiberProp::write_data(std::ostream & os) const
{
    FiberProp::write_data(os);
    
    write_param(os, "growing_speed",    growing_speed, 2);
    write_param(os, "growing_force",    growing_force);
    write_param(os, "shrinking_speed",  shrinking_speed);
    write_param(os, "catastrophe_rate", catastrophe_rate, 2);
    write_param(os, "rescue_rate",      rescue_rate);
    write_param(os, "fate",             fate);
#ifdef NEW_LENGTH_DEPENDENT_CATASTROPHE
    write_param(os, "catastrophe_length",       catastrophe_length);
#endif
}

