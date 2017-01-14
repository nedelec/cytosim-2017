// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "field_prop.h"
#include "property_list.h"
#include "simul_prop.h"
#include "glossary.h"
#include "dim.h"

//------------------------------------------------------------------------------
void FieldProp::clear()
{
    step                  = 0;
    confine_space         = "first";
    confine_space_ptr     = 0;
    diffusion             = 0;
    diffusion_theta       = 0;
    decay_rate            = 0;
    decay_rate_dt         = 0;
    positive              = 0;
    save                  = true;
}


//------------------------------------------------------------------------------
void FieldProp::read(Glossary& glos)
{
    glos.set(step,               "step");
    glos.set(confine_space,      "space");
    glos.set(diffusion,          "diffusion");
    glos.set(decay_rate,         "decay_rate");

    glos.set(positive,           "positive");
    glos.set(save,               "save");
}


//------------------------------------------------------------------------------
void FieldProp::complete(SimulProp const* sp, PropertyList*)
{
    confine_space_ptr = sp->simul->findSpace(confine_space);

    if ( step < REAL_EPSILON )
        throw InvalidParameter("field:step must be defined and > 0");
       
    if ( diffusion < 0 )
        throw InvalidParameter("field:diffusion must be >= 0");
    
    diffusion_theta = sp->time_step * diffusion / ( step * step );
    
    if ( diffusion_theta * 2 * DIM > sp->acceptable_rate )
    {
        std::cerr << "field:diffusion CFL condition = " << diffusion_theta * 2 * DIM << std::endl;
        std::cerr << "This number must be below 1/2." << std::endl;
        throw InvalidParameter("field:diffusion (diffusion*time_step/step^2) is too high");
    }

    if ( decay_rate < 0 )
        throw InvalidParameter("field:decay_rate must be >= 0");
    
    decay_rate_dt = sp->time_step * decay_rate;
    
    if ( decay_rate_dt >= sp->acceptable_rate )
        throw InvalidParameter("field:decay_rate is too high: decrease time_step\n");
}


//------------------------------------------------------------------------------

void FieldProp::write_data(std::ostream & os) const
{
    write_param(os, "step",        step);
    write_param(os, "space",       confine_space);
    write_param(os, "diffusion",   diffusion);
    write_param(os, "decay_rate",  decay_rate);
    write_param(os, "positive",    positive);
    write_param(os, "save",        save);
}

