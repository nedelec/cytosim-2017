// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "picket.h"
#include "simul.h"
#include "meca.h"
#include "modulo.h"


extern Modulo * modulo;

//------------------------------------------------------------------------------
Picket::Picket(SingleProp const* p, Vector const& w)
: Single(p, w)
{
#if ( 0 )
    if ( p->diffusion > 0 )
        throw InvalidParameter("single:diffusion cannot be > 0 if activity=fixed");
#endif
}

//------------------------------------------------------------------------------
Picket::~Picket()
{
    //std::cerr<<"~Picket("<<this<<")"<<std::endl;
}

//------------------------------------------------------------------------------
void Picket::stepFree(const FiberGrid& grid)
{
    assert_false( sHand->attached() );

    sHand->stepFree(grid, sPos);
}

//------------------------------------------------------------------------------
void Picket::stepAttached()
{
    assert_true( sHand->attached() );
    
    sHand->stepLoaded(force());
}


//------------------------------------------------------------------------------
/**
 This calculates the force corresponding to interClamp()
 */
Vector Picket::force() const
{
    if ( sHand->attached() )
    {
        Vector d = sPos - posHand();
        
        if ( modulo )
            modulo->fold(d);
        
        return prop->stiffness * d;
    }
    return Vector(0,0,0);
}

//------------------------------------------------------------------------------
void Picket::setInteractions(Meca & meca) const
{
    assert_true( prop->length == 0 );
    meca.interClamp(sHand->interpolation(), sPos, prop->stiffness);
    //meca.interSlidingClamp( sHand->interpolation(), sPos, prop->stiffness);
}


