// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "messages.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "property_list.h"
#include "simul_prop.h"
#include "mighty_prop.h"
#include "mighty.h"

//------------------------------------------------------------------------------
Hand * MightyProp::newHand(HandMonitor* h) const
{
    return new Mighty(this, h);
}

//------------------------------------------------------------------------------
void MightyProp::clear()
{    
    HandProp::clear();

    stall_force       = 0;
    max_speed         = 0;
    limit_speed       = true;
    unbinding_density = 0;

    var_speed_dt      = 0;
    max_speed_dt      = 0;
    abs_speed_dt      = 0;
}

//------------------------------------------------------------------------------
void MightyProp::read(Glossary& glos)
{
    HandProp::read(glos);
    
    glos.set(stall_force,       "stall_force");
    glos.set(max_speed, "unloaded_speed") || glos.set(max_speed, "max_speed");
    glos.set(limit_speed,       "limit_speed");
    glos.set(unbinding_density, "unbinding_density");
    //alternative syntax:
    glos.set(unbinding_density, "unbinding", 2);
}

//------------------------------------------------------------------------------
void MightyProp::complete(SimulProp const* sp, PropertyList* plist)
{
    HandProp::complete(sp, plist);
    
    if ( stall_force <= 0 )
        throw InvalidParameter("mighty:stall_force must be > 0");
    
    if ( unbinding_density < 0 )
        throw InvalidParameter("mighty:unbinding_density must be >= 0");

    max_speed_dt = sp->time_step * max_speed;
    abs_speed_dt = fabs(max_speed_dt);
    var_speed_dt = abs_speed_dt / stall_force;

    
    // The limits for a displacement in one time_step apply if ( limit_speed = true )
    if ( max_speed > 0 )
    {
        min_dabs = 0;
        max_dabs = 2 * max_speed_dt;
    }
    else
    {
        min_dabs = 2 * max_speed_dt;
        max_dabs = 0;
    }
}



void MightyProp::checkStiffness(real stiff, real len, real mul, real kT) const
{
    HandProp::checkStiffness(stiff, len, mul, kT);

    /*
     Compare mobility with stiffness: this can induce instability
     */
    real ef = abs_speed_dt * stiff * mul / stall_force;
    if ( max_speed  &&  ef > 0.5 )
    {
        std::ostringstream oss;
        oss << "motor `" << name() << "' can be unstable:" << std::endl;
        oss << PREF << "time_step * stiffness * max_speed / stall_force = " << ef << std::endl;
        oss << PREF << "reduce time_step" << std::endl;
        Cytosim::warning(oss.str().c_str());
        //throw InvalidParameter(oss.str());
    }
    
    /*
     Compare the energy in a link due to the equipartition theorem
     to the maximum force that the motor can sustain before detaching:
     1/2 kT * DIM  <<  1/2 stiffness x^2 ~ 1/2 force^2 / stiffness;
     */
    if ( sqrt( DIM * kT * stiff ) > stall_force )
    {
        std::ostringstream oss;
        oss << "The stall force of `" << name() << "' is too small:" << std::endl;
        oss << PREF << "DIM * kT * stiffness > stall_force" << std::endl;
        oss << PREF << "reduce stiffness or increase stall_force" << std::endl;
        Cytosim::warning(oss.str().c_str());
    }
    
    /*
     Compare the distance traveled during the time 1/detachrate,
     and how much force this would induce compared to stall_force.
     This is limit the efficiency of the motor.
     */
    ef = fabs( stiff * max_speed / ( unbinding_rate * stall_force ));
    if ( unbinding_rate && max_speed  &&  ef < 1 )
    {
        std::ostringstream oss;
        oss << "The efficiency of `" << name() << "' is low:" << std::endl;
        oss << PREF << "stiffness * max_speed / stall_force * unbinding_rate = " << ef << std::endl;
        Cytosim::warning(oss.str().c_str());
    }
    
    /*
     Compare detachment rate at stall-force, with detachment rate at rest
     */
    if ( exp( stall_force * unbinding_force_inv ) > 100 )
        Cytosim::warning("Hand:exp( stall_force / unbinding_force ) is greater than 100\n");

}


//------------------------------------------------------------------------------

void MightyProp::write_data(std::ostream & os) const
{
    HandProp::write_data(os);
    write_param(os, "stall_force", stall_force);
    write_param(os, "max_speed",   max_speed);
    write_param(os, "limit_speed", limit_speed);
    write_param(os, "unbinding_density", unbinding_density);
}

