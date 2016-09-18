// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef NODE_LIST_H
#define NODE_LIST_H

#include "node.h"
class ObjectSet;
class Random;

/// Doubly linked list of Nodes
/**
This class is similar to STL::dequeue<OBJ>
and the naming of the functions is consistent with STL whenever possible.
 
 NodeList has pointers to the first and last elements of the list.
 The List also keeps track of how many objects are linked.
 Functions are given to link and unlink Nodes in constant time.\n
 
 A function mix() randomize the order of the Nodes in the list. 
 Such randomization are necessary in the simulation
 to avoid any bias which could derive from a fixed ordering.
 
 The list is zero-terminated on both sides, and it can be traversed in either ways:
 for ( Node * n = first(); n ; n = n->next() );
 for ( Node * n = last(); n ; n = n->prev() );
 */

class NodeList
{
    
private:
        
    /// First Node of the list
    Node *          nFirst;
    
    /// Last Node of the list
    Node *          nLast;
    
    /// Number of Node in the list
    unsigned int    nSize;
    
    /// Disabled copy constructor
    NodeList(NodeList const&);
    
    /// Disabled copy assignment
    NodeList& operator=(NodeList const&);
    
protected:
    
    /// Pointer to embedding ObjectSet 
    ObjectSet * nSet;
    
public:
    
    /// Constructor
    NodeList() : nFirst(0), nLast(0), nSize(0), nSet(0) { }
    
    /// Constructor
    NodeList(ObjectSet * s) : nFirst(0), nLast(0), nSize(0), nSet(s) { }

    /// Destructor
    virtual         ~NodeList()         { erase(); }
    
    /// First Node
    Node *          first()       const { return nFirst; }
    
    /// First Node
    Node *          last()        const { return nLast; }
    
    /// Number of objects in the list
    unsigned int    size()        const { return nSize; }
    
    /// true if no element
    bool            empty()       const { return nFirst == 0; }
    
    /// return associated ObjectSet
    ObjectSet *     objset()      const { return nSet; }
    
    /// put Node first in the list
    void            push_front(Node *);
    
    /// put Node last in the list
    void            push_back(Node *);
    
    /// import all objects from given list, and empty it
    void            transfer(NodeList& list);
    
    /// put new Node np after existing one p
    void            push_after(Node * p, Node * np);
    
    /// put new Node np before existing one p
    void            push_before(Node * p, Node * np);
    
    /// Remove Node op from the list
    void            pop(Node * op);
    
    /// Remove Node op from the list
    Node *          pop_front();

    
    /// clear the list by calling pop(first) until empty
    virtual void    clear();
    
    /// clear the list as above, calling delete( ) for each node
    virtual void    erase();
    
    /// Rearrange (first--P-Pnext--last) as (Pnext--last-first--P)
    void            swap(Node * p);
    
    /// Rearrange (first--P-Pnext--Qprev-Q--last) as (Pnext--Qprev-first--P-Q--last)
    void            shuffle1(Node * p, Node * q);
    
    /// Rearrange (first--P-Pnext--Qprev-Q--last) as (first--P-Q--last-Pnext--Qprev)
    void            shuffle2(Node * p, Node * q);
    
    /// Mix using swap() and shuffle() functions  
    void            mix(Random&);
    
    /// call mix() five times  
    void            mix5(Random&);

    /// test coherence of list
    int             bad() const;
};


#endif
