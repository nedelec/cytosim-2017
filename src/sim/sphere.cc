// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "assert_macro.h"
#include "sphere.h"
#include "wrist.h"
#include "exceptions.h"
#include "rotation.h"
#include "meca.h"
#include "messages.h"
#include "glossary.h"
#include "point_exact.h"
#include "sphere_prop.h"
#include "object_set.h"
#include "space_prop.h"
#include "iowrapper.h"
#include "space.h"
#include "modulo.h"
#include "simul.h"
#include "sim.h"

extern Random RNG;

//------------------- construction and destruction ---------------------------

/**
 The Sphere is returned with no points
 */
Sphere::Sphere(SphereProp const* p)
: prop(p), spRadius(0), spMobility(0), spMobilityRot(0), spAllocated(0), spProj(0)
{
}


Sphere::Sphere(SphereProp const* p, real rad)
: prop(p), spRadius(rad), spMobility(0), spMobilityRot(0), spAllocated(0), spProj(0)
{
    if ( prop == 0 )
        throw InvalidParameter("Sphere:prop should be specified");
    
    if ( rad <= 0 )
        throw InvalidParameter("sphere:radius should be > 0");
    
    // center point
    assert_true( nbPoints() == 0 );
    addPoint( Vector(0,0,0) );
    
    // reference points to track the orientation of the sphere
    if ( DIM >= 2 )
        addPoint( Vector(spRadius,0,0) );
    if ( DIM == 3 ) {
        addPoint( Vector(0,spRadius,0) );
        addPoint( Vector(0,0,spRadius) );
    }
    
    // this only needs to be called once:
    setDragCoefficient();
}


Sphere::~Sphere()
{
    //free memory
    if ( spProj ) delete[] spProj;
    prop = 0;
}

#pragma mark -

/*
 if PointSet::allocatePoints() allocated memory, it will return the
 size of the new array, and we allocate the same size for other arrays.
 */
unsigned int Sphere::allocatePoints(unsigned int nbp)
{
    unsigned int ms = PointSet::allocatePoints(nbp);
    if ( ms )
    {
        //std::cerr << "Sphere::allocatePoints " << ms << std::endl;
        allocateProjection(ms);
    }
    return ms;
}


/*
 here 'cp' is the vector from the center to the point to be added,
 in other words, the position of the point in the local reference frame.
 */
unsigned Sphere::addSurfacePoint(Vector const& cp)
{
    if ( 8 * cp.norm() < spRadius )
        throw InvalidParameter("sphere:point is too far from the surface of radius ", spRadius);
    return addPoint(posPoint(0)+cp.normalized(spRadius));
}


/**
 @ingroup NewObject
 
 Specify radius and number of surface points of a Sphere:
 @code
 new sphere NAME
 {
 radius = REAL
 point0 = INTEGER, POSITION [, SINGLE_SPEC]
 }
 @endcode
 
 The `INTEGER` specifies the number of points created, and `POSITION` can be a
 `VECTOR`, or the string 'surface'.  Multiple `SINGLE_SPEC` can be specified.
 
 <h3> Add Singles to a Sphere </h3>
 
 The parameter 'attach' can be used to add Single to the points of a Solid:
 
 @code
 new sphere NAME
 {
 radius   = ...
 point0   = ...
 etc.
 attach   = SINGLE_SPEC [, SINGLE_SPEC] ...
 attach0  = SINGLE_SPEC [, SINGLE_SPEC] ...
 etc.
 }
 @endcode
 
 Where `SINGLE_SPEC` is string containing at most 3 words: `[INTEGER] NAME [each]`,
 where the `INTEGER` specifies the number of Singles, `NAME` specifies their name,
 and the optional word `each` species that the command applies to every point.
 
 The command `attach` applies to all the points of the Solid, while `attach0`,
 `attach1`, etc. apply to the points specified by `point0`, `point1`, etc.
 With `attach`, the Singles are distributed randomly on all the points,
 and if `each` is specified, the specification is repeated for each point.
 
 For example if `grafted` is the name of a Single, one can use:
 
 @code
 new solid NAME
 {
 attach0 = 1 grafted each
 attach1 = 10 grafted
 }
 @endcode
 */
