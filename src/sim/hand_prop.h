// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef HAND_PROP
#define HAND_PROP

#include "real.h"
#include "property.h"


class Glossary;
class PointDisp;
class Hand;
class HandMonitor;


/**
 Enables detachment rate that are different if Hands are near the Fiber end.
 
 Implementation done by Jamie-Li Rickman for:
 "Determinants of polar versus nematic organization in networks
 of dynamic microtubules and mitotic motors" published in 2018
 By J. Roostalu, J. Rickman, C. Thomas, F. Nedelec and T. Surrey
 */
#define NEW_END_DEPENDENT_DETACHMENT 0


/// Property for Hand
/**
 @ingroup Properties
*/
class HandProp : public Property
{
    friend class Hand;
    
public:
      
    /// return one of the Property derived from HandProp
    static HandProp * newProperty(const std::string& n, Glossary&);
  
    /// the maximum range (useful to set the binding grid)
    static real binding_range_max;
    
public:
    
    /**
     @defgroup HandPar Parameters of Hand
     @ingroup Parameters
     @{
     */
    
    /// binding rate when the Hand is within \a binding_range (also known as \c binding[0])
    /**
     According to Leduc et al. PNAS 2004 vol. 101 no. 49 17096-17101
     the molecular binding_rate of kinesin is 4.7 +/- 2.4 /s.
     <em>
     http://dx.doi.org/10.1073/pnas.0406598101 \n
     http://www.pnas.org/content/101/49/17096.abstract
     </em>
     */
    real         binding_rate;
    
    
    /// maximum distance at which the Hand can bind (also known as \c binding[1])
    real         binding_range;
    
    
    /// can be set to restrict binding to certain type of Fiber
    /**
     The binding to a fiber is allowed only if the keys of the Hand and Fiber match.
     The test is a BITWISE-AND of the two keys:
     @code
     if ( fiber:binding_key & hand:binding_key )
        allowed = true;
     else
        allowed = false;
     @endcode
     */
    unsigned int binding_key;
    
    
    /// detachment rate at force=0 (also known as \c unbinding[0])
    /**
     Kramers theory specifies that the detachment rate depends on the force
     in the link:
     @code
     off_rate = RATE * exp( force / FORCE )
     @endcode
     RATE is specified as \a unbinding_rate, and FORCE as \a unbinding_force,
     but one can also directly specify `unbinding = RATE, FORCE`.
     (see @ref Stochastic)
     */
    real         unbinding_rate;

#if NEW_END_DEPENDENT_DETACHMENT
    /// rate of detachment, once the motor has reached the end of a fiber
    real         unbinding_rate_end;
#endif
    
    
    /// characteristic force of unbinding (also known as \c unbinding[1])
    /**
     @copydetails unbinding_rate
     */
    real         unbinding_force;
    
    
    /// if true, the Hand can also bind directly to the tip of fibers
    /**
     This determines the binding ability of a Hand that is located within the binding_range
     of a fiber, but at a position where the orthogonal projection is outside the Fiber,
     ie. below the MINUS_END, or above the PLUS_END.
     This corresponds to two hemi-spheres at the two ends of a Fiber.
     
     <em>default value = false</em>
     */
    bool         bind_also_ends;
    
    
    /// if false, the Hand will detach immediately upon reaching a growing or a static fiber end
    /**
     A Hand may reach the tip of the fiber on which it is bound,
     either if it is moving, or if is dragged by some other force.
     When this happens, \a hold_growing_end will determine if the Hand
     will detach or not.
     
     <em>default = false</em>
     */
    bool         hold_growing_end;
    
    
    /// if false, the Hand will detach immediately upon reaching a shrinking fiber end
    /**
     A Hand may reach the tip of the fiber on which it is bound,
     of the tip of the fiber may reach a immobile hand because it is disassembling.
     When this happens, \a hold_shrinking_end will determine if the Hand
     will detach or not.
     If \a hold_shrinking_end is true, the hand will be relocated to track the end.

     <em>default = false</em>
     */
    bool         hold_shrinking_end;
    
    
    /// specialization
    /**
     @copydetails HandGroup
     */
    std::string  activity;
    
    
    /// display parameters (see @ref PointDispPar)
    std::string  display;
    
    /// @}
    //------------------ derived variables below ----------------
    
    /// inverse of unbinding_force:
    real unbinding_force_inv;
    
public:
    
    /// binding_rate_dt = binding_rate * time_step;
    real   binding_rate_dt;
    
    /// binding_range_sqr = binding_range * binding_range;
    real   binding_range_sqr;
    
    /// unbinding_rate_dt = unbinding_rate * time_step;
    real   unbinding_rate_dt;
    
#if NEW_END_DEPENDENT_DETACHMENT
    /// unbinding_rate_end_dt = unbinding_rate_end * time_step;
    real   unbinding_rate_end_dt;
#endif
    
    /// the display parameters for this category of Hand
    PointDisp  * disp;
    
public:
    
    /// constructor
    HandProp(const std::string& n) : Property(n), disp(0) { clear(); }
    
    /// destructor
    ~HandProp() { }
    
    /// return a Hand with this property
    virtual Hand * newHand(HandMonitor* h) const;

    /// identifies the property
    std::string kind() const { return "hand"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    virtual void read(Glossary&);
    
    /// compute values derived from the parameters
    virtual void complete(SimulProp const*, PropertyList*);
    
    /// perform more checks, knowing the elasticity
    virtual void checkStiffness(real stiff, real len, real mul, real kT) const;
    
    /// write all values
    void write_data(std::ostream &) const;
    
    /// return a carbon copy of object
    Property* clone() const { return new HandProp(*this); }
};


#endif

