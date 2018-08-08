// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef NUCLEATOR_H
#define NUCLEATOR_H

#include "hand.h"
class NucleatorProp;

/// A Hand that can nucleate a Fiber
/**
 The Nucleator is a Hand, and thus can bind and unbind from fibers,
 but in most cases, however you would want `binding_rate=0`.
 
 A free Nucleator can create new fibers with a prescibed rate.
 The rate, the type of fiber and the characteristics of the fiber are set
 as three values in property @ref NucleatorPar "nucleate".
 
 The parameter @ref NucleatorPar "specificity" allow to constrain
 the direction of the new fiber that is nucleated.
 
 By default the nucleator stays attached at the MINUS_END
 of the fiber that it has created.
 This can be changed by setting: @ref NucleatorPar "track_end"

 See Examples and the @ref NucleatorPar.
 @ingroup HandGroup
 
 */
class Nucleator : public Hand
{

    /// Gillespie time
    real     gspTime;
    
private:
    
    /// disabled default constructor
    Nucleator();
    
    /// Property
    NucleatorProp const* prop;
    
public:
    
    /// constructor
    Nucleator(NucleatorProp const* p, HandMonitor* h);

    /// destructor
    ~Nucleator() {}
    
    
    /// create a new Fiber
    void   nucleate(Vector pos);
    
    /// simulate when is not attached
    void   stepFree(const FiberGrid&, Vector const & pos);

    /// simulate when \a this is attached but not under load
    void   stepUnloaded();

    /// simulate when \a this is attached and under load
    void   stepLoaded(Vector const & force);
    
    /// detach from Fiber
    void   detach();

};

#endif

