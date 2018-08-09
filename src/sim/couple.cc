// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "sim.h"
#include "couple.h"
#include "assert_macro.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "hand_prop.h"
#include "meca.h"
#include "simul.h"
#include "space.h"
#include "modulo.h"
#include "aster.h"
#include "aster_prop.h"

extern Random RNG;
extern Modulo * modulo;

//------------------------------------------------------------------------------

Couple::Couple(CoupleProp const* p, Vector const& w)
:  prop(p), cPos(w), cHand1(0), cHand2(0)
{
    if ( p == 0 )
        throw Exception("Null Couple::prop");

    cHand1 = prop->hand_prop1->newHand(this);
    cHand2 = prop->hand_prop2->newHand(this);

    assert_true( cHand1 );
    assert_true( cHand2 );    
}


Couple::~Couple()
{
    if ( cHand1  &&  cHand1->attached() )
        cHand1->FiberBinder::detach();
    
    if ( cHand2  &&  cHand2->attached() )
        cHand2->FiberBinder::detach();
    
    if ( linked() )
        list()->pop(this);
    
    if ( cHand1 )
    {
        delete(cHand1);
        cHand1 = 0;
    }
    if ( cHand2 )
    {
        delete(cHand2);
        cHand2 = 0;
    }
    
    prop = 0;
}


//------------------------------------------------------------------------------

void Couple::setProperty(CoupleProp * p)
{
    prop = p;
    
    if ( cHand1 )
        delete(cHand1);
    cHand1 = prop->hand_prop1->newHand(this);
    
    if ( cHand2 )
        delete(cHand2);
    cHand2 = prop->hand_prop2->newHand(this);
}


//------------------------------------------------------------------------------
#pragma mark -

real Couple::stiffness() const
{
    return prop->stiffness;
}


void Couple::setInteractions(Meca & meca) const
{
    assert_true( cHand1->attached() && cHand2->attached() );
    
    meca.interLink(cHand1->interpolation(), cHand2->interpolation(), prop->stiffness);
}

//------------------------------------------------------------------------------
#pragma mark -

void Couple::stepFF(const FiberGrid& grid)
{
    assert_true( !attached1() && !attached2() );
    
    // diffusion step:
    cPos.addRand( prop->diffusion_dt);
    
    // confinement:
    if ( prop->confine == CONFINE_INSIDE )
    {
        const Space* spc = prop->confine_space_ptr;
        assert_true(spc);
        if ( ! spc->inside(cPos) )
            spc->bounce(cPos);
    }
    else if ( prop->confine == CONFINE_SURFACE )
    {
        const Space* spc = prop->confine_space_ptr;
        assert_true(spc);
        spc->project(cPos);        
    }    

    // activity (attachment):
    cHand1->stepFree(grid, cPos);
    cHand2->stepFree(grid, cPos);
}


void Couple::stepAF(const FiberGrid& grid)
{
    assert_true( attached1() && !attached2() );
    //we use cHand1->pos() first, because stepUnloaded() may detach cHand1
    cHand2->stepFree(grid, cHand1->pos());
    cHand1->stepUnloaded();
}


void Couple::stepFA(const FiberGrid& grid)
{
    assert_true( !attached1() && attached2() );
    //we use cHand2->pos() first, because stepUnloaded() may detach cHand2
    cHand1->stepFree(grid, cHand2->pos());
    cHand2->stepUnloaded();
}


void Couple::stepAA()
{
    assert_true( attached1() && attached2() );
    
    Vector f = force1();
    cHand1->stepLoaded( f);
    cHand2->stepLoaded(-f);
}



//------------------------------------------------------------------------------
#pragma mark -

/**
 If ( prop->stiff == true ):
 Check if binding is directly adjacent to the place where the other Hand
 of the complex is already attached.
 Such 'degenerate' link are allowed if the parameter prop->stiff is false.  
 They cannot produce forces but change the cooperativity of Couple binding.
 
 
 */

