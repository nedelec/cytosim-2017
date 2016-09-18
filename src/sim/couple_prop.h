// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef COUPLE_PROP_H
#define COUPLE_PROP_H

#include "real.h"
#include "property.h"
#include "hand_prop.h"
#include "common.h"

class Glossary;
class Couple;
class Space;

/// Property for Couple
/**
 @ingroup Properties
*/
class CoupleProp : public Property
{
    friend class Couple;
    
public:
    
    /// indicates a specificity for crosslinking two fibers
    enum Specificity 
    {
        BIND_ALWAYS,        ///< can bind in any configuration
        BIND_ANTIPARALLEL,  ///< can link two fibers only if they are anti-parallel ( cos(angle) < 0 )
        BIND_PARALLEL       ///< can link two fibers only if they are parallel ( cos(angle) > 0 )
    };
    
    /**
     @defgroup CouplePar Parameters of Couple
     @ingroup Parameters
     @{
     */
    
    
    /// name of first Hand in Couple
    std::string  hand1;
    
    /// name of second Hand in Couple
    std::string  hand2;
    
    /// stiffness of link between the two Hands while linking
    real         stiffness;
    
    /// resting length of the link
    real         length;
    
    /// diffusion coefficient while unattached
    real         diffusion;
    
    /// if true, an algorithm is used that assumes uniform concentration of diffusing Couple
    bool         fast_diffusion;
    
    /// prevent both Hands from binding at the same position on a Fiber (default=true)
    bool         stiff;
    
    /// Confinement can be \a none, \a inside (default) or \a surface
    Confinement  confine;
    
    /// Unused Parameter: confinement stiffness (this is specified as \c confine[1])
    real         confine_stiff;
    
    /// name of space for confinement (this is specified as \c confine[2])
    std::string  confine_space;
    
    /// specialization
    /**
     @copydetails CoupleGroup
     */
    std::string  activity;
    
    /// @}
    //------------------ derived variables below ----------------
    
protected:
    
    real          diffusion_dt;
    HandProp *    hand_prop1;
    HandProp *    hand_prop2;
    Space const*  confine_space_ptr;

public:
    
    /// constructor
    CoupleProp(const std::string& n) : Property(n)  { clear(); }
    
    /// destructor
    ~CoupleProp() { }
    
    /// create a Couple having this property
    virtual Couple * newCouple(Glossary * opt = 0) const;
    
    /// identifies the property
    std::string kind() const { return "couple"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute derived parameter values
    void complete(SimulProp const*, PropertyList*);
    
    /// return a carbon copy of object
    Property* clone() const { return new CoupleProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
  
    /// return confining Space
    Space const* confineSpace() const { return confine_space_ptr; }
};

#endif

