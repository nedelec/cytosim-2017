// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "wrist.h"
#include "simul.h"
#include "meca.h"
#include "modulo.h"


extern Modulo * modulo;


//------------------------------------------------------------------------------
Wrist::Wrist(SingleProp const* p, Mecable const* mec, unsigned pti)
: Single(p), sBase(mec, pti)
{
    if ( mec && pti > mec->nbPoints() )
        throw InvalidParameter("Could not anchor Single (invalid point index)");
    
#if ( 0 )
    if ( p->diffusion > 0 )
        throw InvalidParameter("single:diffusion cannot be > 0 if activity=anchored");
#endif
}

//------------------------------------------------------------------------------
Wrist::~Wrist()
{
}


//------------------------------------------------------------------------------
Vector Wrist::force() const
{
    if ( sHand->attached() )
    {
        Vector d = sBase.pos() - sHand->pos();
    
        if ( modulo )
            modulo->fold(d);
        
        return prop->stiffness * d;
    }
    return Vector(0,0,0);
}

//------------------------------------------------------------------------------
void Wrist::stepFree(const FiberGrid& grid)
{
    assert_false( sHand->attached() );

    sHand->stepFree(grid, sBase.pos());
}

//------------------------------------------------------------------------------
void Wrist::stepAttached()
{
    assert_true( sHand->attached() );
    
    sHand->stepLoaded(force());
}


//------------------------------------------------------------------------------
void Wrist::setInteractions(Meca & meca) const
{
    meca.interLink(sHand->interpolation(), sBase, prop->stiffness);
}


//------------------------------------------------------------------------------

void Wrist::write(OutputWrapper& out) const
{
    sHand->write(out);
    sBase.write(out);
}


void Wrist::read(InputWrapper & in, Simul& sim)
{
    try
    {
        sHand->read(in, sim);
        sBase.read(in, sim);
    }
    catch( Exception & e ) {
        e << ", in Single::read()";
        throw;
    }
}

