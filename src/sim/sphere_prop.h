// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SPHERE_PROP_H
#define SPHERE_PROP_H

#include "real.h"
#include "property.h"
#include "common.h"

class Glossary;
class PointDisp;
class Space;


/// Property for Sphere
/**
 @ingroup Properties
*/
class SphereProp : public Property
{
    friend class Sphere;

public:
   
    /**
     @defgroup SpherePar Parameters of Sphere
     @ingroup Parameters
     @{
     */
    
    
    /// mobility of points on the surface
    real          point_mobility;
    
    /// effective viscosity (if not specified, simul:viscosity is used)
    real          viscosity;
    
    /// if true, use special formula to calculate mobilities
    /**
     This formula is derived from Lubrication theory and applies
     in the case where the sphere fits tightly in an elongated volume.
     */
    bool          piston_effect;
    
    /// flag to include steric interaction for this object
    int           steric;
    
    /// flag to confine this object
    Confinement   confine;
    
    /// confinement stiffness (this is specified as \c confine[1])
    real          confine_stiff;
    
    /// name of space for confinement (this is specified as \c confine[2])
    std::string   confine_space;
    
    /// display parameters (see @ref PointDispPar)
    std::string   display;
    
    /// @}
    //------------------ derived variables below ----------------
    
    /// parameters derived from string \a display
    PointDisp *   disp;

private:
    
    Space const*  confine_space_ptr;

public:
        
    /// constructor
    SphereProp(const std::string& n) : Property(n), disp(0) { clear(); }

    /// destructor
    ~SphereProp() { }
    
    /// identifies the property
    std::string kind() const { return "sphere"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// check and derive parameters
    void complete(SimulProp const*, PropertyList*);

    /// return a carbon copy of object
    Property* clone() const { return new SphereProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
    
};

#endif

