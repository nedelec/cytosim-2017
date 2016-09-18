// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef BUNDLE_PROP_H
#define BUNDLE_PROP_H

#include "real.h"
#include "property.h"
#include "fiber_naked.h"

class FiberProp;
class Glossary;

/// Property for Bundle
/**
 @ingroup Properties
*/
class BundleProp : public Property
{
    friend class Bundle;
    
public:
    
    /**
     @defgroup BundlePar Parameters of Bundle
     @ingroup Parameters
     @{
     */
    
    
    /// name of Fiber in the Bundle
    std::string   fibers;
    
    /// number of fibers in the Bundle
    unsigned int  nb_fibers;

    /// stiffness of the links that connect the overlapping fibers
    real          stiffness;
    
    /// length of the zone where fibers overlap
    real          overlap;
    
    /// designates which end of the fiber is towards the center
    FiberEnd      focus;
    
    /// if true, any missing fiber is replaced by a new one
    bool          nucleate;
    
    /// @}
    //------------------ derived variables below ----------------
    
private:
    
    FiberProp *   fiber_prop;
    

public:
 
    /// constructor
    BundleProp(const std::string& n) : Property(n)  { clear(); }
    
    /// destructor
    ~BundleProp() { }
    
    /// identifies the property
    std::string kind() const { return "bundle"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// check and derive parameters
    void complete(SimulProp const*, PropertyList*);
    
    
    /// return a carbon copy of object
    Property* clone() const { return new BundleProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
    
};

#endif

