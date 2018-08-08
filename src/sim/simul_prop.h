// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SIMUL_PROP_H
#define SIMUL_PROP_H


#include "real.h"
#include "simul.h"
#include "vector.h"
#include "property.h"

class Glossary;
class SpaceProp;
class Simul;
class Space;


/// Property for Simul
/** 
 There is normally only one instantiation of this class.
 
 @ingroup Properties
 */
class SimulProp : public Property
{
    friend class Simul;
    
public:
    
    /**
     @defgroup SimulPar Parameters of Simul
     @ingroup Parameters
     @{
     */
    
    
    /// a small interval of time
    /**
     The time_step is the amount of time between two consecutive simulation states.
     It controls the precision of the simulation, at the expense of computation.\n
     We can guarantee that the numerical result will converge to the true mathematical solution 
     of the equations when `time_step` becomes infinitely small, but we do not necessarily 
     know how fast this convergence will be.\n
     To check that `time_step` is appropriate for a particular problem, one should 
     thus run several simulations where `time_step` is varied systematically.
     
     Useful rules:
     - If the results of two simulations with <em>time_step=h</em> and <em>time_step=h/2</em> are identical, 
     then \a h is probably an appropriate `time_step`.
     - If the simulation has some rate R, then time_step should be such that <em>time_step * R << 1</em>
     - A smaller time step is always preferable, provided that the time to run the simulation
     remains acceptable to the user.
     .
     */
    real      time_step;
    
    
    /// Ambient viscosity
    /**
     The viscosity should be given in units of pN.s/um^2 = N.s/m^2 = Pa.s:
     - Water at ambient temperature has a viscosity of 0.001,
     - The viscosity inside an embryo can be ~ 1,
     - The viscosity of the cytoplasm in S.pombe is ~ 1,
     - The viscosity of a cleared cytoplasm can be 0.02.
     .
     http://en.wikipedia.org/wiki/Viscosity
     
     <em>default value = 1</em>
     */
    real      viscosity;
    
#ifdef NEW_CYTOPLASMIC_FLOW
    /// uniform and constant fluid flow
    Vector    flow;
#endif
    
    /// Level of Brownian motion in the system = temperature * Boltzman constant
    /**
     <em>kT</em> is the product of the absolute temperature in Kelvin,
     by the Boltzmann constant k (http://en.wikipedia.org/wiki/Boltzmann_constant):
     - k = 1.3806 . 10^-23 Joule/Kelvin = 1.3806 . 10^-5 pN.um / Kelvin
     - Kelvin = Celsius + 273.15
     .
     
     Examples:
     - If temperature T = 24 C = 297 Kelvin, then kT = 1.3806 * 297 = 410 . 10^-5 = 0.0041 pN.um
     - If temperature T = 37 C = 310.15 Kelvin, then kT = 1.3806 * 310.15 = 410 . 10^-5 = 0.00428 pN.um
     .
     <em>default value = 0.0042</em>
     */
    real      kT;
    
    
    /// Seed for random number generator
    /**
     The simulation uses SFMT, a fast Mersenne Twister to generate pseudo-random numbers
     http://en.wikipedia.org/wiki/Mersenne_twister
     
     The generator is initialized from \a random_seed.
     If ( random_seed == 0 ) then random_seed is calculated from the clock time during initialization.
    
     <em>default value = 0</em>
     */
    unsigned long random_seed;
    
    
    /// Desired precision in the motion of the objects
    /**
     The motion of the objects is solved with a residual error that
     is lower than \a tolerance * \a B, where \a B is the typical 
     Brownian displacement of the objects in one time_step.\n
     <em>Thus one should set 0 < tolerance < 1.</em>\n
     Lower values of tolerance lead to increased precision at the expense of CPU time.
     
     <em>default value = 0.05</em>
    */
    real      tolerance;
    
    
    /// Precision threshold for stochastic events
    /** 
     A warning message is issued for a rate K if:
     @code
     K * time_step > acceptable_rate
     @endcode
     In most implementations, a stochastic event (binding/unbinding) may only occur once
     during a time_step, and this becomes inaccurate if ( K * time_step is not small compared to 1 ).
     
     A user may control the `rate overflow' by adjusting \a acceptable_rate and monitoring the 
     warning messages.
     
     <em>default value = 0.5</em>
     */
    real      acceptable_rate;
    
    
    /// A flag to enable preconditionning when solving the system of equations
    /**
     The accepted values of \a precondition are:
     - 0 : never use preconditionning
     - 1 : always use preconditionning
     - 2 : try the two methods, and use the fastest one
     .
     
     <em>default value = 1</em>
     */
    int       precondition;

    
    /// A flag to control the engine that implement steric interactions between objects
    int       steric;
    
