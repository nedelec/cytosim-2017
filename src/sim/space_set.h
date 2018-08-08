// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SPACE_SET_H
#define SPACE_SET_H

#include "object_set.h"
#include "space.h"
class Simul;

///a list of Space
class SpaceSet : public ObjectSet
{
public:
    
    /// creator
    SpaceSet(Simul& s) : ObjectSet(s) {}
    
    /// destructor
    virtual ~SpaceSet() {}
    
    //--------------------------
    
    /// identifies the property
    std::string kind() const { return "space"; }

    /// create a new property for class \a kind with given name
    Property * newProperty(const std::string& kind, const std::string& name, Glossary&) const;
    
    //--------------------------
    
    /// add Object
    void add(Object *);
    
    /// remove Object
    void remove(Object *);

    /// erase all Object and all Property
    void erase();
    
    /// create a new object from the corresponding property
    ObjectList newObjects(const std::string& kind, const std::string& prop, Glossary& opt);
    
    /// construct object
    Object * newObjectT(const Tag tag, int prop_index);
    
    /// Monte-Carlo step for every Space
    void step();
    
    /// first Space
    Space * first() const
    {
        return static_cast<Space*>(nodes.first());
    }

    /// first Space with this Property
    Space * first(const Property * prop) const
    {
        return static_cast<Space*>(ObjectSet::first(prop));
    }

    /// return pointer to the Object of given Number, or zero if not found
    Space * find(Number n) const
    {
        return static_cast<Space*>(inventory.get(n));
    }
        
};


#endif

