// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "node.h"
#include "node_list.h"

ObjectSet * Node::objset() const
{
    assert_true( nList );
    return nList->objset();
}

Node::~Node()
{
    /// the Node should have been unlinked first
    if ( nList )
    {
        //if ( nList->objset() )
        //    nList->objset()->remove(this);
        nList->pop(this);
    }
}