ObjectList Sphere::build(Glossary & opt, Simul& simul)
{
    ObjectList res;
    std::string str;
    unsigned inp = 0, inx = 0, nbp = 1;
    
    // interpret each instruction as a command to add points:
    std::string var = "point0";
    while ( opt.has_key(var) )
    {
        inx = 0;
        nbp = 1;
        if ( opt.is_number(var) == 2 && opt.set(nbp, var) )
            ++inx;
        
        if ( nbp > 0 )
        {
            unsigned fip = nbPoints();
            // add 'nbp' points:
            for ( unsigned n = 0; n < nbp; ++n )
            {
                Vector vec(0,0,0);
                str = opt.value(var, inx);
                if ( str == "surface" )
                    vec = Vector::randUnit(radius());
                else
                {
                    std::istringstream iss(str);
                    vec = Movable::readPosition(iss, 0);
                    if ( 8 * vec.norm() < spRadius )
                        throw InvalidParameter(var+" cannot be brought to the Sphere surface");
                }
                addSurfacePoint(vec);
            }
            
            // attach Single to this set of points:
            ++inx;
            while ( opt.set(str, var, inx++) )
                res.append(simul.singles.makeWrists(this, fip, nbp, str));
            
            // attach Single to this set of points:
            inx = 0;
            var = "attach" + sMath::repr(inp);
            while ( opt.set(str, var, inx++) )
                res.append(simul.singles.makeWrists(this, fip, nbp, str));
        }
        
        // set next keyword:
        var = "point" + sMath::repr(++inp);
    }
    
    
    // attach Singles distributed over the surface points:
    inx = 0;
    while ( opt.set(str, "attach", inx++) )
        res.append(simul.singles.makeWrists(this, nbRefPts, nbSurfacePoints(), str));
    
    
    // final verification of the number of points:
    nbp = 0;
    if ( opt.set(nbp, "nb_points")  &&  nbp != nbPoints() )
    {
        throw InvalidParameter("could not find the number of points specified in solid:nb_points");
    }
    
    //std::cerr << *this << std::endl;
    return res;
}


//------------------------------------------------------------------------------
void Sphere::setInteractions(Meca & meca) const
{
    switch ( prop->confine )
    {
        case CONFINE_NOT:
            break;

        case CONFINE_INSIDE:
        {
            Space const* spc = prop->confine_space_ptr;
            
            Vector pos = posPoint(0);
            if ( !spc->inside(pos) )
                spc->setInteraction(pos, PointExact(this, 0), meca, prop->confine_stiff);
        } break;
        
        case CONFINE_ALL_INSIDE:
        {
            const Space* spc = prop->confine_space_ptr;
            
            //for the sphere, we only need to confine the center (point with index 0)
            Vector pos = posPoint(0);
            if ( ! spc->allInside(pos, spRadius) )
                spc->setInteraction(pos, PointExact(this, 0), spRadius, meca, prop->confine_stiff);
        } break;
        
        case CONFINE_SURFACE:
        {
            const Space* spc = prop->confine_space_ptr;
            spc->setInteraction(posPoint(0), PointExact(this, 0), meca, prop->confine_stiff);
        }
            
        default:
            throw InvalidParameter("Invalid sphere::confine");            
    }
}


void Sphere::foldPosition(const Modulo * s)
{
    //modulo around the first point, which is the center
    s->fold( psPos );
    for ( unsigned int p = 1; p < nbPoints(); p++ )
        s->fold( psPos+DIM*p, psPos );
}


void Sphere::resize(const real R)
{
    //std::cerr << "Sphere::resize " << R << std::endl;
    if ( R > 0 )
    {
        spRadius = R;
        //this only needs to be called once:
        setDragCoefficient();
        reshape();
    }
}

/**
 the mobility is that of a sphere in an infinite fluid (Stokes law)
 */
void Sphere::setDragCoefficientStokes()
{
    assert_true( spRadius > 0 );
    
    const real rad = spRadius;
    
    //hydrodynamic not corrected: infinite fluid is assumed
    spMobility    = 1.0 / ( 6 * M_PI * prop->viscosity * rad );
    spMobilityRot = 1.0 / ( 8 * M_PI * prop->viscosity * rad * rad * rad );

    //Cytosim::MSG("Mobility of Sphere is %.2e\n", spMobility);
}