bool Couple::allowAttachment(const FiberBinder & fb)
{
    if ( prop->stiff )
    {
        const Hand * other = attachedHand();
        
        if ( other == 0 )
            return true;
        
        if ( other->fiber() == fb.fiber()
            && fabs(fb.abscissa()-other->abscissa()) < 2*fb.fiber()->segmentation() )
        return false;
    
#if ( 0 )
        /*
         Test here if binding would create a link inside an aster, near the center:
         i.e. a link between two Fibers from the same aster, very close to the center
         of this aster. Such links would be improductive, and would trap the Couples.
         */
        const Organizer * org = fb.fiber()->organizer();
        
        if ( org  &&  org->tag() == Aster::TAG )
        {
            Fiber * fib = other->fiber();
            if ( fib->organizer() == org )
            {
                real rad = static_cast<const Aster*>(org)->prop->radius[0];
                if (  fb.abscissa() < rad  &&  other->abscissa() < rad )
                    return false;
            }
        }
#endif
    }

    //attachment is allowed:
    return true;
}


void Couple::afterAttachment()
{
    if ( linked() )
    {
        //assert_true( objset() );
        objset()->relink(this);
    }
}


void Couple::beforeDetachment()
{
    //we set the position of the complex, just before detaching:
    if ( cHand1->attached() )
        cPos = cHand1->pos();
    else 
        cPos = cHand2->pos();
}


void Couple::afterDetachment()
{
    if ( linked() )
    {
        //assert_true( objset() );
        objset()->relink(this);
    }
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 The position is:
 - cPos if the Couple is free,
 - the position of the attached Hand if only one is attached
 - the average position of the two hands if they are both attached
.
 */
Vector Couple::position() const
{
    if ( cHand2->attached() )
    {
        if ( cHand1->attached() )
            return 0.5 * ( cHand2->pos() + cHand1->pos() );
        return cHand2->pos();
    }
    if ( cHand1->attached() )
    {
        return cHand1->pos();
    }
    return cPos;
}


void Couple::foldPosition(const Modulo * s)
{
    modulo->fold(cPos);
}

void Couple::randomizePosition()
{
    cPos = prop->confine_space_ptr->randomPlace();
}

//------------------------------------------------------------------------------
#pragma mark -

Vector Couple::force1() const
{
    Vector d = cHand2->pos() - cHand1->pos();
    
    //correct for periodic space:
    if ( modulo )
        modulo->fold(d);
    
    return prop->stiffness * d;
}


Hand * Couple::attachedHand() const
{
    if ( attached1() )
        return cHand1;
    else if ( attached2() )
        return cHand2;
    else
        return 0;
}


Hand* Couple::otherHand(Hand * h) const
{
    if ( h == cHand1 )
        return cHand2;
    else
        return cHand1;
}


Vector Couple::otherDirection(Hand * h) const
{
    if ( h == cHand1 )
    {
        if ( cHand2->attached() )
            return cHand2->dirFiber();
        else
            return Vector::randUnit();
    }
    else
    {
        if ( cHand1->attached() )
            return cHand1->dirFiber();
        else
            return Vector::randUnit();
    }
}


real Couple::interactionLength() const
{
    return prop->length;
}

//------------------------------------------------------------------------------
#pragma mark -


void Couple::write(OutputWrapper& out) const
{
    cHand1->write(out);
    cHand2->write(out);
    if ( !cHand1->attached() && !cHand2->attached() )
        out.writeFloatVector(cPos, DIM);
}


void Couple::read(InputWrapper & in, Simul& sim)
{
    try {

        cHand1->read(in, sim);
        cHand2->read(in, sim);
        
        if ( cHand1->attached() || cHand2->attached() )
            cPos = position();
        else
            in.readFloatVector(cPos, DIM);
        
    }
    catch( Exception & e ) {
        e << ", in Couple::read()";
        throw;
    }
}

//------------------------------------------------------------------------------

int Couple::whichLinkAA() const
{
    // Hands within 10 nm are considered end-bound (see testDetachment() )
    int endLinks = (hand1()->abscissaFrom(PLUS_END) < 0.010) + (hand2()->abscissaFrom(PLUS_END) < 0.010);
    int linkType;
    
    if (endLinks == 0)
        if (cosAngle() > 0.5)
            linkType = 0; // H-P: angle < PI/3
        else if (cosAngle() < -0.5)
            linkType = 1; // H-AP: angle > 2PI/3
        else
            linkType = 2; // X
        else if (endLinks == 1)
            linkType = 3; // T
        else if (endLinks == 2)
            linkType = 4; // V
        else
            linkType = 5; // ?
    
    return linkType;
}


int Couple::whichLinkAF() const
{
    int endLink = 0;
    
    // Hands within 10 nm are considered end-bound (see testDetachment() )
    if (attachedHand()->abscissaFrom(PLUS_END) < 0.010)
        endLink = 1;
    return endLink;
}
