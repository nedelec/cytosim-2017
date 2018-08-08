// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef NUCLEATOR_PROP_H
#define NUCLEATOR_PROP_H

#include "hand_prop.h"
#include "common.h"

class FiberProp;
class FiberSet;


/// additional Property for Nucleator
/**
 @ingroup Properties
 */
class NucleatorProp : public HandProp
{
public:
    
    friend class Nucleator;
    
    /// indicates a specificity
    enum Specificity
    {
        NUCLEATE_ORIENTATED,
        NUCLEATE_PARALLEL, 
        NUCLEATE_ANTIPARALLEL
    };
    
public:
    
    /**
     @defgroup NucleatorPar Parameters of Nucleator
     @ingroup Parameters
     Inherits @ref HandPar.
     Check the examples!
     @{
     */
    
    /// rate for nucleation (also known as \c nucleate[0])
    real         rate;

    /// type of fiber that is nucleated (also known as \c nucleate[1])
    std::string  fiber;
    
    /// specifications of a new fiber (also known as \c nucleate[2])
    /**
     Options valid for the command 'new fiber' may be specified here:
     see @ref FiberGroup.
     */
    std::string  spec;
    
    /// specifies the direction of the new Fiber
    /**
     The `specificity` can be:
     - none (default)
     - parallel
     - antiparallel
     .
     
     With 'specificity=none', the direction will follow the value of 'orientation',
     specified within `nucleation_spec`.
     */
    Specificity  specificity;
    
    /// option to track a specified end [none, plus_end, minus_end, nearest_end]
    /**
     By default the Hand will remain attached at the MINUS_END of any fiber that it has nucleated.
     But other values are possible:
     - none
     - plus_end
     - minus_end
     - nearest_end
     .
     */
    FiberEnd     track_end;
    
    /// if true, set the Dynamic State of the nearest filament end to STATE_RED upon detachment
    bool         addictive;
    
    /// @}
    //------------------ derived variables below ----------------

    /// global Simul
    Simul * simul;

private:
    
    real         rate_dt;
    
    
public:
    
    /// constructor
    NucleatorProp(const std::string& n) : HandProp(n)  { clear(); }
    
    /// destructor
    ~NucleatorProp() { }
    
    /// return a Hand with this property
    virtual Hand * newHand(HandMonitor* h) const;
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute values derived from the parameters
    void complete(SimulProp const*, PropertyList*);
    
    /// return a carbon copy of object
    Property* clone() const { return new NucleatorProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
   
};

#endif