/**
 Expect higher friction due to flow around the sphere in a narrow tube.
 This is only valid if (r -a)/a << 1, where r = radius of the tube, and
 a = radius of the sphere.
 
 The formula are taken from:
 <em>The Motion of a Closely-Fitting Sphere in a Fluid-Filled Tube</em>\n
 <b>P. Bungay and H. Brenner, Int. J. Multiphase Flow</b>\n
 Vol 1, pp. 25-56, 1973 (see 3.6, 4.68a and 5.11)
 */
void Sphere::setDragCoefficientPiston()
{
    assert_true( spRadius > 0 );
    assert_true( prop->confine_space_ptr );
    
    const real rad = spRadius;
    real cell_radius = prop->confine_space_ptr->length(1);
    real eps  = ( cell_radius - rad ) / rad;
    
    if ( eps <= 0 )
        throw InvalidParameter("Error: piston formula invalid if sphere is larger than the cell");

    if ( eps > 1 )
        throw InvalidParameter("Error: piston formula invalid if sphere and cylinder do not fit");

    spMobility    = 4*sqrt(sMath::power(eps,5)/2) / ( 9*M_PI*M_PI * prop->viscosity * rad );
    spMobilityRot = sqrt(eps/2) / ( 2*M_PI*M_PI * prop->viscosity * rad * rad * rad );
        
    //report the reduced mobility of the sphere:
    //Cytosim::MSG("Mobility of Sphere is %.2e, due to piston effect\n", spMobility);
}


void Sphere::setDragCoefficient()
{
    setDragCoefficientStokes();

    if ( prop->piston_effect )
    {
        if ( prop->confine_space_ptr )
            setDragCoefficientPiston();
        else
            Cytosim::MSG("Piston effect ignored because space is undefined\n");
    }
}


#pragma mark -

void Sphere::prepareMecable()
{
    // setDragCoefficient() was already called by the constructor
    //setDragCoefficient();
    
    assert_true( spMobility > 0 );
    assert_true( spMobilityRot > 0 );
    
    makeProjection();
}

//------------------------------------------------------------------------------

real Sphere::addBrownianForces(real* rhs, real sc) const
{
    real bT = sqrt( 2 * sc / spMobility );
    real bS = sqrt( 2 * sc / prop->point_mobility );
    
    Vector F(0, 0, 0);
    Torque T = nullTorque;
    
    real cx = psPos[0];
    real cy = psPos[1];
    real cz = psPos[2];
    
    for ( unsigned dp = DIM*nbRefPts; dp < DIM*nbPoints(); dp+=DIM )
    {
        Vector fp = Vector::randGauss(bS);
        F += fp;
        
        rhs[dp  ] += fp.XX;
        
#if   ( DIM == 2 )
        rhs[dp+1] += fp.YY;
        T += vecProd(Vector(psPos[dp]-cx, psPos[dp+1]-cy), fp);
#elif ( DIM == 3 )
        rhs[dp+1] += fp.YY;
        rhs[dp+2] += fp.ZZ;
        T += vecProd(Vector(psPos[dp]-cx, psPos[dp+1]-cy, psPos[dp+2]-cz), fp);
#endif
    }
    
    /*
     The Torque is distributed to the surface points.
     In 2D, there is one point, and the coefficient is therefore 1.
     in 3D, there are 3 points, but always one is parallel to the axis of the torque,
     and the decomposition over these 3 points gives a factor 2.
     */
    T /= - ( DIM - 1 ) * spRadius * spRadius;
    Vector R = vecProd(Vector(cx,cy,cz), T);
    
    for ( unsigned dp = DIM; dp < DIM*nbRefPts; dp+=DIM )
    {
#if   ( DIM == 2 )
        rhs[dp]   += R.XX - T * psPos[dp+1] + bT * RNG.gauss();
        rhs[dp+1] += R.YY + T * psPos[dp  ] + bT * RNG.gauss();
        F += vecProd(T, Vector(psPos[dp]-cx, psPos[dp+1]-cy));
#elif ( DIM == 3 )
        rhs[dp  ] += R.XX + T.YY * psPos[dp+2] - T.ZZ * psPos[dp+1] + bT * RNG.gauss();
        rhs[dp+1] += R.YY + T.ZZ * psPos[dp  ] - T.XX * psPos[dp+2] + bT * RNG.gauss();
        rhs[dp+2] += R.ZZ + T.XX * psPos[dp+1] - T.YY * psPos[dp  ] + bT * RNG.gauss();
        F += vecProd(T, Vector(psPos[dp]-cx, psPos[dp+1]-cy, psPos[dp+2]-cz));
#endif
    }
    
#if   ( DIM == 2 )
    rhs[0] -= F.XX + bT * RNG.gauss();
    rhs[1] -= F.YY + bT * RNG.gauss();
#elif ( DIM == 3 )
    rhs[0] -= F.XX + bT * RNG.gauss();
    rhs[1] -= F.YY + bT * RNG.gauss();
    rhs[2] -= F.ZZ + bT * RNG.gauss();
#endif
    
    return std::min(bT*spMobility, bS*prop->point_mobility);
}

