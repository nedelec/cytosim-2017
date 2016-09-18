// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "sim.h"
#include "assert_macro.h"
#include "exceptions.h"
#include "glossary.h"
#include "iowrapper.h"
#include "single.h"
#include "simul.h"
#include "space.h"
#include "modulo.h"
#include "meca.h"


//------------------------------------------------------------------------------
Single::Single(SingleProp const* p, Vector const& w)
: prop(p), sHand(0), sPos(w)
{
    if ( p == 0 )
        throw Exception("Null Single::prop");
    
    assert_true(prop->hand_prop);
    sHand = prop->hand_prop->newHand(this);
    assert_true(sHand);
}


Single::~Single()
{
    if ( sHand  &&  sHand->attached() )
        sHand->FiberBinder::detach();
    
    if ( linked() )
        list()->pop(this);    
    
    if ( sHand )
    {
        delete(sHand);
        sHand = 0;
    }
    
    prop = 0;
}

//------------------------------------------------------------------------------
#pragma mark -

void Single::afterAttachment()
{
    assert_true( attached() );
    if ( linked() )
    {
        //assert_true( objset() );
        objset()->relink(this);
    }
}

/**
 This sets the position to where the Hand detaches
 */
void Single::beforeDetachment()
{
    assert_true( attached() );
    sPos = sHand->pos();
}

void Single::afterDetachment()
{
    assert_false( attached() );
    if ( linked() )
    {
        //assert_true( objset() );
        objset()->relink(this);
    }
}


real Single::interactionLength() const
{
    return prop->length;
}


//------------------------------------------------------------------------------
#pragma mark -


Vector Single::position() const
{
    if ( sHand->attached() )
        return sHand->pos();
    return sPos;
}

void Single::foldPosition(const Modulo * s)
{
    s->fold(sPos);
}


void Single::stepFree(const FiberGrid& grid)
{
    assert_false( sHand->attached() );

    // diffusion:
    sPos.addRand( prop->diffusion_dt );
    
    // confinement
    if ( prop->confine == CONFINE_INSIDE )
    {
        Space const* spc = prop->confine_space_ptr;
        assert_true(spc);
        if ( ! spc->inside(sPos) )
            spc->bounce(sPos);
    }
    else if ( prop->confine == CONFINE_SURFACE )
    {
        Space const* spc = prop->confine_space_ptr;
        assert_true(spc);
        spc->project(sPos);
    }
    
    sHand->stepFree(grid, sPos);
}


void Single::stepAttached()
{
    assert_true( sHand->attached() );
    
    sHand->stepLoaded(force());
}


bool Single::hasInteraction() const
{
    return false;
}

/**
 Add confinement force to the bound fiber
 */
void Single::setInteractions(Meca & meca) const
{
    assert_true( sHand->attached() );
}

//------------------------------------------------------------------------------
#pragma mark -

void Single::write(OutputWrapper& out) const
{
    sHand->write(out);
    out.writeFloatVector(sPos, DIM);
}


void Single::read(InputWrapper & in, Simul& sim)
{
    try
    {
        sHand->read(in, sim);
        in.readFloatVector(sPos, DIM);
    }
    catch( Exception & e ) {
        e << ", in Single::read()";
        throw;
    }
}


