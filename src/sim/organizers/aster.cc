// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "assert_macro.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "messages.h"
#include "aster.h"
#include "solid.h"
#include "solid_prop.h"
#include "fiber_prop.h"
#include "pointsonsphere.h"
#include "point_exact.h"
#include "point_interpolated.h"
#include "glossary.h"
#include "simul.h"
#include "meca.h"
extern Random RNG;

extern bool functionKey[];

//------------------------------------------------------------------------------
void Aster::step()
{
    assert_true( linked() );
    assert_true( asClamp.size()+1 == nbOrganized() );
    
    // nucleation:
    Glossary opt;

    for ( unsigned ii = 0; ii < asClamp.size(); ++ii )
    {
        if ( 0 == fiber(ii) &&  RNG.test(prop->nucleation_rate_prob) )
        {
            AsterClamp const& clamp = asClamp[ii];
            Solid const* so = solid();
            const Vector pos = so->posPoint( clamp.clamp1 );
            const Vector dir = ( so->posPoint( clamp.clamp2 ) - pos ).normalized();
            
            ObjectList objs = prop->fiber_set->newObjects("fiber", prop->fibers, opt);
            if ( objs.size() )
            {
                Fiber * fib = static_cast<Fiber*>(objs[0]);
                fib->setStraight(pos, dir, clamp.clampA, prop->focus);
                objset()->simul.add(objs);
                grasp(fib, ii+1);
            }
        }
    }
}

//------------------------------------------------------------------------------
void Aster::setInteractions(Meca & meca) const
{
    assert_true( linked() );
    assert_true( asClamp.size()+1 == nbOrganized() );

    Solid const* so = solid();
    
    if ( so == 0 )
        return;
    
    for ( unsigned ii = 0 ; ii < asClamp.size(); ++ii )
    {
        Fiber * fib = fiber(ii);
        if ( fib )
        {
            //link a central point of the solid with the focus-end of the Fiber
            meca.interLink(PointExact(so, asClamp[ii].clamp1),
                           fib->exactEnd(prop->focus), prop->stiffness[0]);
            
            //attach another point of the Solid to hold the Fiber in direction
            PointInterpolated pti;
            if ( fib->length()  >  asClamp[ii].clampA )
            {
                meca.interLink(fib->interpolate(asClamp[ii].clampA, prop->focus),
                               PointExact(so, asClamp[ii].clamp2), prop->stiffness[1]);
            }
            else
            {
                FiberEnd tip = ( prop->focus == PLUS_END ? MINUS_END : PLUS_END );
                // link the opposite end to an interpolation of the two solid-points:
                real c = fib->length() / asClamp[ii].clampA;
                meca.interLink(PointInterpolated(so, asClamp[ii].clamp1, asClamp[ii].clamp2, c),
                               fib->exactEnd(tip), prop->stiffness[1]);
            }
        }
    }
}


//------------------------------------------------------------------------------
ObjectList Aster::build(Glossary& opt, Simul& simul)
{
    assert_true(prop);
    assert_true(nbOrganized()==0);
    
    Solid * so = buildSolid(opt, simul);
    grasp(so, 0);
    ObjectList res;
    res.push_back(so);
        
    for ( unsigned ii = 0; ii < asClamp.size(); ++ii )
        grasp(0, ii+1);

    for ( unsigned ii = 0; ii < asClamp.size(); ++ii )
    {
        ObjectList objs = prop->fiber_set->newObjects("fiber", prop->fibers, opt);
        if ( objs.size() )
        {
            Fiber * fib = static_cast<Fiber*>(objs[0]);
            grasp(fib, ii+1);
            const Vector pos = so->posPoint( asClamp[ii].clamp1 );
            const Vector dir = ( so->posPoint( asClamp[ii].clamp2 ) - pos ).normalized();
            fib->setStraight(pos, dir, prop->focus);
        }
        res.append(objs);
    }
    return res;
}

