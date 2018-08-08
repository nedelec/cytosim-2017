// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef TRACKER_PROP_H
#define TRACKER_PROP_H

#include "common.h"
#include "hand_prop.h"

/// additional Property for Tracker
/**
 @ingroup Properties
 */
class TrackerProp : public HandProp
{
    friend class Tracker;
        
public:
    
    /**
     @defgroup TrackerPar Parameters of Tracker
     @ingroup Parameters
     Inherits @ref HandPar.
     Check the examples!
     @{
     */
    
    /// if set, bind only specified end [minus_end, plus_end, nearest_end]
    FiberEnd   bind_end;
    
    /// distance from the fiber tip defining region where hand can bind (also set as \c bind_end[1])
    real       bind_end_range;
    
    /// if set, always follow specified end [minus-end, plus_end, nearest_end]
    FiberEnd   track_end;
    
    /// if true, only bind fiber tip if no other hand is bound
    bool       bind_only_free_end;
    
    /// if true, bind only to growing fiber ends
    bool       bind_only_growing_end;

    /// @}
    //------------------ derived variables below ----------------

public:
        
    /// constructor
    TrackerProp(const std::string& n) : HandProp(n)  { clear(); }
   
    /// destructor
    ~TrackerProp() { }
    
    /// return a Hand with this property
    virtual Hand * newHand(HandMonitor* h) const;
    
    /// set default values
    void clear();
        
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute values derived from the parameters
    void complete(SimulProp const*, PropertyList*);
    
    /// return a carbon copy of object
    Property* clone() const { return new TrackerProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
   
};

#endif

