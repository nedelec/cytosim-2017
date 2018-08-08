// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SPHERE_SET_H
#define SPHERE_SET_H

#include "object_set.h"
#include "sphere.h"
class Simul;

///a list of Sphere
class SphereSet : public ObjectSet
{
public:
    
    ///creator
    SphereSet(Simul& s) : ObjectSet(s) {}
    
    ///destructor
    virtual ~SphereSet() {}
    
    //--------------------------
    
    /// identifies the class
    std::string kind() const { return "sphere"; }
    
    /// create a new property for class \a kind with given name
    Property*   newProperty(const std::string& kind, const std::string& name, Glossary&) const;

    //--------------------------
    
    /// create a new object directly from a glossary
    ObjectList  newObjects(const std::string& kind, const std::string& prop, Glossary&);

    /// construct object
    Object *    newObjectT(const Tag tag, int prop_index);
    
    
    /// register object
    void        add(Object *);
    
    /// remove object
    void        remove(Object *);
    
    /// erase all Object and all Property
    void        erase();

    /// first Solid
    Sphere *    first() const
    {
        return static_cast<Sphere*>(nodes.first());
    }
    
    /// return pointer to the Object of given Number, or zero if not found
    Sphere *    find(Number n) const
    {
        return static_cast<Sphere*>(inventory.get(n));
    }
    
    /// modulo the position (periodic boundary conditions)
    void        foldPosition(const Modulo * s) const;
    
    /// Monte-Carlo simulation step for every Object
    void        step() {}
 };

#endif
