// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef ORGANIZER_SET_H
#define ORGANIZER_SET_H

#include "object_set.h"
#include "organizer.h"

class Simul;
class Aster;

/// a list of Organizer (Aster, Nucleus, Bundle)
class OrganizerSet : public ObjectSet
{
public:
    
    ///creator
    OrganizerSet(Simul& s) : ObjectSet(s) {}
    
    ///destructor
    virtual ~OrganizerSet() {}
    
    //--------------------------
    
    /// identifies the class
    std::string kind() const { return "organizer"; }
    
    /// create a new property for class \a kind with given name
    Property*   newProperty(const std::string& kind, const std::string& name, Glossary&) const;
    
    //--------------------------
    
    /// create a new object directly from a glossary
    ObjectList  newObjects(const std::string& kind, const std::string& prop, Glossary&);

    /// construct object
    Object *    newObjectT(const Tag tag, int);
    
    /// register Organizer
    void        add(Object *);
    
    /// first Organizer
    Organizer * first() const
    {
        return static_cast<Organizer*>(nodes.first());
    }
    
    /// find object with given Number
    Organizer * find(Number n) const
    {
        return static_cast<Organizer*>(inventory.get(n));
    }
    
    /// find Aster with given Number
    Aster *     findAster(Number n) const;
    
    /// modulo the position (periodic boundary conditions)
    void        foldPosition(const Modulo * s) const;

    /// Monte-Carlo simulation step for every Object
    void        step();
};


#endif


