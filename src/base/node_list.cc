// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// doubly linked list, STL style, with acces by iterators,
// some additions to manipulate the list: sorting, unsorting, etc.

#include "node_list.h"
#include "random.h"

//------------------------------------------------------------------------------
void NodeList::push_front(Node * n)
{
    assert_true( n->nList == 0 );
    //Cytosim::MSG("NodeList: pushFirst   %p in   %p\n", n, this);

    n->nPrev = 0;
    n->nNext = nFirst;
    if ( nFirst )
        nFirst->nPrev = n;
    else
        nLast = n;
    nFirst = n;
    n->nList = this;
    ++nSize;
}

//------------------------------------------------------------------------------
void NodeList::push_back(Node * n)
{
    assert_true( n->nList == 0 );
    //Cytosim::MSG("NodeList: pushLast   %p in   %p\n", n, this);
    
    n->nPrev = nLast;
    n->nNext = 0;
    if ( nLast )
        nLast->nNext = n;
    else
        nFirst = n;
    nLast = n;
    n->nList = this;
    ++nSize;
}

//------------------------------------------------------------------------------
void NodeList::transfer(NodeList& list)
{
    Node * n = list.nFirst;
    
    if ( nLast )
        nLast->nNext = n;
    else
        nFirst = n;
    
    if ( n )
    {
        n->nPrev = nLast;
        nLast = list.nLast;
    }
    
    while ( n )
    {
        n->nList = this;
        n = n->nNext;
    }
    nSize += list.nSize;
    
    list.nSize  = 0;
    list.nFirst = 0;
    list.nLast  = 0;
}

//------------------------------------------------------------------------------
void NodeList::push_after(Node * p, Node * n)
{
    assert_true( p->nList == this );
    assert_true( n->nList == 0 );

    n->nPrev = p;
    n->nNext = p->nNext;
    if ( p->nNext )
        p->nNext->nPrev = n;
    else
        nLast = n;
    p->nNext = n;
    n->nList = this;
    ++nSize;
}

//------------------------------------------------------------------------------
void NodeList::push_before(Node * p, Node * n)
{
    assert_true( p->nList == this );
    assert_true( n->nList == 0 );

    n->nNext = p;
    n->nPrev = p->nPrev;
    if ( p->nPrev )
        p->nPrev->nNext = n;
    else
        nFirst = n;
    p->nPrev = n;
    n->nList = this;
    ++nSize;
}

//------------------------------------------------------------------------------
Node * NodeList::pop_front()
{
    assert_true( nFirst );
 
    Node * n = nFirst;
    n->nList = 0;
    nFirst = nFirst->nNext;
    if ( nFirst )
        nFirst->nPrev = 0;
    else
        nLast = 0;
    --nSize;
    return n;
}

//------------------------------------------------------------------------------
void NodeList::pop(Node * n)
{
    assert_true( n->nList == this );
    assert_true( nSize > 0 );

    if ( n->nPrev )
        n->nPrev->nNext = n->nNext;
    else {
        assert_true( nFirst == n );
        nFirst = n->nNext;
    }
    
    if ( n->nNext )
        n->nNext->nPrev = n->nPrev;
    else {
        assert_true( nLast == n );
        nLast = n->nPrev;
    }
    
    n->nPrev = 0;
    n->nNext = 0;
    n->nList = 0;
    --nSize;
}


//------------------------------------------------------------------------------
void NodeList::clear()
{
    Node * n = nFirst;
    while ( n )
    {
        n->nList = 0;
        n = n->nNext;
    }
    nFirst = 0;
    nLast  = 0;
    nSize  = 0;
}

//------------------------------------------------------------------------------
void NodeList::erase()
{
    Node * n = nFirst;
    Node * p;
    while ( n )
    {
        p = n->nNext;
        delete(n);
        n = p;
    }
    nFirst = 0;
    nLast  = 0;
    nSize  = 0;
}


//------------------------------------------------------------------------------
/**
 Rearrange (first--P-Pnext--last) as (Pnext--last-first--P)
 */
 void NodeList::swap(Node * p)
{
    if ( p  &&  p->nNext )
    {
        nLast->nNext   = nFirst;
        nFirst->nPrev  = nLast;
        nFirst         = p->nNext;
        nLast          = p;
        nLast->nNext   = 0;
        nFirst->nPrev  = 0;
    }
    assert_false( bad() );
}

//------------------------------------------------------------------------------
/**
 Rearrange (first--P-Pnext--Qprev-Q--last) as (Pnext--Qprev-first--P-Q--last)
 
 If q is between nFirst and p, this will destroy the list,
 but there is no way to check such condition here.
 */
void NodeList::shuffle1(Node * p, Node * q)
{
    assert_true( p  &&  p->nNext );
    assert_true( q  &&  q->nPrev );
    
    if ( q != p->nNext )
    {
        p->nNext->nPrev = 0;
        nFirst->nPrev   = q->nPrev;
        q->nPrev->nNext = nFirst;
        nFirst          = p->nNext;
        p->nNext        = q;
        q->nPrev        = p;
    }
    assert_false( bad() );
}

/**
 Rearrange (first--P-Pnext--Qprev-Q--last) as (first--P-Q--last-Pnext--Qprev)
 
 If q is between nFirst and p, this will destroy the list,
 but there is no way to check such condition here.
 */
void NodeList::shuffle2(Node * p, Node * q)
{
    assert_true( p  &&  p->nNext );
    assert_true( q  &&  q->nPrev );
    
    if ( q != p->nNext )
    {
        nLast->nNext    = p->nNext;
        p->nNext->nPrev = nLast;
        p->nNext        = q;
        nLast           = q->nPrev;
        q->nPrev->nNext = 0;
        q->nPrev        = p;
    }
    assert_false( bad() );
}

//------------------------------------------------------------------------------
void NodeList::mix(Random& rng)
{
    if ( nSize < 2 )
        return;
    
    unsigned int pp = rng.pint_exc(nSize);
    unsigned int qq = rng.pint_exc(nSize);

    unsigned int n = 0;
    Node *p = nFirst, *q;

    if ( pp+1 < qq )
    {
        for ( ; n < pp; ++n )
            p = p->nNext;
        for ( q = p; n < qq; ++n )
            q = q->nNext;
        
        shuffle1(p, q);
    }
    else if ( qq+1 < pp )
    {
        for ( ; n < qq; ++n )
            p = p->nNext;
        for ( q = p; n < pp; ++n )
            q = q->nNext;
        
        shuffle2(p, q);
    }
    else
    {
        for ( ; n < qq; ++n )
            p = p->nNext;

        swap(p);
    }
}

void NodeList::mix5(Random& rng)
{
    mix(rng);
    mix(rng);
    mix(rng);
    mix(rng);
    mix(rng);
}

//------------------------------------------------------------------------------
int NodeList::bad() const
{
    unsigned int cnt = 0;
    Node * p = first(), * q;
    
    if ( p  &&  p->nPrev != 0 )
        return 71;
    while ( p )
    {
        q = p->nNext;
        if ( q == 0 ) {
            if ( p != nLast )
                return 73;
        }
        else {
            if ( q->nPrev != p )
                return 74;
        }
        p = q;
        ++cnt;
    }
    
    if ( cnt != nSize )
        return 75;
    return 0;
}