//------------------------------------------------------------------------------

void Sphere::orthogonalizeRef()
{
#if ( DIM == 3 )
    
    const int i = RNG.pint_exc(3);
    const int ix = 1+i, iy = 1+(i+1)%3, iz = 1+(i+2)%3;
    
    psCenter.set(psPos[0], psPos[1], psPos[2]);
    assert_true( nbPoints() >= nbRefPts );
    
    // reduce to the center of mass an normalize
    Vector tmpX =   posPoint(ix) - psCenter;
    Vector tmpY =   posPoint(iy) - psCenter;
    Vector tmpZ = ( posPoint(iz) - psCenter ).normalized();
    
    // make tmpY orthogonal to tmpZ, and normalized
    tmpY -= (tmpZ*tmpY) * tmpZ;
    tmpY.normalize();
    
    // make tmpX orthogonal to tmpZ and tmpY
    tmpX -= (tmpZ*tmpX) * tmpZ + (tmpY*tmpX) * tmpY;
    
    // normalize all to radius
    tmpX.normalize(spRadius);
    tmpY *= spRadius;
    tmpZ *= spRadius;
    
    // put the corrected vectors back into the array
    for ( unsigned int ii=0; ii < 3; ii++)
    {
        psPos[DIM*ix+ii] = tmpX[ii] + psCenter[ii];
        psPos[DIM*iy+ii] = tmpY[ii] + psCenter[ii];
        psPos[DIM*iz+ii] = tmpZ[ii] + psCenter[ii];
    }
#endif
}


/**
 we get rid of finite-step errors but conserve the shape
 by projecting back onto the sphere,
 without changing the position of point zero (the center)
*/
void Sphere::reshape()
{
    assert_true( nbPoints() > 0 );
    assert_true( spRadius > 0 );
    Vector axis;
    psCenter.set(psPos[0], psPos[1], psPos[2]);
    
    for ( unsigned int j = 1; j < nbPoints(); ++j ) 
    {
        axis = ( posPoint(j) - psCenter ).normalized( spRadius );
        setPoint( j, psCenter + axis );
    }
    
    if ( DIM == 3 )
        orthogonalizeRef();
}


//------------------------------------------------------------------------------
//------------------- methods for the projection -------------------------------
#pragma mark -


void Sphere::allocateProjection(const unsigned int nbp)
{
    //std::cerr << "Sphere::allocateProjection(" << nbp << ")" << std::endl;
    if ( spAllocated < nbp )
    {        
        if ( spProj ) delete[] spProj;
        
        spAllocated = nbp;
        spProj = new real[DIM*spAllocated];
    }
}


#if (DIM == 1)

//this is unsafe, don't use the sphere in 1D!
void Sphere::makeProjection() { ABORT_NOW("Sphere is not implemented in 1D"); }
void Sphere::addSurfaceSpeedsFromForces(real const*, real*, real) const {}
void Sphere::setSpeedsFromForces(const real* X, real* Y, const real, bool) const {}

#elif ( DIM == 2 || DIM == 3 )

/**
 prepare variables for the projection 
 */
