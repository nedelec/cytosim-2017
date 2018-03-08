// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SPACE_PROP_H
#define SPACE_PROP_H

#include "real.h"
#include "property.h"

class PointDisp;
class Glossary;
class Space;
class Simul;

#define NEW_DYNAMIC_SPACES 1

/// Property for Space
/**
 @ingroup Properties
 */
class SpaceProp : public Property
{
    friend class Space;

public:

    /**
     @defgroup SpacePar Parameters of Space
     @ingroup Parameters
     @{
    */
    
    /// shape followed by parameters (see @ref SpaceGroup)
    std::string  geometry;
    
    /// primitive (rectangle, etc), derived from geometry
    std::string  shape;
    
    /// sizes of the space, or name of input file or another dependecy as necessary
    std::string  dimensions;
    
    /// display string (see @ref PointDispPar)
    std::string  display;
    
#if NEW_DYNAMIC_SPACES
    
    /// Surface tension
    real		 tension;
    
    /// Target volume
    real         volume;
    
    /// Viscosity
    real		 viscosity;
    
    /// Viscosity for rotation
    real		 viscosity_rot;

#endif

    /// @}
    //------------------ derived variables below ----------------
    
    /// equal to time_step / viscosity
    real         mobility_dt, mobility_rot_dt;

    /// a word present in geometry
    std::string  spec;
    
    /// file name including a full path (derived from \a name)
    std::string  file;
    
    /// for display
    PointDisp *  disp;

public:

    /// constructor
    SpaceProp(const std::string& n) : Property(n), disp(0) { clear(); }

    /// destructor
    ~SpaceProp() { }
 
    /// create a new Space
    Space * newSpace() const;
    
    /// identifies the property
    std::string kind() const { return "space"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// check and derive parameters
    void complete(SimulProp const*, PropertyList*);
        
    
    /// return a carbon copy of object
    Property* clone() const { return new SpaceProp(*this); }
    
    /// write all values
    void write_data(std::ostream &) const;
    
};

#endif

