// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "simul_prop.h"
#include "assert_macro.h"
#include "space_prop.h"
#include "space_set.h"
#include "simul.h"
#include "exceptions.h"
#include "messages.h"
#include "glossary.h"
#include "property_list.h"
#include "random.h"

extern Random RNG;
extern bool functionKey[];


void SimulProp::clear()
{
    viscosity         = 1;
#ifdef NEW_CYTOPLASMIC_FLOW
    flow.set(0, 0, 0);
#endif
    time_step         = 0;
    kT                = 0.0042;
    tolerance         = 0.05;
    acceptable_rate   = 0.5;
    precondition      = 1;
    random_seed       = 0;
    steric            = 0;
 
    steric_stiffness_push[0] = 100;
    steric_stiffness_pull[0] = 100;
    steric_stiffness_push[1] = 100;
    steric_stiffness_pull[1] = 100;

    steric_max_range  = -1;
    binding_grid_step = -1;
    
    strict            = 0;
    verbose           = 0;

    config            = "config.cym";
    trajectory_file   = "objects.cmo";
    property_file     = "properties.cmo";
    append_file       = false;
    
    display           = "";
    display_fresh     = false;
}


void SimulProp::read(Glossary& glos)
{
    glos.set(viscosity,         "viscosity");
#ifdef NEW_CYTOPLASMIC_FLOW
    glos.set(flow,              "flow");
#endif
    glos.set(time_step,         "time_step");
    glos.set(kT,                "kT");

    glos.set(tolerance,         "tolerance");
    glos.set(acceptable_rate,   "acceptable_rate");
    glos.set(precondition,      "precondition");
    
    glos.set(steric,                   "steric");
    glos.set(steric_stiffness_push[0], "steric", 1);
    glos.set(steric_stiffness_pull[0], "steric", 2);
    glos.set(steric_stiffness_push, 2, "steric_stiffness_push");
    glos.set(steric_stiffness_pull, 2, "steric_stiffness_pull");
    glos.set(steric_max_range,         "steric_max_range");

    glos.set(binding_grid_step, "binding_grid_step");

    // these parameters are not written:
    glos.set(strict,            "strict");
    glos.set(verbose,           "verbose");
    Cytosim::setVerbose(verbose);
    
    glos.set(functionKey, 17,   "function");
    
    // names of files and path:
    glos.set(config,            "config");
    glos.set(config,            ".cym");
    
    glos.set(property_file,     "property_file");
    glos.set(property_file,     "properties");

#ifdef BACKWARD_COMPATIBILITY
    glos.set(trajectory_file,   "object_file");
#endif
    glos.set(trajectory_file,   "trajectory_file");
    glos.set(trajectory_file,   "trajectory");
    glos.set(trajectory_file,   ".cmo");
    
    glos.set(append_file,       "append_file");
    
    real t;
    if ( glos.set(t, "time") )
        simul->setTime(t);
    
    if ( glos.set(random_seed,  "random_seed") )
        RNG.seed(random_seed);
    
    if ( glos.set(display,      "display") )
        display_fresh = true;
}


/**
 If the Global parameters have changed, we update all derived parameters.
 This makes it possible to change the time-step in the middle of a config file.
 
 However, if \a plist == 0, this operation is skipped
 */
void SimulProp::complete(SimulProp const* sp, PropertyList* plist)
{
    if ( plist )
    {        
        if ( time_step <= 0 )
            throw InvalidParameter("simul:time_step must be specified and > 0");

        if ( kT <= 0 )
            throw InvalidParameter("simul:kT must be > 0");

        // set a valid seed if necessary:
        if ( random_seed == 0 )
        {
            random_seed = RNG.seedTimer();
            Cytosim::MSG(5, "Cytosim: time-generated random seed 0x%lx\n", random_seed);
        }
        
        /* 
         If the Global parameters have changed, we update all derived parameters.
         To avoid an infinite recurence, it is important that SimulProp * this
         is not included in the PropertyList Simul::properties;
         */
        plist->complete(sp);
    }
}

//------------------------------------------------------------------------------

void SimulProp::write_data(std::ostream & os) const
{
    write_param(os, "time_step",       time_step);
    write_param(os, "kT",              kT);
    write_param(os, "viscosity",       viscosity);
#ifdef NEW_CYTOPLASMIC_FLOW
    write_param(os, "flow",            flow);
#endif
    os << std::endl;
    write_param(os, "tolerance",       tolerance);
    write_param(os, "acceptable_rate", acceptable_rate);
    write_param(os, "precondition",    precondition);
    write_param(os, "random_seed",     random_seed);
    os << std::endl;
    write_param(os, "steric", steric, steric_stiffness_push[0], steric_stiffness_pull[0]);
    write_param(os, "steric_max_range",  steric_max_range);
    write_param(os, "binding_grid_step", binding_grid_step);
    write_param(os, "verbose", verbose);
    os << std::endl;

    write_param(os, "display", "("+display+")");
}

