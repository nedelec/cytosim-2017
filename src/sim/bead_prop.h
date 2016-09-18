// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef BEAD_PROP_H
#define BEAD_PROP_H

#include "real.h"
#include "property.h"
#include "common.h"

class Glossary;
class PointDisp;
class Space;


/// Property for Bead and Solid
/*
 @ingroup Properties
 */
class BeadProp : public Property
{
    friend class Bead;
    friend class Solid;
    
public:
    
    /**
     @defgroup BeadPar Parameters of Solid and Bead
     @ingroup Parameters
     @{
     */
    
    
    /// effective viscosity (if not specified, simul:viscosity is used)
    real          viscosity;
    
    /// flag to include steric interactions
    int           steric;
    
    /// flag to confine the object
    Confinement   confine;
    
    /// confinement stiffness (this is specified as \c confine[1])
    real          confine_stiff;
    
    /// name of space for confinement (this is specified as \c confine[2])
    std::string   confine_space;
    
    /// display string (see @ref PointDispPar)
    std::string   display;
    
    /// @}
    //------------------ derived variables below ----------------
    
    /// parameters derived from string \a display
    PointDisp *   disp;
    
private:
    
    Space const*  confine_space_ptr;
    
public:
    
    /// constructor
    BeadProp(const std::string& n) : Property(n), disp(0) { clear(); }
    
    /// destructor
    ~BeadProp() { }
    
    /// identifies the property
    std::string kind() const { return "bead"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// derive parameters
    void complete(SimulProp const*, PropertyList*);
    
    /// return a carbon copy of object
    Property* clone() const { return new BeadProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
   
};

#endif

