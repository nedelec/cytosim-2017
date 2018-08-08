// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "assert_macro.h"
#include "bead.h"
#include "bead_prop.h"
#include "single_prop.h"
#include "exceptions.h"
#include "single.h"
#include "hand_prop.h"
#include "iowrapper.h"
#include "glossary.h"
#include "meca.h"
#include "simul.h"
#include "space.h"
#include "modulo.h"
extern Random RNG;

extern bool functionKey[];

//------------------------------------------------------------------------------

Bead::Bead(BeadProp const* p, Vector pos, real rad)
: prop(p), paRadius(rad), paDrag(0)
{
    paPos = pos;
    paForce.set(0,0,0);
    setDragCoefficient();
}


Bead::~Bead()
{
    //Cytosim::MSG(32, "destroying %c%lu\n", TAG, number());
    prop = 0;
}

real Bead::volume() const
{
#if ( DIM == 1 )
    return 2 * paRadius;
#elif ( DIM == 2 )
    return M_PI * paRadius * paRadius;
#else
    return 4 * M_PI / 3.0 * paRadius * paRadius * paRadius;
#endif
}

//------------------------------------------------------------------------------

void Bead::setInteractions(Meca & meca) const
{
    switch ( prop->confine )
    {
        case CONFINE_NOT:
            break;

        case CONFINE_INSIDE:
        {            
            // Confine only the center of gravity
            Space const* spc = prop->confine_space_ptr;
            if ( !spc->inside(paPos) )
                spc->setInteraction(paPos, PointExact(this, 0), meca, prop->confine_stiff);
        } break;
        
        case CONFINE_ALL_INSIDE:
        {
            // Confine the entire bead
            Space const* spc = prop->confine_space_ptr;
            if ( !spc->allInside(paPos, paRadius) )
                spc->setInteraction(paPos, PointExact(this, 0), paRadius, meca, prop->confine_stiff);
        } break;
        
        case CONFINE_OUTSIDE:
        {
            // confine the center outside
            Space const* spc = prop->confine_space_ptr;
            if ( spc->inside(paPos) )
                spc->setInteraction(paPos, PointExact(this, 0), meca, prop->confine_stiff);
        } break;
            
        case CONFINE_SURFACE:
        {
            Space const* spc = prop->confine_space_ptr;
            spc->setInteraction(paPos, PointExact(this, 0), meca, prop->confine_stiff);
        } break;
        
        default:
            throw InvalidParameter("Invalid bead::confine");
    }
}

void Bead::foldPosition(const Modulo * s)
{
    //modulo around the first point, which is the center
    s->fold( paPos );
}


real Bead::addBrownianForces(real* rhs, real sc) const
{
    // Brownian amplitude:
    real b = sqrt( 2 * sc * paDrag );

    for ( unsigned int jj = 0; jj < DIM*nbPoints(); ++jj )
        rhs[jj] += b * RNG.gauss();
    
    //the amplitude is needed in Meca
    return b / paDrag;
}


/**
 Bead follow Stokes' law.
 
 Translation:
 @code
   muT = 6*M_PI*viscosity*soRadius[pp];
   muT * dx/dt = force
 @endcode
 Rotation:
 @code
   muR = 8*M_PI*viscosity*cub(soRadius[pp])
   muR * d(angle)/dt = momentum-of-forces
 @endcode
 */
void Bead::setDragCoefficient()
{
    paDrag = 6 * M_PI * prop->viscosity * paRadius;
#if ( 0 )
    static bool virgin = true;
    if ( paRadius > 0  &&  virgin )
    {
        std::cerr << "bead     radius   = " << paRadius << std::endl;
        std::cerr << "         drag     = " << paDrag << std::endl;
        std::cerr << "         mobility = " << 1.0/paDrag << std::endl;
        virgin = false;
    }
#endif
}


/**
 The projection is trivial
 */
void Bead::setSpeedsFromForces(const real* X, real* Y, const real sc, bool) const
{
    assert_true( paDrag > 0 );
    real s = sc / paDrag;
    for ( int d = 0; d < DIM; ++d )
        Y[d] = s * X[d];
}


//------------------------------------------------------------------------------

void Bead::write(OutputWrapper& out) const
{
    out.writeFloatVector(paPos, DIM, '\n');
    out.writeSoftSpace(2);
    out.writeFloat(paRadius);
}


void Bead::read(InputWrapper & in, Simul&)
{
    try {

        in.readFloatVector(paPos, DIM);
        real r = in.readFloat();
        resize(r);
        
    }
    catch( Exception & e ) {
        
        e << ", in Particle::read()";
        throw;
        
    }
}

