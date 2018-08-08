// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SINGLE_SET_H
#define SINGLE_SET_H

#include "object_set.h"
#include "single.h"
#include "single_prop.h"

/// Set for Single
/**
 A Single is stored in one of 2 NodeList, depending on its state:
 - fList = free,
 - aList = attached.
 .
 Each list is accessible via its head firstF() and firstA().
 This way, the state of the Single are known when accessing them.
 
 A Single is automatically transfered to the appropriate list,
 if its Hand binds or unbind. This is one role of HandMonitor:
 HandMonitor::afterAttachment() and HandMonitor::afterDetachment() 
 are called by the Hand, and calls SingleSet::relink().
 */
class SingleSet: public ObjectSet
{
    
private:
    
    /// List for non-attached Singles (f=free)
    NodeList     fList;
    
    /// List for attached Singles (a=attached)
    NodeList     aList;
    
    /// frozen lists
    NodeList     fIce, aIce;
        
    /// register a Single into the list
    void         link(Object *);
    
public:
        
    ///creator
    SingleSet(Simul& s) : ObjectSet(s), fList(this), aList(this) {}
    
    ///destructor
    virtual      ~SingleSet() {}
    
    //--------------------------

    /// identifies the class
    std::string   kind() const { return "single"; }

    /// create a new property for class \a kind with given name
    Property *    newProperty(const std::string& kind, const std::string& name, Glossary&) const;

    //--------------------------
     
    /// construct object
    Object *      newObjectT(const Tag tag, int prop_index);
    
    /// create a new object directly from a glossary
    ObjectList    newObjects(const std::string& kind, const std::string& prop, Glossary&);

    /// create Wrists anchored on given Mecable
    ObjectList    makeWrists(Mecable const*, unsigned, unsigned, std::string&);
    
    /// return all Wrists anchored on `obj`
    ObjectList    collectWrists(Object * obj) const;

    /// remove all Wrists anchored on Object 'obj'
    void          removeWrists(Object * obj);
    
    ///returns the first free Single
    Single *      firstF()     const { return static_cast<Single*>(fList.first()); }
    
    ///returns the first bound Single
    Single *      firstA()     const { return static_cast<Single*>(aList.first()); }
    
    /// return pointer to the Object of given Number, or zero if not found
    Single *      find(Number n)  const { return static_cast<Single*>(inventory.get(n)); }
    
    /// collect Object for which func(obj, val) == true
    ObjectList    collect(bool (*func)(Object const*, void*), void*) const;

    /// erase all Object and all Property
    void          erase();
    
    /// number of elements
    unsigned int  size() const;
    
    /// mix order of elements
    void          mix();

    /// transfer all object to ice
    void          freeze();
    
    /// delete objects, or put them back in normal list
    void          thaw(bool erase);

    /// Monte-Carlo step
    void          step(FiberSet const&, FiberGrid const&);
    
    /// write
    void          write(OutputWrapper&) const;
    
    /// modulo the position (periodic boundary conditions)
    void          foldPosition(const Modulo *) const;
    
    /// check internal consistency, returns 0 if everything is OK
    int           bad() const;

};


#endif