Aster::~Aster()
{
    //Cytosim::MSG(31, "destroying %c%lu\n", TAG, number() );
    prop = 0;
}


//------------------------------------------------------------------------------

/**
 @defgroup NewAster How to create an Aster
 @ingroup NewObject
 
 By default the aster creates a radial distribution of fiber,
 and only the radius need to be specified:
 
 @code
 new aster NAME
 {
   nb_fibers = INTEGER
   radius = OUTER_RADIUS, INNER_RADIUS
   ...
 }
 @endcode
 
 The configuration of the Aster can also be customized by providing
 points on which the fibers are attached.
 
 Syntax:
 @code
 new aster NAME
 {
   nb_fibers = INTEGER
   anchor0 = POINT, POINT
   anchor1 = POINT, POINT
   ...
 }
 @endcode
 
 Each POINT can be specified in two ways:
 - as a VECTOR, to specify the position of a new point,
 - as an #INDEX refereeing to an already defined point.
 .
 A point with index=0 is always added at the center of the Aster.
 
 Example:
 
 @code
 new aster centrosome
 {
   nb_fibers = 3
   anchor0 = point0, 1  0 0
   anchor1 = point0, 0  1 0
   anchor2 = point0, 0 -1 0
 }
 @endcode
 
 It is also possible to add point to the Solid, before adding the anchors.
 The syntax is the same as for customizing a solid (@ref NewSolid):
 
 @code
 new aster centrosome
 {
   nb_points = 3
   point0 = 0 -0.2 0, 0.2
   point1 = 0  0   0, 0.2
   point2 = 0 +0.2 0, 0.2

   nb_fibers = 3
   anchor0 = point0,  0.5 -0.2 0
   anchor1 = point1,  0.5  0   0
   anchor2 = point2,  0.5 +0.2 0
 }
 @endcode
 
 */
Solid * Aster::buildSolid(Glossary& opt, Simul& simul)
{
    assert_true(prop->solid_prop);
    Solid * so = new Solid(prop->solid_prop);
    so->build(opt, simul);

    unsigned cnt = 0;
    opt.set(cnt, "nb_fibers");
    if ( cnt < 1 )
        throw InvalidParameter("aster:nb_fibers must be specified and > 0");
    
    asClamp.resize(cnt);
    
    // points can be specified directly:
    if ( opt.has_key("anchor0") )
    {
        std::string str, var;
        Vector pos;
        
        for ( unsigned ii = 0; ii < cnt; ++ii )
        {
            var = "anchor" + sMath::repr(ii);
            unsigned pt1 = 0, pt2 = 0;
            
            opt.set(str, var);
            
            if ( str.substr(0,5) == "point" )
                pt1 = atoi(str.substr(5).c_str());
            else
            {
                if ( opt.set(pos, var) )
                    pt1 = so->addPoint(pos);
                else
                    throw InvalidParameter("aster::"+var+"[0] should be specified");
            }

            opt.set(str, var, 1);
            
            if ( str.substr(0,5) == "point" )
                pt2 = atoi(str.substr(5).c_str());
            else
            {
                if ( opt.set(pos, var, 1) )
                    pt2 = so->addPoint(pos);
                else
                    throw InvalidParameter("aster::"+var+"[1] should be specified");
            }

            if ( pt1 >= so->nbPoints() )
                throw InvalidParameter("point index aster::"+var+"[0] is out of range");
            if ( pt2 >= so->nbPoints() )
                throw InvalidParameter("point index aster::"+var+"[1] is out of range");

            asClamp[ii].set(pt1, pt2, (so->posPoint(pt1)-so->posPoint(pt2)).norm());
        }
    }
    else
    {
        unsigned center = 0;
        real rad[2] = { 0 };
        
        opt.set(rad, 2, "radius");
        
        if ( rad[0] <= 0 )
            throw InvalidParameter("aster:radius must be specified and > 0");
        
        // add a massive bead if needed:
        if ( so->dragCoefficient() < REAL_EPSILON )
            so->addSphere(Vector(0,0,0), rad[0]);

#if ( DIM == 1 )
        
        //we use one central point, and an additional one on each side:
        unsigned left  = so->addPoint( Vector(-rad[0], 0, 0) );
        unsigned right = so->addPoint( Vector(+rad[0], 0, 0) );
        
        for ( unsigned ii = 0; ii < cnt; ++ii )
            asClamp[ii].set(center,  ( ii & 1 ) ? left : right, rad[0]);
        
#elif ( DIM == 2 )
        
        real ang = 0;
        for ( unsigned ii = 0; ii < cnt; ++ii )
        {
            Vector P(cos(ang), sin(ang), 0);
            unsigned pt = so->addPoint((rad[0]+rad[1])*P);
            if ( rad[1] > 0 )
                asClamp[ii].set(so->addPoint(rad[1]*P), pt, rad[0]);
            else
                asClamp[ii].set(center, pt, rad[0]);
            ang  += 2 * M_PI / real(cnt);
        }
#else
        
        //we use PointOnSphere to distribute points on the sphere
        PointsOnSphere sphere(cnt);
        Vector P;
        for ( unsigned ii = 0; ii < cnt; ++ii )
        {
            sphere.copyCoordinatesOfPoint(P, ii);
            unsigned pt = so->addPoint((rad[0]+rad[1])*P);
            if ( rad[1] > 0 )
                asClamp[ii].set(so->addPoint(rad[1]*P), pt, rad[0]);
            else
                asClamp[ii].set(center, pt, rad[0]);
        }

#endif
    }
    
    so->fixShape();
    return so;
}


