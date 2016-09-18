// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "tubule_prop.h"
#include <cmath>
#include "sim.h"
#include "tubule.h"
#include "glossary.h"
#include "exceptions.h"
#include "property_list.h"
#include "simul_prop.h"


Fiber* TubuleProp::newFiber() const
{
    return new Tubule(this);
}

//------------------------------------------------------------------------------
void TubuleProp::clear()
{
    FiberProp::clear();
    
    dynamic_model[0] = 0;
    dynamic_model[1] = 0;
    growing_force    = INFINITY;
    min_length       = 0.01;
    fate             = FATE_DESTROY;
    
    for ( unsigned int n=0; n<4; ++n )
    {
        dynamic_trans1[n] = 0;
        dynamic_trans2[n] = 0;
        dynamic_speed1[n] = 0;
        dynamic_speed2[n] = 0;
    }
}

//------------------------------------------------------------------------------
void TubuleProp::read(Glossary& glos)
{
    FiberProp::read(glos);
    
    glos.set(dynamic_model, 2,  "dynamic_model");
    glos.set(growing_force,     "growing_force");
    glos.set(min_length,        "min_length");
    glos.set(fate,              "fate", KeyList<Fate>("destroy", FATE_DESTROY, "rescue", FATE_RESCUE, "none", FATE_NONE));
    
    glos.set(dynamic_trans1, 4, "dynamic_trans1");
    glos.set(dynamic_trans2, 4, "dynamic_trans2");
    glos.set(dynamic_speed1, 4, "dynamic_speed1");
    glos.set(dynamic_speed2, 4, "dynamic_speed2");
    
#ifdef BACKWARD_COMPATIBILITY
    
    if ( glos.set(growing_force, "dynamic_force") )
        MSG.warning("fiber:dynamic_force was renamed growing_force\n");
    
    if ( glos.set(fate, "dynamic_fate", KeyList<Fate>("destroy", FATE_DESTROY, "rescue", FATE_RESCUE, "none", FATE_NONE)) )
        MSG.warning("fiber:dynamic_fate was renamed fate\n");
    
    if ( glos.set(fate, "shrinking_fate", KeyList<Fate>("destroy", FATE_DESTROY, "rescue", FATE_RESCUE, "none", FATE_NONE)) )
        MSG.warning("fiber:shrinking_fate was renamed fate\n");
    
#endif
}


void TubuleProp::complete(SimulProp const* sp, PropertyList* plist)
{
    FiberProp::complete(sp, plist);
    
    real dt = sp->time_step;
    
    if ( growing_force <= 0 )
        throw InvalidParameter("fiber:growing_force should be specified and > 0");
    
    for ( unsigned int n=0; n<4; ++n )
    {
        if ( dynamic_trans1[n] < 0 )
            throw InvalidParameter("tubule:dynamic_trans1[",n,"] should be >= 0");
        if ( dynamic_trans1[n] * dt > sp->acceptable_rate )
            throw InvalidParameter("tubule:dynamic_trans1[",n,"] is too high: decrease time_step");
        
        if ( dynamic_trans2[n] < 0 )
            throw InvalidParameter("tubule:dynamic_trans2[",n,"] should be >= 0");
        if ( dynamic_trans2[n] * dt > sp->acceptable_rate )
            throw InvalidParameter("tubule:dynamic_trans2[",n,"] is too high: decrease time_step");
    }
}

//------------------------------------------------------------------------------

void TubuleProp::write_data(std::ostream & os) const
{
    FiberProp::write_data(os);
    write_param(os, "dynamic_model",  dynamic_model, 2);
    write_param(os, "growing_force",  growing_force);
    write_param(os, "min_length",     min_length);
    write_param(os, "fate",           fate);
    write_param(os, "dynamic_speed1", dynamic_speed1, 4);
    write_param(os, "dynamic_speed2", dynamic_speed2, 4);
    write_param(os, "dynamic_trans1", dynamic_trans1, 4);
    write_param(os, "dynamic_trans2", dynamic_trans2, 4);
}

