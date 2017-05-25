// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SINGLE_PROP_H
#define SINGLE_PROP_H

#include "real.h"
#include "property.h"
#include "hand_prop.h"
#include "common.h"

class Mecable;
class Glossary;
class Single;
class Wrist;
class Space;

/// Property for Single
/**
 @ingroup Properties
*/
class SingleProp : public Property
{
    friend class Single;
    friend class Wrist;
    friend class WristLong;

public:
    
    /**
     @defgroup SinglePar Parameters of Single
     @ingroup Parameters
     @{
     */
    
    
    /// name of Hand
    std::string  hand;
    
    /// stiffness of link
    real         stiffness;
    
    /// resting length of link
    real         length;
    
    /// diffusion coefficient
    real         diffusion;
    
    /// Confinement can be \c none, \c inside (default) or \c surface
    Confinement  confine;
    
    /// Unused Parameter: confinement stiffness (this is specified as \c confine[1])
    real         confine_stiff;
    
    /// name of space for confinement (this is specified as \c confine[2])
    std::string  confine_space;
    
    /// specialization
    /**
     @copydetails SingleGroup
     */
    std::string  activity;
    
    /// @}
    //------------------ derived variables below ----------------

protected:
    
    /// Property of associated Hand
    HandProp *     hand_prop;
    
    /// associated Space
    Space const*   confine_space_ptr;
    
    /// displacement in one time_step
    real           diffusion_dt;

public:
    
    /// constructor
    SingleProp(const std::string& n) : Property(n)  { clear(); }

    /// destructor
    ~SingleProp() { }
    
    /// create a Single with this property
    Single * newSingle(Glossary * opt = 0) const;
    
    /// create a Write with this property
    Wrist * newWrist(Mecable const*, unsigned) const;
    
    /// identifies the property
    std::string kind() const { return "single"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute derived parameter values
    void complete(SimulProp const*, PropertyList*);

    /// return a carbon copy of object
    Property* clone() const { return new SingleProp(*this); }
    
    /// write all values
    void write_data(std::ostream &) const;
    
};

#endif