//------------------------------------------------------------------------------

void Aster::write(OutputWrapper& out) const
{
    Organizer::write(out);
    
    out.writeSoftNewLine();
    assert_true( asClamp.size() == nbOrganized()-1 );
    for ( unsigned ii = 0; ii < asClamp.size(); ++ii )
    {
        out.writeSoftSpace();
        out.writeUInt16(asClamp[ii].clamp1);
        out.writeUInt16(asClamp[ii].clamp2);
    }
}


void Aster::read(InputWrapper & in, Simul& sim)
{
#ifdef BACKWARD_COMPATIBILITY
    if ( in.formatID() < 40 )
        in.readUInt16();
#endif
    
    Organizer::read(in, sim);
    
    assert_true( nbOrganized() > 0 );
    assert_true( organized(0)->tag() == Solid::TAG );
    
    try
    {
        size_t nc = nbOrganized() - 1;
        const Solid * so = solid();

        asClamp.resize(nc);
        for ( unsigned ii = 0; ii < nc; ++ii )
        {
            unsigned p1 = in.readUInt16();
            unsigned p2 = in.readUInt16();
            asClamp[ii].clamp1 = p1;
            asClamp[ii].clamp2 = p2;
            assert_true( p1 < so->nbPoints() );
            assert_true( p2 < so->nbPoints() );
            asClamp[ii].clampA = so->diffPoints(p1,p2).norm();
        }
    }
    catch( Exception & e )
    {
        e << ", in Aster::read()";
        throw;
    }
}


//------------------------------------------------------------------------------

unsigned Aster::nbLinks() const
{
    return 2 * asClamp.size();
}


Vector Aster::posLink1(const unsigned x) const
{
    unsigned ii = x / 2;
    
    const Solid * so = solid();
    assert_true( so );
    assert_true( ii < asClamp.size() );
    
    if ( x & 1 )
        return so->posPoint(asClamp[ii].clamp1);
    else
        return so->posPoint(asClamp[ii].clamp2);
}


Vector Aster::posLink2(const unsigned x) const
{
    unsigned ii = x / 2;

    Fiber * fib = fiber(ii);

    if ( fib == 0 )
        return posLink1(x);
    
    if ( x & 1 )
        return fib->posEnd(prop->focus);
    else
        return fib->pos(asClamp[ii].clampA, prop->focus);
}

