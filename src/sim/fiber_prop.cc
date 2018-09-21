// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "fiber_prop.h"
#include "sim.h"
#include "smath.h"
#include <cmath>
#include "messages.h"
#include "exceptions.h"
#include "glossary.h"
#include "property_list.h"
#include "single_prop.h"
#include "simul_prop.h"
#include "simul.h"
#include "fiber.h"

/**
 This is virtualized to return a derived Fiber if appropriate
 */
Fiber* FiberProp::newFiber() const
{
    return new Fiber(this);
}


/**
 @addtogroup FiberGroup
 @{
 <hr>
 
 When creating a new Fiber, you may specify:
 - the initial length,
 - the initial state of the PLUS_END and MINUS_END,
 - if the position refers to the center or to the tip of the fiber
 .
 
 Syntax:
 
 @code
 new fiber ...
 {
   length = REAL, LENGTH_MODIFIER
   end_state = PLUS_END_STATE, MINUS_END_STATE
   reference = REFERENCE
 }
 @endcode
 
 The optional LENGTH_MODIFIER can be:
 - `exponential`,
 - REAL
 .
 This introduces variability, without changing the mean length.
 The second form generates a flat distribution of width 2*LENGTH_MODIFIER.
 
 The initial states PLUS_END_STATE and MINUS_END_STATE can be:
 - 0 = white
 - 1 = green
 - 4 = red
 .
 
 Optional reference specificiation:
 - center [default]
 - plus_end
 - minus_end
 .
 
 
 Examples:
 
 @code
 new fiber ...
 {
   length = 1
   plus_end_state = 1
   minus_end_state = 0
 }
 @endcode

 which is equivalent to:
 @code
 new fiber ...
 {
   length = 1
   end_state = green, white
 }
 @endcode
 
 @}
 */
Fiber* FiberProp::newFiber(Glossary& opt) const
{
    Fiber * fib = newFiber();
    real len = length;
    
    /*
     initial length and reference point for placement can be specified in 'opt'
     */
#ifdef BACKWARD_COMPATIBILITY
    opt.set(len, "initial_length");
#endif
    opt.set(len, "length") || opt.set(len, "fiber_length");
    
    // exponential distribution:
    std::string dis;
    if ( opt.set(dis, "length", 1) && dis == "exponential" )
    {
        len *= RNG.exponential();
    }
    else
    {
        // add variability without changing mean:
        real dlen = 0;
        if ( opt.set(dlen, "length", 1) )
        {
            len += dlen * RNG.sreal();
            if ( len <= 0 )
                throw InvalidParameter("fiber:length with added variability became negative");
        }
    }
    
    if ( opt.has_key("shape") )
    {
        unsigned nbp = opt.nb_values("shape");
        real* tmp = new real[DIM*nbp];
        
        for ( unsigned p = 0; p < nbp; ++p )
        {
            Vector vec(0,0,0);
            if ( ! opt.set(vec, "shape", p) )
                throw InvalidParameter("fiber:shape must be a vector");
            vec.put(tmp+DIM*p);
        }
        fib->setShape(tmp, nbp, 0);
        fib->reshape();
        delete[] tmp;
    }
    else
    {
        FiberEnd reference = CENTER;
        
        opt.set(reference, "reference",
                KeyList<FiberEnd>("plus_end", PLUS_END, "minus_end", MINUS_END, "center", CENTER));
        
        // initialize points:
        fib->setStraight(Vector(0,0,0), Vector(1,0,0), len, reference);
    }
    
    // initialize state of the ends
    KeyList<int> keys("white",  STATE_WHITE,
                      "green",  STATE_GREEN,
                      "grow",   STATE_GREEN,
                      "shrink", STATE_RED,
                      "yellow", STATE_YELLOW,
                      "orange", STATE_ORANGE,
                      "red",    STATE_RED);
    
    int s = STATE_WHITE;
    
    if ( opt.set(s, "plus_end", keys) || opt.set(s, "plus_end_state", keys) || opt.set(s, "end_state", keys) )
        fib->setDynamicState(PLUS_END, s);
    
    if ( opt.set(s, "minus_end", keys) || opt.set(s, "minus_end_state", keys) || opt.set(s, "end_state", keys, 1) )
        fib->setDynamicState(MINUS_END, s);
    
    return fib;
}



//------------------------------------------------------------------------------
void FiberProp::clear()
{
    length              = 1;
    min_length          = 0;
    total_polymer       = 0;
    
    viscosity           = -1;
    surface_effect      = false;
    cylinder_height     = 0;
    
    binding_key         = (~0);  //all bits at 1
    rigidity            = -1;
    segmentation        = 1;
    
    confine             = CONFINE_NOT;
    confine_stiff       = -1;
    confine_space       = "first";
    confine_space_ptr   = 0;
    
    steric              = 0;
    steric_radius       = 0;
    steric_range        = 0;
    
    glue                = 0;
    glue_single         = "undefined";
    glue_prop           = 0;
    glue_set            = 0;
    
    hydrodynamic_radius[0] = 0.0125;
    hydrodynamic_radius[1] = 10;
    
    activity            = "none";
    display             = "";
    
    total_length        = 0;
    free_polymer        = 1;
    time_step           = 0;
}

