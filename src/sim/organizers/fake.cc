// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "assert_macro.h"
#include "exceptions.h"
#include "glossary.h"
#include "messages.h"
#include "organizer_set.h"
#include "point_exact.h"
#include "fake.h"
#include "aster.h"
#include "meca.h"



void Fake::step()
{
}


void Fake::setInteractions(Meca & meca) const
{
    assert_true( linked() );
    
    for (unsigned int ii = 0; ii < asterPoints.size(); ++ii )
        meca.interLink(asterPoints[ii], solidPoints[ii], prop->stiffness);
}


//------------------------------------------------------------------------------
ObjectList Fake::build(Glossary& opt, Simul& simul)
{
    assert_true(prop);
    Aster * a  = new Aster(prop->aster_prop);
    Aster * p  = new Aster(prop->aster_prop);
    Solid * so = new Solid(0);
    mSolid = so;
    
    ObjectList res;

    res.push_back(a);
    res.push_back(p);
    res.push_back(so);
    res.append(a->build(opt, simul));
    res.append(p->build(opt, simul));

    real span = 0;
    if ( ! opt.set(span, "span") || span < 0 )
        throw InvalidParameter("fake:span must be specified and >= 0");

    a->translate(Vector(-0.5*span, 0, 0));
    p->translate(Vector(+0.5*span, 0, 0));    
    
    Solid * sa = a->solid();
    Solid * sp = p->solid();
    so->prop = sa->prop;
    
    Vector wa = a->position();
    Vector wp = p->position();
    Vector dir1 = ( wa - wp ).orthogonal(1);
#if ( DIM == 3 )
    Vector dir2 = vecProd(wa-wp, dir1).normalized();
#else
    Vector dir2 = dir1;
#endif
    asterPoints.clear();
    solidPoints.clear();
    
    real rad = 0;
    if ( ! opt.set(rad, "radius", 2) ||  rad <= 0 )
        throw InvalidParameter("fake:radius[2] must be specified and > 0");

    Vector s(0,0,0);
    for ( int pt = 0; pt < DIM+(DIM==3); ++pt )
    {
        switch( pt )
        {
            case 0: s=+dir1; break;
            case 1: s=-dir1; break;
            case 2: s=+dir2; break;
            case 3: s=-dir2; break;
            default:
                ABORT_NOW("wrong number of points");
        }
        solidPoints.push_back(PointExact(so, so->addSphere( wa+s, rad )));
        asterPoints.push_back(PointExact(sa, sa->addPoint( wa+s )));
        solidPoints.push_back(PointExact(so, so->addSphere( wp+s, rad )));
        asterPoints.push_back(PointExact(sp, sp->addPoint( wp+s )));
    }
    
    sa->fixShape();
    sp->fixShape();
    so->fixShape();
    
    return res;
}


Fake::~Fake()
{
    //Cytosim::MSG(31, "destroying %s\n", reference().c_str() );
    prop = 0;
}

