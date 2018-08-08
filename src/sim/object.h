// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef OBJECT_H
#define OBJECT_H

#include "inventoried.h"
#include "movable.h"
#include "array.h"
#include "node.h"

class Simul;
class Property;
class InputWrapper;
class OutputWrapper;


/// Type for unique class identifier used to read/write from objects from file
typedef char Tag;

/// Parent class for all simulated objects
/**
 This is the interface used for writing / reading from a file.
 
 Three functions identify an Object:
 - tag() [ASCII character] identifies the class of Object.
 - property->index() [integer] identifies its Property.
 - number() [serial-number] derived from Inventoried identifies unique instantiations.
 .
 These three qualities are concatenated in reference() and writeReference().
 
 Objects are stored in ObjectSet.
 */
class Object : public Node, public Inventoried, public Movable
{
    
    friend class ObjectSet;
    
private:
    
    /// integer used for custom tasks, which is recorded to file
    int          oMark;
    
    /// another integer that can be used for custom tasks, but this one is not saved
    mutable int  oFleck;
    
public:
    
    /// Object::TAG = 'v' represents the 'void' pointer
    static const Tag TAG = 'v';
    
    /// build a string reference by concatenating (tag, property_index, number, mark)
    static std::string strReference(char, int, Number, int);

    /// read a reference (property_index, number, mark) from input
    static void        readReference(InputWrapper&, int&, Number&, int&, char pretag);
    
public:
    
    /// constructor
    Object() : oMark(0), oFleck(0) {}
    
    /// an ASCII character identifying the ObjectSet corresponding to this object
    virtual Tag     tag() const = 0;
    
    /// Property associated with the Object
    virtual const Property* property() const = 0;
    
    /// write Object data to file
    virtual void    write(OutputWrapper&) const = 0;
    
    /// read Object from file, within the Simul
    virtual void    read(InputWrapper&, Simul&) = 0;
    
    
    /// concatenation of [ tag(), property()->index(), number() ] in plain ascii
    std::string     reference() const;
    
    /// write a reference that identifies the Object uniquely
    void            writeReference(OutputWrapper &) const;
    
    /// write a reference, but using the provided Tag
    void            writeReference(OutputWrapper &, Tag tag) const;
    
    /// write a reference that does not refer to any Object
    static void     writeNullReference(OutputWrapper &);
    
    
    /// birth mark value
    int             mark()       const { return oMark; }
    
    /// set birth mark
    void            mark(int m)        { oMark = m; }
    
    
    /// fleck value (not stored in trajectory files)
    int             fleck()      const { return oFleck; }
    
    /// set fleck (the value is not stored in trajectory files)
    void            fleck(int f) const { oFleck = f; }

    
    /// extends Node::next(), with a cast to preserve type
    Object *        next()       const { return static_cast<Object*>(nNext); }
    
    /// extends Node::prev(), with a cast to preserve type
    Object *        prev()       const { return static_cast<Object*>(nPrev); }
    
};



/// a list of Object
typedef Array<Object *> ObjectList;
//typedef std::vector<Object *> ObjectList;


#endif