//------------------------------------------------------------------------------
void FiberProp::read(Glossary& glos)
{
    glos.set(length,            "length");
    glos.set(min_length,        "min_length");
    glos.set(total_polymer,     "total_polymer");

    glos.set(viscosity,         "viscosity");
    glos.set(surface_effect,    "surface_effect");
    glos.set(cylinder_height,   "surface_effect", 1);
    
    glos.set(binding_key,       "binding_key");
    glos.set(rigidity,          "rigidity");
    glos.set(segmentation,      "segmentation");
    
    glos.set(confine,           "confine",
             KeyList<Confinement>("none",      CONFINE_NOT,
                                  "inside",    CONFINE_INSIDE,
                                  "outside",   CONFINE_OUTSIDE,
                                  "surface",   CONFINE_SURFACE,
                                  "plus_end",  CONFINE_PLUS_END,
                                  "minus_end", CONFINE_MINUS_END));
    glos.set(confine_stiff,     "confine", 1);
    glos.set(confine_space,     "confine", 2);

#ifdef BACKWARD_COMPATIBILITY
    if ( confine_space == "current" )
        confine_space = "last";

    glos.set(confine,           "confined",
             KeyList<Confinement>("none",      CONFINE_NOT,
                                  "inside",    CONFINE_INSIDE,
                                  "outside",   CONFINE_OUTSIDE,
                                  "surface",   CONFINE_SURFACE,
                                  "minus_end", CONFINE_MINUS_END,
                                  "plus_end",  CONFINE_PLUS_END));
    glos.set(confine_stiff,     "confined", 1);
#endif
    
    glos.set(steric,            "steric");
    glos.set(steric_radius,     "steric", 1);
    glos.set(steric_range,      "steric", 2);
    glos.set(steric_radius,     "steric_radius");
    glos.set(steric_range,      "steric_range");
    
    glos.set(glue,              "glue");
    glos.set(glue_single,       "glue", 1);
    
    glos.set(hydrodynamic_radius, 2, "hydrodynamic_radius");
        
    glos.set(display,  "display");
    glos.set(activity, "activity");
}


void FiberProp::complete(SimulProp const* sp, PropertyList* plist)
{
    time_step = sp->time_step;
    
    confine_space_ptr = sp->simul->findSpace(confine_space);
    
    if ( length <= 0 )
        throw InvalidParameter("fiber:length should be > 0");
    
    if ( viscosity < 0 )
        viscosity = sp->viscosity;
    
    if ( viscosity < 0 )
        throw InvalidParameter("fiber:viscosity or simul:viscosity should be defined");

    if ( glue )
    {
        glue_set = &sp->simul->singles;
        glue_prop = static_cast<SingleProp*>(plist->find_or_die("single", glue_single));
    }

    if ( rigidity < 0 )
        throw InvalidParameter("fiber:rigidity must be specified and >= 0");
    
    if ( segmentation <= 0 )
        throw InvalidParameter("fiber:segmentation must be > 0");
 
    if ( steric && steric_radius <= 0 )
        throw InvalidParameter("fiber:steric[1] (radius) must be specified and > 0");
    
    if ( confine && confine_stiff < 0 )
        throw InvalidParameter("The stiffness fiber:confinement[1] must be specified and >= 0");
    
    if ( hydrodynamic_radius[0] <= 0 )
        throw InvalidParameter("fiber:hydrodynamic_radius[0] must be > 0");
    
    if ( hydrodynamic_radius[1] <= 0 )
        throw InvalidParameter("fiber:hydrodynamic_radius[1] must be > 0");

#if ( 0 )
    //print some information on the 'stiffness' of the matrix
    Fiber fib(this);
    fib.setStraight(Vector(0,0,0), Vector(1,0,0), 10, CENTER);
    
    fib.setDragCoefficient();
    real mob_dt = sp->time_step * fib.nbPoints() / fib.dragCoefficient();
    
    real stiffness = 100;
    real coef1 = mob_dt * stiffness;

    Cytosim::MSG(5, "Numerical hardness (stiffness=%.1f): %7.2f\n", stiffness, coef1);

    real rod   = segmentation;
    real coef2 = mob_dt * rigidity / ( rod * rod * rod );
    
    Cytosim::MSG(5, "Numerical hardness (rigidity=%.1f): %7.2f\n", rigidity, coef2);
#endif
}


//------------------------------------------------------------------------------

void FiberProp::write_data(std::ostream & os) const
{
    write_param(os, "length",              length);
    write_param(os, "min_length",          min_length);
    write_param(os, "total_polymer",       total_polymer);

    write_param(os, "viscosity",           viscosity);
    write_param(os, "surface_effect",      surface_effect, cylinder_height);
    write_param(os, "rigidity",            rigidity);
    write_param(os, "segmentation",        segmentation);

    write_param(os, "hydrodynamic_radius", hydrodynamic_radius, 2);
    write_param(os, "binding_key",         binding_key);
    write_param(os, "confine",             confine, confine_stiff, confine_space);
    write_param(os, "steric",              steric, steric_radius, steric_range);
    write_param(os, "glue",                glue, glue_single);
    write_param(os, "activity",            activity);

    write_param(os, "display",             "("+display+")");
}

