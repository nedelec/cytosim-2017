// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef PARTICLE_SET_H
#define PARTICLE_SET_H

#include "object_set.h"
#include "bead.h"

class Simul;

/// a list of Bead
class BeadSet : public ObjectSet
{
public:
    
    /// creator
    BeadSet(Simul& s) : ObjectSet(s) {}
    
    /// destructor
    virtual ~BeadSet() {}
    
    //--------------------------
    
    /// identifies the class
    std::string kind() const { return "bead"; }
    
    /// create a new property for class \a kind with given name
    Property*   newProperty(const std::string& kind, const std::string& name, Glossary&) const;

    //--------------------------
    
    /// create a new object directly from a glossary
    ObjectList  newObjects(const std::string& kind, const std::string& prop, Glossary&);

    /// construct object
    Object *    newObjectT(const Tag tag, int prop_index);
    
    
    /// register a Bead into the list
    void        add(Object *);
    
    /// remove from the list
    void        remove(Object *);
    
    /// erase all Object and all Property
    void        erase();
    
    /// first Solid
    Bead *      first() const
    {
        return static_cast<Bead*>(nodes.first());
    }
        
    /// find object from its Number
    Bead *      find(Number n) const
    {
        return static_cast<Bead*>(inventory.get(n));
    }
    
    /// modulo the position (periodic boundary conditions)
    void        foldPosition(const Modulo *) const;
    
    /// Monte-Carlo simulation step for every Object
    void        step() {}
};


#endif

