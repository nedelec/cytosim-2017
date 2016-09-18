// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FAKE_PROP_H
#define FAKE_PROP_H

#include "real.h"
#include "property.h"
class AsterProp;
class Glossary;

/// Property for a Fake
/**
 @ingroup Properties
*/
class FakeProp : public Property
{
    friend class Fake;
    
public:
    
    /**
     @defgroup FakePar Parameters of Fake
     @ingroup Parameters
     @{
     */
    
    
    /// name of the two asters composing the fake
    std::string   asters;
    
    /// stiffness of assembly links
    real          stiffness;
    
    /// @}
    //------------------ derived variables below ----------------
    
private:
    
    AsterProp *   aster_prop;

public:
 
    /// constructor
    FakeProp(const std::string& n) : Property(n)  { clear(); }
    
    /// destructor
    ~FakeProp() { }
    
    /// identifies the property
    std::string kind() const { return "fake"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// check and derive parameters
    void complete(SimulProp const*, PropertyList*);
    
    
    /// return a carbon copy of object
    Property* clone() const { return new FakeProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
    
};

#endif

