// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FIELD_SET_H
#define FIELD_SET_H

#include "object_set.h"
#include "field.h"
class Simul;



/// a list of Field
/**
 FieldSet is not an usual ObjectSet, because Field is a templated class,
 and its instantiations do not derive from a unique parent 'Field'.

 
 Field is defined from templated class FieldBase
 */
class FieldSet : public ObjectSet
{
public:
    
    /// creator
    FieldSet(Simul& s) : ObjectSet(s) {}
    
    /// destructor
    virtual ~FieldSet() {}
    
    //--------------------------
    
    /// identifies the class
    std::string kind() const { return "field"; }
    
    /// create a new property for class \a kind with given name
    Property*   newProperty(const std::string& kind, const std::string& name, Glossary&) const;
    
    //--------------------------
    
    /// create a new object from the corresponding property
    ObjectList  newObjects(const std::string& kind, const std::string& prop, Glossary& opt);

    /// construct object
    Object *    newObjectT(const Tag tag, int);
    
    /// register a Field into the list
    void        add(Object *);
    
    /// remove from the list
    void        remove(Object *);
    
    /// first object
    Field *     first() const
    {
        return static_cast<Field*>(nodes.first());
    }
    
    ///  return pointer to the Object of given Number, or zero if not found
    Field *     find(Number n) const
    {
        return static_cast<Field*>(inventory.get(n));
    }
    
    /// get ready to do a step()
    void        prepare();
    
    /// Monte-Carlo simulation step for every Object
    void        step();

};


#endif
