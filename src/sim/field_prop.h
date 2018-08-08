// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FIELD_PROP_H
#define FIELD_PROP_H

#include "real.h"
#include "property.h"

class Glossary;
class Space;

/// Property for Field
/**
 @ingroup Properties
*/
class FieldProp : public Property
{
    friend class FieldSet;
    
public:
    
    /**
     @defgroup FieldPar Parameters of Field
     @ingroup Parameters
     @{
     */
    
    
    /// name of confining Space
    std::string   confine_space;

    /// size of square unit cell
    real          step;
    
    /// diffusion constant
    real          diffusion;
    
    /// decay rate per unit time
    real          decay_rate;
    
    /// if > 0, the simulation is stopped in any cell becomes negative
    /** The test is done just after writing the field to file */
    int           positive;
    
    /// if false, the field is not recorded in the trajectory file
    bool          save;

    /// @}
    //------------------ derived variables below ----------------

public:
    
    real          diffusion_theta;
    real          decay_rate_dt;
    
    Space const*  confine_space_ptr;

public:
    
    /// constructor
    FieldProp(const std::string& n) : Property(n)  { clear(); }

    /// destructor
    ~FieldProp() { }
    
    /// identifies the property
    std::string kind() const { return "field"; }
        
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// derive parameters
    void complete(SimulProp const*, PropertyList*);
    
    /// return a carbon copy of object
    Property* clone() const { return new FieldProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
    
};

#endif

