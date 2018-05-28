// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "fiber_binder.h"
#include "fiber_locus.h"
#include "iowrapper.h"
#include "messages.h"
#include "simul.h"
#include "sim.h"

//------------------------------------------------------------------------------

FiberBinder::FiberBinder(Fiber* f, real a)
: fbFiber(f), fbAbs(a)
{
    assert_true(f);
    inter = f->interpolate(a);
}


void FiberBinder::relocate(Fiber* f)
{
    assert_true(f);
    if ( fbFiber )
        fbFiber->removeBinder(this);
    fbFiber = f;
    f->addBinder(this);
    updateBinder();
}


void FiberBinder::relocate(Fiber* f, real a)
{
    assert_true(f);
    if ( f != fbFiber )
    {
        if ( fbFiber )
            fbFiber->removeBinder(this);
        f->addBinder(this);
        fbFiber = f;
    }
    fbAbs = a;
    updateBinder();
}


//------------------------------------------------------------------------------
#pragma mark -

void FiberBinder::moveToEnd(const FiberEnd end)
{
    assert_true(fbFiber);
    assert_true(end==PLUS_END || end==MINUS_END || end==CENTER);
    
    fbAbs = fbFiber->abscissa(end);
    inter = fbFiber->interpolateEnd(end);
}


void FiberBinder::moveBy(const real dabs)
{
    assert_true(fbFiber);
    fbAbs += dabs;
    updateBinder();
    checkFiberRange();
}


void FiberBinder::moveTo(const real abs)
{
    assert_true(fbFiber);
    fbAbs = abs;
    updateBinder();
    checkFiberRange();
}



//------------------------------------------------------------------------------
#pragma mark -

void FiberBinder::attach(FiberBinder const& fb)
{
    assert_true(fbFiber==0);
    assert_true(fb.fbFiber);
    
    fbAbs   = fb.fbAbs;
    fbFiber = fb.fbFiber;
    
    assert_true(fbFiber->abscissaM() <= fbAbs);
    assert_true(fbAbs <= fbFiber->abscissaP());

    fbFiber->addBinder(this);
    updateBinder();
}


void FiberBinder::detach()
{
    assert_true( fbFiber );
    fbFiber->removeBinder(this);
    fbFiber = 0;
}


//------------------------------------------------------------------------------
#pragma mark -


void FiberBinder::checkFiberRange()
{
    assert_true(fbFiber);
    
    if ( fbAbs < fbFiber->abscissaM() )
        handleOutOfRange(MINUS_END);
    else if ( fbAbs > fbFiber->abscissaP() )
        handleOutOfRange(PLUS_END);
}


FiberEnd FiberBinder::nearestEnd() const
{
    assert_true(fbFiber);
    if ( 2 * fbAbs > fbFiber->abscissaM() + fbFiber->abscissaP() )
        return PLUS_END;
    else
        return MINUS_END;
}


real  FiberBinder::abscissaFrom(const FiberEnd from) const
{
    assert_true(fbFiber);
    switch( from )
    {
        case MINUS_END:  return fbAbs - fbFiber->abscissaM();
        case PLUS_END:   return fbFiber->abscissaP() - fbAbs;
        case ORIGIN:     return fbAbs;
        case CENTER:     return fbAbs - 0.5*( fbFiber->abscissaM() + fbFiber->abscissaP() );
        default:         ABORT_NOW("invalid argument value");
    }
    return 0;
}


//------------------------------------------------------------------------------
#pragma mark -

void FiberBinder::write(OutputWrapper& out) const
{
    out.writeSoftSpace();
    if ( fbFiber )
    {
        checkAbscissa();
        fbFiber->writeReference(out);
        out.writeFloat(fbAbs);
    }
    else {
        Object::writeNullReference(out);
    }
}


void FiberBinder::read(InputWrapper& in, Simul& sim)
{
    Fiber * oldFiber = fbFiber;
    
    Tag tag = 0;
    Object * w = sim.readReference(in, tag);

    if ( w )
    {
        //std::cerr << "FiberBinder::read() " << tag << std::endl;

        if ( tag == Fiber::TAG )
        {
            fbFiber = static_cast<Fiber*>(w);
            fbAbs   = in.readFloat();
        }
#ifdef BACKWARD_COMPATIBILITY
        else if ( tag == 'm' )
        {
            fbFiber = static_cast<Fiber*>(w);
            fbAbs   = in.readFloat();
        } 
#endif
        else
        {
            ///\todo: we should allow binder to refer to any Mecable
            throw InvalidIO("FiberBinder should be bound to a Fiber!");
        }
        
        // link the FiberBinder as in attach():
        if ( oldFiber != fbFiber )
        {
            if ( oldFiber )
                oldFiber->removeBinder(this);
            fbFiber->addBinder(this);
        }
        updateBinder();
        checkAbscissa();
    }
    else
    {
        if ( oldFiber )
            FiberBinder::detach();
    }
}

//------------------------------------------------------------------------------
#pragma mark -

void FiberBinder::checkAbscissa() const
{
    assert_true(fbFiber);
    
    if ( fbAbs < fbFiber->abscissaM() - 1e-2 )
        Cytosim::warning("FiberBinder:abscissa < fiber:abscissa(MINUS_END) :  %e\n", fbFiber->abscissaM()-fbAbs );
    
    if ( fbAbs > fbFiber->abscissaP() + 1e-2 )
        Cytosim::warning("FiberBinder:abscissa > fiber:abscissa(PLUS_END)  :  %e\n", fbAbs-fbFiber->abscissaP() );
}


int FiberBinder::bad() const
{
    if ( fbFiber != inter.object() )
    {
        std::cerr << "Interpolation mismatch " << fbFiber << " " << inter.object() << std::endl;
        return 7;
    }
    
    if ( fbFiber->within(fbAbs) )
    {    
        const real e = fbAbs - fbFiber->abscissaP(inter.point1()+inter.coef1());
        
        if ( fabs(e) > 1e-3 )
        {
            std::cerr << "Interpolation error " << std::scientific << e << std::endl;
            return 8;
        }
    }
    return 0;
}


