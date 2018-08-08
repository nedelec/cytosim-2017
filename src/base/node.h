// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef NODE_H
#define NODE_H

#include "assert_macro.h"

class NodeList;
class ObjectSet;

/// Can be linked in a NodeList
/**
 This provides the necessary pointers to build doubly-linked lists:
 - nNext points to the next Node, and is null if *this is first in the list.
 - nPrev points to the previous Node, and is null if *this is last in the list.
 - nList is a back pointer to the current NodeList.
 .
 A given Node can only be part of one NodeList. 
 One can unlink the object with
 @code
 obj->list()->pop(obj);
 @endcode
*/

class Node
{
    friend class NodeList;

private:
        
    /// the list where this is linked
    NodeList  *nList;

protected:

    /// the next Node in the list
    Node      *nNext;
    
    /// the previous Node in the list
    Node      *nPrev;
        
public:

    /// constructor set as unlinked
    Node() : nList(0), nNext(0), nPrev(0) { }
    
    /// destructor
    virtual   ~Node();
    
    /// true if Node is linked
    bool       linked()    const { return nList != 0; }

    /// the next Node in the list, or zero if this is last
    Node*      next()      const { return nNext; }
    
    /// the previous Node in the list, or zero if this is first
    Node*      prev()      const { return nPrev; }
    
    /// the list where the object is linked, or zero
    NodeList*  list()      const { return nList; }

    /// return associated ObjectSet
    ObjectSet* objset()    const;
};


#endif
