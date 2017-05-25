// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef OBJECT_SET_H
#define OBJECT_SET_H

#include "node.h"
#include "object.h"
#include "node_list.h"
#include "inventory.h"
#include <vector>

class OutputWrapper;
class Property;
class PropertyList;
class Glossary;
class Simul;
extern Random RNG;

/// A set of Object
/**
 Encapsulates the different functions used to manage Objects.
 Pointers to the Objects are stored in two lists:
 - a doubly linked list: nodes
 - an array: inventory
 .
 The NodeList nodes is mixed at every time step,
 and thus it can be used to access the objects in a random order,
 as necessary for Monte-Carlo. 
 
 The Inventory can be used to access objects directly.
 
 Functions are used to manage:
 - object creation: newProperty(), newObjects().
 - object lists: size(), add(), remove(), relink(), erase().
 - object access: first(), find().
 - simulation: step(), mix().
 - I/O: readObject(), read(), write(), freeze(), thaw().
 .
 */
class ObjectSet
{
private:
    
    ObjectSet();
    
public:

    /// holds pointers to the Objects in a array of Numbers
    Inventory         inventory;
    
    /// holds pointers to the Objects in a doubly linked list
    NodeList          nodes;
    
    /// the Simul containing this ObjectSet
    Simul&            simul;
        
protected:
    
    /// a list used to store the objects temporarily while a state is imported
    NodeList          ice;
    
    /// remove all nodes in the list from the inventory
    void              forget(NodeList&);
    
public:
    
    /// transfer all nodes to list \a ice
    virtual void      freeze();
    
    /// erase objects, or put them back in normal list
    virtual void      thaw(bool erase);
    
    /// apply translation to all Objects in ObjectList
    static void       translateObjects(ObjectList const&, Vector const&);
    
    /// apply rotation to all Objects in ObjectList
    static void       rotateObjects(ObjectList const&, Rotation const&);

protected:
    
    /// collect Object from NodeList for which func(obj, val) == true
    static ObjectList collect(const NodeList&, bool (*func)(Object const*, void*), void*);

    /// write Object in NodeList to file
    static void       write(const NodeList&, OutputWrapper&);
    
    /// place all Objects in ObjectList using the same combinations of translation/rotation
    static Vector     placeObjects(ObjectList const&, Glossary& opt, const Space*);
    
    /// link the object last in the list
    virtual void      link(Object *);
    
public:
    
    /// creator
    ObjectSet(Simul& s) : nodes(this), simul(s), ice(0) { }
    
    /// destructor
    virtual ~ObjectSet() { erase(); }    
    
    //--------------------------
    
    /// identifies the sort of objects stored in the set
    virtual std::string kind() const { return "undefined"; };

    /// create a new property for class \a kind with given name
    virtual Property*  newProperty(const std::string& kind, const std::string& name, Glossary&) const = 0;
    
    //--------------------------
    
    /// create new objects, given Property and options in \a opt
    virtual ObjectList newObjects(const std::string& kind, const std::string& name, Glossary& opt) = 0;
    
    /// create new objects, translate and rotate them according to specifications in \a opt
    virtual ObjectList newPlacedObjects(const std::string& kind, const std::string& name, Glossary& opt);
    
    /// create a non-initialized Object with the corresponding Tag (used for reading trajectory file)
    virtual Object *   newObjectT(const Tag, int prop_index) = 0;
    
    //--------------------------
    
    /// register Object, and add it at the end of the list
    virtual void       add(Object *);
    
    /// remove Object in ObjectList
    void               add(ObjectList&);
    
    /// remove Object
    virtual void       remove(Object *);

    /// remove Object in ObjectList
    void               remove(ObjectList&);
    
    /// unlink and relink object. This places it last in the list
    virtual void       relink(Object *);

    /// remove Object, and delete it
    void               erase(Object *);

    /// delete all Objects in list and forget all serial numbers
    virtual void       erase();
    
    /// number of elements
    virtual unsigned   size()                 const { return nodes.size(); }

    /// mix the order of elements in the doubly linked list nodes
    virtual void       mix()                        { nodes.mix(RNG); }
    
    /// first Object in the list
    Object *           first()                const { return static_cast<Object*>(nodes.first()); }
    
    /// return an Object which has this property
    Object *           first(const Property*) const;
    
    /// last Object
    Object *           last()                 const { return static_cast<Object*>(nodes.last()); }
    
    /// find Object of given serial-number (see Inventoried)
    Object *           find(const Number n)   const { return static_cast<Object*>(inventory.get(n)); }
    
    /// return Object with serial-number \a if ( n > 0 ) or object from the end of the list if ( n <= 0 )
    Object *           findObject(long n) const;
    
    /// collect Object for which func(obj, val) == true
    virtual ObjectList collect(bool (*func)(Object const*, void*), void*) const;

    //--------------------------
    
    /// read one Object from file
    void               readObject(InputWrapper&, Tag, char pretag);

    /// write all Objects to file
    virtual void       write(OutputWrapper& out) const { write(nodes, out); }
    
};

#endif