    /// Stiffness for repulsive steric interaction
    real      steric_stiffness_push[2];
    
    /// Stiffness for attractive steric interaction
    real      steric_stiffness_pull[2];

    /// Lattice size used to determine steric interactions
    /**
     Cytosim uses a divide-and-conquer approach to find pairs of objects that are 
     close enough to interact, based on a dividing the Space with a rectangular grid (see PointGrid).
     
     \a steric_max_range defines the minimum size of the cells in the grid. 
     A finer grid reduces false positives, but increases the amount of memory occupied by the grid,
     and the number calculations that are necessary to maintain and clear the grid.
     
     Thus optimal performance is usually obtained for an intermediate value of \a steric_max_range.
     However \a steric_max_range must remain greater than the maximum interaction distance,
     otherwise some interacting pairs will be missed. 
     Experimentation is usually necessary to find the best value.
     
     The maximum distance at which an object may interact with a sibling is its diameter.
     Generally, \a steric_max_range should be greater or equal to the sum of the radiuses,
     of any two object that may interact.
     In the case of fiber, the 'interaction-radius' is a combination of the segmentation,
     and the radius: sqrt( (4/3*segmentation)^2 + 4*radius^2 )

     If the parameter is not set, cytosim attempts to calculate \a steric_max_range automatically.
     */
    real      steric_max_range;
    
    
    /// Lattice size used to determine the attachment of Hand to Fiber
    /**
     Cytosim uses a divide-and-conquer approach to detect which Fibers are near a given point,
     witout testing every Fiber. This is necessary to determine onto which Fiber a Hand may bind.
     The algorithm is based on partitionning Space with a rectangular grid
     with cells of size \a binding_grid_step (see FiberGrid).

     \a binding_grid_step affects the execution speed of the algorithm, but not its result.
     Smaller values of binding_grid_step reduce the number of false positives, 
     but require more memory and housekeeping calculations. 
     Memory requirements also increase with the physical dimensions of the system, 
     to the power DIM (the dimensionality, set at compilation time).
     */
    real      binding_grid_step;
    
    /// level of verbosity
    int           verbose;

    /// Name of configuration file (<em>default = config.cym</em>)
    std::string   config;
    
    /// Name of output property file  (also known as \a properties, <em>default = properties.cmo</em>)
    std::string   property_file;
    
    /// Name of output trajectory file (also known as \a trajectory, <em>default = objects.cmo</em>)
    std::string   trajectory_file;
    
    /// If false, any pre-existing trajectory_file will be erased (<em>default = false</em>)
    bool          append_file;
    
    /// Display parameters (see @ref DisplayPar)
    std::string   display;

    /// @}
    //------------------ derived variables below ----------------
    
    /// if true, do not accept parameter values that would lead to incorrect results
    /**
     This parameter is automatically set to false when objects are set,
     and to 'true' when the simulation is calculated by the 'run' command.
     */
    bool          strict;

    /// this is set to true when 'display' is modified, and to 'false' when it is read
    bool          display_fresh;
    
    /// this is a backpointer to the associated Simul
    Simul*        simul;

public:
    
    /// constructor
    SimulProp(const std::string& n, Simul * s) : Property(n) { clear(); simul=s; }
    
    /// destructor
    ~SimulProp()  { }
    
    /// identifies the property
    std::string kind() const { return "simul"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// check and derive parameters
    void complete(SimulProp const*, PropertyList*);

    /// return a carbon copy of object
    Property* clone() const { return new SimulProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;

};

#endif

