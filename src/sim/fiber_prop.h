// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FIBER_PROP
#define FIBER_PROP

#include "real.h"
#include "property.h"
#include "common.h"

class Fiber;
class Glossary;
class FiberDisp;
class SingleProp;
class SingleSet;
class Space;

typedef char Tag;


/// Property for a Fiber
/**
 @ingroup Properties
 */
class FiberProp : public Property
{
    friend class Fiber;
    friend class FiberSet;

public:
    
    /**
     @defgroup FiberPar Parameters of Fiber
     @ingroup Parameters
     These are the parameters for Fiber
     @{
     */
    
    /// length or initial-length for dynamic fibers
    real         length;
    
    /// effective viscosity (if not specified, simul:viscosity is used)
    real         viscosity;
    
    /// can be set to control which Hands may bind
    /** 
     To decide if a Hand may bind to a Fiber, the two keys are compared:
     attachement is forbiden if the BITWISE-AND is 0:
     @code
     if ( fiber:binding_key & hand:binding_key )
        allowed = true;
     else
        allowed = false;
     @endcode   
     It is thus recommended to use powers of 2: 1, 2, 4, etc.
     */
    unsigned int binding_key;
    
    /// modulus for bending elasticity
    /**
     This has units of pN.um^2, and it is related to the persitence length:
     @code
     L_p = rigidity / kT
     @endcode
     
     Many measurments have been made and they agree somewhat.\n
     According to Gittes et al. (1993):
     Filament      |   L_p       | rigidity
     --------------|-------------|-----------------
     Microtubule   | ~ 5200 um   | ~22 pN.um^2
     Actin         |  ~ 18 um    | ~0.075 pN.um^2
     
     <em>
     Flexural rigidity of microtubules and actin filaments measured from thermal fluctuations in shape.
     JCB vol. 120 no. 4 923-934
     http://dx.doi.org/10.1083/jcb.120.4.923 \n
     http://jcb.rupress.org/content/120/4/923
     </em>
     */
    real         rigidity;
    
    /// desired distance between model points
    /**
     This is a distance.
     As a rule of thumb, segmentation should scale with rigidity,
     depending on the expected external forces:
     @code
     segmentation = sqrt(rigidity/force)
     force ~ rigidity / segmentation^2
     @endcode
     
     Furthermore, if any filament contains kinks 
     (the angle between consecutive segments is above 45 deg),
     the simulation should not be trusted and instead should 
     be recalculated with a reduced segmentation.
     */
    real         segmentation;
    
    /// amount of polymer available for this type of fiber
    real         total_polymer;
    
    /// Minimum authorized length
    /**
     When the fiber becomes shorter than \a min_length,
     the action specified by \a fate is applied.
     */
    real         min_length;

    /// radius used to calculate mobility
    /**
     hydrodynamic_radius[0] corresponds to the radius of the fiber
     hydrodynamic_radius[1] is a cut-off for the length of the fiber
     */
    real         hydrodynamic_radius[2];

    /// if true, the mobility of a cylinder moving near a plane will be used
    /**
     You can select between two possible formulas to calculate viscous drag coefficient:
     @code
     if ( fiber:surface_effect )
         setDragCoefficientSurface();
     else
         setDragCoefficientVolume();
     @endcode
     <hr>
     @copydetails Fiber::setDragCoefficientVolume
     <hr>
     @copydetails Fiber::setDragCoefficientSurface
     */
    bool         surface_effect;
    
    /// distance of fluid between slide and cylinder surface (set as \c surface_effect[1])
    real         cylinder_height;

    /// set forces between fiber and Space [none, inside, outside, surface]
    Confinement  confine;
    
    /// stiffness of confinement (set as \c confine[1])
    real         confine_stiff;
    
    /// name of space for confinement (set as \c confine[2])
    std::string  confine_space;
    
    /// if true, include steric interaction for this object
    int          steric;
    
    /// radius of repulsive steric interaction (also known as \c steric[1])
    real         steric_radius;
    
    /// extra radius of attractive steric interaction (also known as \c steric[2])
    real         steric_range;
    
    /// type of glue (interaction between fiber tip and Space)
    int          glue;
    
    /// name of Single used for glue (set a \c glue[1])
    std::string  glue_single;
    
    /// specialization
    /**
     @copydetails FiberGroup
     */
    std::string  activity;
    
    /// display string (see @ref FiberDispPar)
    std::string  display;
    
    /// @}
    //------------------ derived variables below ----------------
    
    /// display
    FiberDisp *  disp;

    /// used for confinement
    Space const* confine_space_ptr;
    
protected:
    
    /// local copy of SimulProp::time_step
    real    time_step;
    
    /// fraction in [0, 1]
    real    free_polymer;
    
    /// total length of fiber for this type
    real    total_length;
    
    /// SingleSet where glue are stored
    SingleSet  * glue_set;
    
    /// SingleProp used for glue
    SingleProp * glue_prop;

public:
    
    /// constructor
    FiberProp(const std::string& n) : Property(n), disp(0) { clear(); }
    
    /// destructor
    ~FiberProp() { }
    
    /// return a non-initialized Fiber with this property
    virtual Fiber* newFiber() const;
    
    /// return a Fiber with this property, initialized
    Fiber* newFiber(Glossary&) const;
    
    /// identifies the property
    std::string kind() const { return "fiber"; }
    
    /// set default values
    virtual void clear();
       
    /// set using a Glossary
    virtual void read(Glossary&);
   
    /// check and derive parameter values
    virtual void complete(SimulProp const*, PropertyList*);
    
    /// return a carbon copy of object
    Property* clone() const { return new FiberProp(*this); }

    /// write
    virtual void write_data(std::ostream &) const;

};

#endif

