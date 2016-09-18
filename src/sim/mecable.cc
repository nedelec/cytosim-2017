// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "mecable.h"
#include "exceptions.h"
#include "cblas.h"
#include "iowrapper.h"
#include "organizer.h"


Mecable::Mecable() : mIndex(0), pBlock(0), pBlockSize(0), pBlockUse(false)
{
}


real* Mecable::allocateBlock(unsigned size)
{
    if ( size > pBlockSize )
    {
        if ( pBlock )
            delete[] pBlock;
        pBlock = 0;
        
        /*  The first time, we allocate exactly what is demanded.
        but if allocation is required again, we allocate with some margin,
        because it means that this object is probably growing. */
        if ( 0 == pBlockSize )
            pBlockSize = size;
        else
            pBlockSize = size + 4;
        
        pBlock = new real[ pBlockSize * pBlockSize ];
    }
    return pBlock;
}


Mecable::~Mecable()
{
    if ( pBlock )
        delete[] pBlock;
}

