// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <cmath>
#include "sim.h"
#include "dynamic_fiber_prop.h"
#include "dynamic_fiber.h"
#include "property_list.h"
#include "simul_prop.h"
#include "exceptions.h"
#include "glossary.h"
#include "messages.h"


Fiber* DynamicFiberProp::newFiber() const
{
    return new DynamicFiber(this);
}

//------------------------------------------------------------------------------
void DynamicFiberProp::clear()
{
    FiberProp::clear();
    
    unit_length      = 0.008;
    fate             = FATE_DESTROY;
    
    growing_speed[0] = 0;
    growing_speed[1] = 0;
    hydrolysis_rate  = 0;
    
    growing_force    = INFINITY;
    shrinking_speed  = 0;
}

//------------------------------------------------------------------------------
void DynamicFiberProp::read(Glossary& glos)
{
    FiberProp::read(glos);
    
    glos.set(unit_length,        "unit_length");
    glos.set(growing_speed, 2,   "growing_speed");
    glos.set(hydrolysis_rate,    "hydrolysis_rate");
    glos.set(growing_force,      "growing_force");
    glos.set(shrinking_speed,    "shrinking_speed");
    glos.set(fate,               "fate", KeyList<Fate>("destroy", FATE_DESTROY, "rescue", FATE_RESCUE, "none", FATE_NONE));

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


void DynamicFiberProp::complete(SimulProp const* sp, PropertyList* plist)
{
    FiberProp::complete(sp, plist);

    if ( growing_force <= 0 )
        throw InvalidParameter("fiber:growing_force should be specified and > 0");
    if ( growing_speed[0] < 0 )
        throw InvalidParameter("fiber:growing_speed should be >= 0");
    if ( hydrolysis_rate < 0 )
        throw InvalidParameter("fiber:hydrolysis_rate should be >= 0");
    if ( shrinking_speed > 0 )
        throw InvalidParameter("fiber:shrinking_speed should be <= 0");
    
    growing_rate_dt[0]  =   sp->time_step * fabs(growing_speed[0]) / unit_length;
    growing_rate_dt[1]  =   sp->time_step * fabs(growing_speed[1]) / unit_length;
    
    hydrolysis_rate_2dt = 2*sp->time_step * hydrolysis_rate;
    shrinking_rate_dt   =   sp->time_step * fabs(shrinking_speed) / unit_length;
    
    if ( min_length <= 0 )
        min_length = 3 * unit_length;
    
    if ( 0 && plist )
    {
        /*
         Using formula from:
         A theory of microtubule catastrophes and their regulation</b>\n
         Brun L, Rupp B, Ward J, Nedelec F\n
         PNAS 106 (50) 21173-21178; 2009\n
         */
        real rate = hydrolysis_rate;
        real ctime = ( growing_speed[0] / unit_length ) / ( 3 * rate * rate );
        std::cerr << " DynamicFiber: catastrophe_rate = " << std::setprecision(3) << 1/ctime;
        std::cerr << "  length = " << std::setprecision(3) << growing_speed[0]*ctime << std::endl;
    }
}

//------------------------------------------------------------------------------

void DynamicFiberProp::write_data(std::ostream & os) const
{
    FiberProp::write_data(os);
    
    write_param(os, "unit_length",     unit_length);
    write_param(os, "growing_force",   growing_force);
    write_param(os, "growing_speed",   growing_speed, 2);
    write_param(os, "hydrolysis_rate", hydrolysis_rate);
    write_param(os, "shrinking_speed", shrinking_speed);
    write_param(os, "fate",            fate);
}

