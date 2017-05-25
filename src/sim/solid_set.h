// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SOLID_SET_H
#define SOLID_SET_H

#include "object_set.h"
#include "solid.h"

class Simul;

/// a list of Solid
class SolidSet : public ObjectSet
{
public:
    
    /// creator
    SolidSet(Simul& s) : ObjectSet(s) {}
    
    /// destructor
    virtual ~SolidSet() {}
    
    //--------------------------
    
    /// identifies the class
    std::string kind() const { return "solid"; }

    /// create a new property for class \a kind with given name
    Property*   newProperty(const std::string& kind, const std::string& name, Glossary&) const;

    //--------------------------
  
    /// create a new object directly from a glossary
    ObjectList  newObjects(const std::string& kind, const std::string& prop, Glossary&);

    /// construct object
    Object *    newObjectT(const Tag tag, int prop_index);
    
    
    /// register a Solid into the list
    void        add(Object *);
    
    /// remove from the list
    void        remove(Object *);
    
    /// erase all Object and all Property
    void        erase();
    
    /// first Solid
    Solid *     first() const
    {
        return static_cast<Solid*>(nodes.first());
    }
        
    /// return pointer to the Object of given Number, or zero if not found
    Solid *     find(Number n) const
    {
        return static_cast<Solid*>(inventory.get(n));
    }
    
    /// modulo the position (periodic boundary conditions)
    void        foldPosition(const Modulo *) const;
    
    /// Monte-Carlo simulation step for every Object
    void        step();
};


#endif