void Sphere::makeProjection()
{
    //allocate more memory if needed
    allocateProjection(nbPoints());
    assert_true( spAllocated >= nbPoints() );
    assert_true( nbPoints() >= nbRefPts );

    //copy center of sphere:
    psCenter.set(psPos[0], psPos[1], psPos[2]);

    //preparations for the motion of the Surfaces:
    //the reference points will be omitted!
        
    //calculate the nonzero components of j, omitting factors of 2
    real curv = 1.0 / spRadius;
    for ( unsigned int p = nbRefPts; p < nbPoints(); ++p)
    {
        for ( int d = 0; d < DIM; ++d)
            spProj[DIM*p+d] = curv * ( psPos[DIM*p+d] - psPos[d] );
    }
}


void Sphere::setSphereSpeedsFromForces(const real* X, real* Y, const real sc) const
{
    Vector F(0,0,0);
#if   ( DIM == 2 )
    real T = 0;
#elif ( DIM == 3 )
    Vector T(0,0,0);
#endif
    
    for ( unsigned int p = 0; p < nbPoints(); p++ )
    {
        F.XX +=  X[DIM*p  ];
        F.YY +=  X[DIM*p+1];
#if   ( DIM == 2 )
        T    += psPos[DIM*p] * X[DIM*p+1] - psPos[DIM*p+1] * X[DIM*p];
#elif ( DIM == 3 )
        F.ZZ +=  X[DIM*p+2];
        T.XX += psPos[DIM*p+1] * X[DIM*p+2] - psPos[DIM*p+2] * X[DIM*p+1];
        T.YY += psPos[DIM*p+2] * X[DIM*p  ] - psPos[DIM*p  ] * X[DIM*p+2];
        T.ZZ += psPos[DIM*p  ] * X[DIM*p+1] - psPos[DIM*p+1] * X[DIM*p  ];
#endif
    }
    
    T -= vecProd( psCenter, F );       // reduce the torque to the center of mass
    T *= sc*spMobilityRot;             // multiply by the mobility and maybe dt
    F  = sc*spMobility*F + vecProd( psCenter, T );
    
    for ( unsigned int p = 0; p < nbPoints(); p++ )
    {
#if   ( DIM == 2 )
        Y[DIM*p]   = F.XX - T * psPos[DIM*p+1];
        Y[DIM*p+1] = F.YY + T * psPos[DIM*p];
#elif ( DIM == 3 )
        Y[DIM*p  ] = F.XX + T.YY * psPos[DIM*p+2] - T.ZZ * psPos[DIM*p+1];
        Y[DIM*p+1] = F.YY + T.ZZ * psPos[DIM*p  ] - T.XX * psPos[DIM*p+2];
        Y[DIM*p+2] = F.ZZ + T.XX * psPos[DIM*p+1] - T.YY * psPos[DIM*p  ];
#endif
    }
}


void Sphere::addSurfaceSpeedsFromForces(const real* X, real* Y, real sc) const
{
    //scale by point mobility:
    sc *= prop->point_mobility;
    
    // no surface-motion on the center and reference points
    assert_true( nbPoints() >= nbRefPts );
    
    // this should not apply to the reference-points:
    for ( unsigned int p = nbRefPts; p < nbPoints(); ++p )
    {
        real a = spProj[DIM*p] * X[DIM*p];
        
        for ( int d = 1; d < DIM; ++d )
            a += spProj[DIM*p+d] * X[DIM*p+d];
        
        for ( int d = 0; d < DIM; ++d )
            Y[DIM*p+d] += sc * ( X[DIM*p+d] - a * spProj[DIM*p+d] );
    }
}


void Sphere::setSpeedsFromForces(const real* X, real* Y, const real sc, bool) const
{
    // set motions from the rigid body
    setSphereSpeedsFromForces( X, Y, sc );
    //blas_xzero(DIM*nbPoints(), Y);
    
    // add contribution from the Surface motion
    addSurfaceSpeedsFromForces( X, Y, sc );
}
#endif



//------------------------------------------------------------------------------
#pragma mark -

void Sphere::write(OutputWrapper& out) const
{
    out.writeFloat(radius());
    PointSet::write(out);
}


void Sphere::read(InputWrapper & in, Simul& sim)
{
    try {
        real rad;
#ifdef BACKWARD_COMPATIBILITY
        if ( in.formatID() < 36 )
            rad = radius();
        else
#endif
        rad = in.readFloat();
        PointSet::read(in, sim);
        resize(rad);
    }
    catch( Exception & e ) {
        e << ", in Sphere::read()";
        clearPoints();
        throw;
    }
}
