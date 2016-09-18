// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef NUCLEUS_PROP_H
#define NUCLEUS_PROP_H

#include "real.h"
#include "property.h"
#include "fiber_naked.h"

class FiberProp;
class SphereProp;
class BundleProp;
class Glossary;


/// Property for Nucleus
/**
 @ingroup Properties
*/
class NucleusProp : public Property
{
    friend class Nucleus;
        
public:
    
    /**
     @defgroup NucleusPar Parameters of Nucleus
     @ingroup Parameters
     @{
     */
    
   
    /// name of sphere
    std::string   sphere;
    
    /// name of fibers 
    std::string   fibers;
    
    /// number of fibers
    unsigned int  nb_fibers;
    
    /// rate at which a new fiber is created at an unoccupied site
    real          nucleation_rate;

    /// stiffness of assembly links
    real          stiffness;
    
    /// designates which end of the fiber is attached to the sphere
    FiberEnd      focus;
    
    /// name of bundles
    std::string   bundles;
    
    /// number of bundles
    unsigned int  nb_bundles;
    
    /// @}
    //------------------ derived variables below ----------------
    
private:
    
    FiberProp *   fiber_prop;
    SphereProp *  sphere_prop;
    BundleProp *  bundle_prop;
    real          nucleation_rate_prob;

public:
 
    /// constructor
    NucleusProp(const std::string& n) : Property(n)  { clear(); }
    
    /// destructor
    ~NucleusProp() { }
    
    /// identifies the property
    std::string kind() const { return "nucleus"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// check and derive parameters
    void complete(SimulProp const*, PropertyList*);
    
    
    /// return a carbon copy of object
    Property* clone() const { return new NucleusProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
    
};

#endif

