// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef ASTER_PROP_H
#define ASTER_PROP_H

#include "real.h"
#include "property.h"
#include "fiber_naked.h"
#include "solid_prop.h"

class FiberSet;
class Glossary;


/// Property for Aster
/**
 @ingroup Properties
 
 Note: the Aster can be constrained by confining the Solid around which it is built.
 */
class AsterProp : public Property
{
    friend class Aster;
    
public:
    
    /**
     @defgroup AsterPar Parameters of Aster
     @ingroup Parameters
     @{
     */
    
    
    /// name of Solid located at the center of the Aster to which each Fiber is attached
    std::string   solid;
    
    /// name of Fiber that make up the Aster
    std::string   fibers;
    
    /// stiffness of links between the Solid and the Fiber
    /**
     - stiffness[0] is the link between the central point of the Solid 
       and the focus-end of the Fiber
     - stiffness[1] is the link between a secondary point of the Solid and an
       intermediate point of the Fiber. This holds the Fiber in direction.
     .
    */
    real          stiffness[2];
    
    /// designates which end of the fiber is towards the center
    FiberEnd      focus;
    
    /// rate at which a new fiber is created at an unoccupied site
    real          nucleation_rate;
    
    /// @}
    //------------------ derived variables below ----------------
    
private:
    
    SolidProp *  solid_prop;
    FiberSet *   fiber_set;
    real         nucleation_rate_prob;

public:
    
    /// constructor
    AsterProp(const std::string& n) : Property(n)  { clear(); }
    
    /// destructor
    ~AsterProp() { }
    
    /// identifies the property
    std::string kind() const { return "aster"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// check and derive parameters
    void complete(SimulProp const*, PropertyList*);
    
    /// return a carbon copy of object
    Property* clone() const { return new AsterProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
    
};

#endif

