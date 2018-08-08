// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "assert_macro.h"
#include "solid.h"
#include "solid_prop.h"
#include "clapack.h"
#include "exceptions.h"
#include "hand_prop.h"
#include "iowrapper.h"
#include "glossary.h"
#include "meca.h"
#include "simul.h"
#include "space.h"
extern Random RNG;

#if ( DIM == 3 )
#   include "quaternion.h"
#   include "matrix3.h"
#endif


Solid::Solid (SolidProp const* p)
: prop(p), soRadius(0), soShape(0), soShapeSize(0)
{
    soDrag = 0;
    soMom2D = 0;
    for ( int n = 0; n < DIM*DIM; ++n )
        soMom[n] = 0;
    soReshapeTimer = RNG.pint_exc(7);
}


Solid::~Solid()
{
    //Cytosim::MSG(32, "destroying %c%lu\n", TAG, number());
    deallocatePoints();
    prop = 0;
}


#pragma mark -

//------------------------------------------------------------------------------
/**
 This calls PointSet::allocatePoints().
 If PointSet::allocatePoints() allocated memory, it will return the 
 size of the new array, and in that case, the same size is allocated for other arrays.
 */
unsigned int Solid::allocatePoints(const unsigned int nbp)
{
    unsigned int ms = PointSet::allocatePoints(nbp);
    if ( ms )
    {
        //std::cerr << "Solid::allocatePoints " << ms << std::endl;
        
        // allocate a new array of the right size:
        real  *  soShape_new = new real[DIM*ms];
        real  *  soRadius_new = new real[ms];
        
        //set the radii to zero (no drag) by default:
        for ( unsigned int p = 0; p < ms; ++p )
            soRadius_new[p] = 0;
        
        // copy the current values in the new array:
        if ( soShape )
        {
            for ( unsigned int p = 0; p < nbPoints(); ++p )
            {
                soRadius_new[p] = soRadius[p];
                for ( int d = 0; d < DIM; ++d )
                    soShape_new[DIM*p+d] = soShape[DIM*p+d];
            }
            // delete the 'current' array:
            delete[] soShape;
            delete[] soRadius;
        }
        // the 'new' array becomes the 'current' one:
        soShape = soShape_new;
        soRadius = soRadius_new;
    }
    return ms;
}

//------------------------------------------------------------------------------
void Solid::deallocatePoints()
{
    PointSet::deallocatePoints();
    if ( soRadius )
    {
        delete[] soRadius;
        soRadius = 0;
    }
    if ( soShape )
    {
        delete[] soShape;
        soShape = 0;
    }
}


#pragma mark -

//------------------------------------------------------------------------------

void Solid::step()
{
}


void Solid::setInteractions(Meca & meca) const
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
            Space const* spc = prop->confine_space_ptr;
            
            for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
            {
                Vector pos = posPoint(pp);
                const real rad = soRadius[pp];
                
                // only confine massive points:
                if ( rad > 0 )
                {
                    if ( !spc->allInside(pos, rad) )
                        spc->setInteraction(pos, PointExact(this, pp), rad, meca, prop->confine_stiff);
                }
            }
        } break;
        
        case CONFINE_SURFACE:
        {
            Space const* spc = prop->confine_space_ptr;
            
            for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
            {
                // only confine massive points:
                if ( soRadius[pp] > 0 )
                    spc->setInteraction(posPoint(pp), PointExact(this, pp), meca, prop->confine_stiff);
            }
        } break;
        
        default:
            throw InvalidParameter("Invalid solid::confine");            
    }
}


#pragma mark -

//------------------------------------------------------------------------------
void Solid::radius(const unsigned int indx, real rad)
{
    assert_true( indx < nbPoints() );
    if ( rad < 0 )
        throw InvalidParameter("solid:radius must be positive");
    soRadius[indx] = rad;
}

real Solid::sumRadius()
{
    real sum = 0;
    for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
        sum += soRadius[pp];
    return sum;
}

Vector Solid::centroid()
{
    if ( nbPoints() == 0 )
        ABORT_NOW("empty Solid");
    
    Vector res(0,0,0);
    real sum = 0;
    for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
    {
        if ( soRadius[pp] > 0 )
        {
            res += soRadius[pp] * posPoint(pp);
            sum += soRadius[pp];
        }
    }
    if ( sum < REAL_EPSILON )
        ABORT_NOW("zero drag in solid");
    
    res /= sum;
    return res;
}


#pragma mark -

//------------------------------------------------------------------------------

/**
 @ingroup NewObject
 
 There are different ways to specify the number and positions of points in a Solid:
 
 @code
 new solid NAME
 {
 point0 = [INTEGER,] POSITION, RADIUS [, SINGLE_SPEC]
 point1 = [INTEGER,] POSITION, RADIUS [, SINGLE_SPEC]
 point2 = [INTEGER,] POSITION, RADIUS [, SINGLE_SPEC]
 etc.
 }
 @endcode
 
 each `point#` specifies a number of points to be added.
 The first parameter (`INTEGER`) specifies the number of points.
 The second argument (`POSITION`) specifies their position with respect to the center.
 The keywords are the same as for other position in cytosim (see examples below).
 The last argument (`RADIUS`) specifies the radius of the bead attached at this point,
 and it can be zero.
 
 Examples:
 
 @code
 new solid blob
 {
  point0 = center, 1.0
  point1 = 10, sphere 1, 0, grafted
 ...
 }
 @endcode
 
 `POSITION` can be a `VECTOR`, or the usual keywords:
 - `center`
 - `ball RADIUS`
 - `sphere RADIUS`
 - `equator RADIUS`
 .
 
 <h3> Add Singles to a Solid </h3>
 
 The parameter 'attach' can be used to add Single to the points of a Solid:
 
 @code
 new solid NAME
 {
  point0   = ... , SINGLE_SPEC
...
  attach   = SINGLE_SPEC [, SINGLE_SPEC] ...
  attach0  = SINGLE_SPEC [, SINGLE_SPEC] ...
  attach1  = SINGLE_SPEC [, SINGLE_SPEC] ...
...
 }
 @endcode
 
 Where `SINGLE_SPEC` is string containing at most 3 words: `[INTEGER] NAME [each]`,
 where the `INTEGER` specifies the number of Singles, `NAME` specifies their name,
 and the optional word `each` species that the command applies to every point.
 
 The command `attach` applies to all the points of the Solid, while `attach0`,
 `attach1`, etc. apply to the points specified by `point0`, `point1`, etc. only.
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

ObjectList Solid::build(Glossary & opt, Simul& simul)
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
        // optionally specify a number of points
        if ( opt.is_number(var) == 2 && opt.set(nbp, var) )
            ++inx;
        
        if ( nbp > 0 )
        {
            // get sphere radius:
            real sr = 0;
            opt.set(sr, var, inx+1);
            
            if ( sr < 0 )
                throw InvalidParameter("the radius of solid:sphere must be >= 0");
            
            unsigned fip = nbPoints();
            // add 'nbp' points:
            for ( unsigned n = 0; n < nbp; ++n )
            {
                // get position:
                Vector vec(0,0,0);
                std::istringstream iss(opt.value(var, inx));
                vec = Movable::readPosition(iss, 0);
                addSphere(vec, sr);
            }
            
            // attach Single to this set of points:
            inx += 2;
            while ( opt.set(str, var, inx++) )
                res.append(simul.singles.makeWrists(this, fip, nbp, str));
            
            // attach Single to this set of points:
            inx = 0;
            var = "attach" + sMath::repr(inp);
            while ( opt.set(str, var, inx++) )
                res.append(simul.singles.makeWrists(this, fip, nbp, str));
        }
        
        var = "point" + sMath::repr(++inp);
    }

    // attach Singles to be distributed over all the points:
    inx = 0;
    while ( opt.set(str, "attach", inx++) )
        res.append(simul.singles.makeWrists(this, 0, nbPoints(), str));
    
    // final verification of the number of points:
    nbp = 0;
    if ( opt.set(nbp, "nb_points")  &&  nbp != nbPoints() )
    {
        throw InvalidParameter("could not find the number of points specified in solid:nb_points");
    }
    
    return res;
}


//------------------------------------------------------------------------------
unsigned int Solid::addSphere(Vector const& vec, real rad)
{
    if ( rad < 0 )
        throw InvalidParameter("solid:sphere's radius should be >= 0");

    unsigned inx = addPoint(vec);
    soRadius[inx] = rad;
    //std::clog << "addSphere(" << vec << ", " << rad << ") for " << reference() << " index " << inx << "\n";
    return inx;
}


//------------------------------------------------------------------------------
/**
 fixShape() copies the current shape in the array soShape[],
 and calculates the moment of inertia of the ensemble of points.
 The reference soShape[] is used by 'reshape()', and 'rescale()'.
 */
void Solid::fixShape()
{
    if ( nbPoints() == 0 )
        throw InvalidParameter("Solid has no points!");
    
    real cc[DIM], pp[DIM];
    
    calculateMomentum(cc, pp, true);
    
    // calculate the center of gravity
    const real N = nbPoints();
    
    // store momentum of the current shape:
    soShapeSqr = 0;
    for ( unsigned int d = 0; d < DIM; ++d )
        soShapeSqr += pp[d] - N * cc[d] * cc[d];
    
    
    //we store the current points:
    soShapeSize = nbPoints();
    //set reference to current shape translated by -G (center) :
    for ( unsigned int nn = 0; nn < DIM*soShapeSize; nn += DIM )
    {
        for ( int dd = 0; dd < DIM; ++dd )
            soShape[nn+dd]  =  psPos[nn+dd] - cc[dd];
    }
}


#pragma mark -

/**
 The function rescale the reference shape soShape[], that was specified last time fixShape() was called.
 If axis==-1 (default), then all dimensions are scaled uniformly.
 The next call to reshape() will then apply the new reference to the current shape.
 */
void Solid::scaleShape(const real sx, const real sy, const real sz)
{
    //scale in only in the specified dimension
    for ( unsigned int pp = 0; pp < DIM * soShapeSize; pp += DIM)
    {
        soShape[pp  ] *= sx;
#if ( DIM > 1 )
        soShape[pp+1] *= sy;
#endif
#if ( DIM > 2 )
        soShape[pp+2] *= sz;
#endif
    }
    
    
    //recalculate the momentum needed in rescale():
    soShapeSqr = 0;
    for ( unsigned int pp = 0; pp < DIM * soShapeSize; ++pp )
        soShapeSqr += soShape[pp] * soShape[pp];
}


//------------------------------------------------------------------------------
/**
 Rescale the current cloud of points around its center of gravity,
 to recover the same 'size' as the reference soShape[]. 
 Size is measured as sum( ( x - g )^2 ).
 */
void Solid::rescale()
{
    real cc[DIM], pp[DIM];
    calculateMomentum(cc, pp, true);
    
    // calculate the center of gravity
    const real N = nbPoints();
    
    // calculate the momentum of the current shape:
    real sz = 0;
    for ( unsigned int d = 0; d < DIM; ++d )
        sz += pp[d] - N * cc[d] * cc[d];
    
    if ( sz <= 0 )
    {
        ABORT_NOW("Cannot rescale the Solid, because it has collapsed!");
        return;
    }
    
    // calculate the scaling factor to restore the size to 'soShapeSqr':
    real scale = sqrt( soShapeSqr / sz );
    
    // scale the shape around the center of gravity:
    for ( unsigned int p = 0; p < DIM * nbPoints(); p += DIM )
    {
        for ( unsigned int d = 0; d < DIM; ++d )
             psPos[p+d] = scale * ( psPos[p+d] - cc[d] ) + cc[d];
    }
}


//------------------------------------------------------------------------------

/**
 reshapeReally() finds the best isometric transformation = rotation + translation
 to bring the reference (soShape[]) onto the current shape (PointSet::psPos[]),
 and then replaces psPos[] by the transformed soShape[]. 
 This restores the shape of the cloud of point which is stored in soShape[],
 into the current position and orientation of the object.
 The best translation is the ones that conserves the center of gravity,
 The best rotation is obtained differently in 2D and 3D, and is unique.

 @todo: store the rotation and translation calculated by reshapeReally()
*/

#if ( DIM == 1 )

void Solid::reshape()
{    
    //we check that the number of points is the same as when fixShape() was called.
    if ( soShapeSize != nbPoints() )
        ABORT_NOW("mismatch with current number of points: forgot to call fixShape()?");
         
    real cc = 0, a = 0;
    for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
    {
        a  += psPos[pp] * soShape[pp];
        cc += psPos[pp];
    }
    
    cc /= real( nbPoints() );
    real s = a / fabs(a);
    
    for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
        psPos[pp] = s * soShape[pp] + cc;
}

#elif ( DIM == 2 )

void Solid::reshape()
{    
    // the number of points should be the same as when fixShape() was called.
    if ( soShapeSize != nbPoints() )
        ABORT_NOW("mismatch with current number of points: forgot to call fixShape()?");
    
    real cc[DIM];
    calculateMomentum(cc, true);
    
    /*
     The best rotation is obtained by simple math on the cross products
     and vector products of soShape[] and psPos[]: (see it on paper)
    */
    
    real a = 0, b = 0;
    
    for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
    {
        a += psPos[DIM*pp] * soShape[DIM*pp  ] + psPos[DIM*pp+1] * soShape[DIM*pp+1];
        b += soShape[DIM*pp] * psPos[DIM*pp+1] - soShape[DIM*pp+1] * psPos[DIM*pp  ];
    }
    
    real n = sqrt( a*a + b*b );
    
    // cosine and sinus of the rotation:
    real c = 1, s = 0;
    if ( n > REAL_EPSILON ) {
        c = a / n;
        s = b / n;
    }
    
    //printf(" n %8.3f, c %8.3f, s %8.3f norm = %8.3f\n", n, c, s, c*c + s*s);
    
    // apply transformation = rotation + translation:
    
    for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
    {
        psPos[DIM*pp  ] = c * soShape[DIM*pp] - s * soShape[DIM*pp+1] + cc[0];
        psPos[DIM*pp+1] = s * soShape[DIM*pp] + c * soShape[DIM*pp+1] + cc[1];
    }
}

#elif ( DIM == 3 )

void Solid::reshape()
{
    // the number of points should be the same as when fixShape() was called.
    if ( soShapeSize != nbPoints() )
        ABORT_NOW("mismatch with current number of points: forgot to call fixShape()?");
    
    /*
     We follow the procedure described by Berthold K.P. Horn in
     "Closed-form solution of absolute orientation using unit quaternions"
     Journal of the optical society of America A, Vol 4, Page 629, April 1987
    */
    
    real cc[DIM];
    calculateMomentum(cc, true);
    
    real S[3*3] = { 0 };
    for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
    {
        for ( unsigned int dd = 0; dd < DIM; ++dd )
            for ( int ee = 0; ee < DIM; ++ee )
                S[dd+DIM*ee] += soShape[DIM*pp+dd] * psPos[DIM*pp+ee];
    }
    
    real scale = 1.0 / sMath::max(fabs(S[0]), fabs(S[4]), fabs(S[8]));
    
    real N[4*4];
    
    N[0+4*0] = scale * ( S[0+DIM*0] + S[1+DIM*1] + S[2+DIM*2] );
    N[0+4*1] = scale * ( S[1+DIM*2] - S[2+DIM*1] );
    N[0+4*2] = scale * ( S[2+DIM*0] - S[0+DIM*2] );
    N[0+4*3] = scale * ( S[0+DIM*1] - S[1+DIM*0] );
    N[1+4*1] = scale * ( S[0+DIM*0] - S[1+DIM*1] - S[2+DIM*2] );
    N[1+4*2] = scale * ( S[0+DIM*1] + S[1+DIM*0] );
    N[1+4*3] = scale * ( S[2+DIM*0] + S[0+DIM*2] );
    N[2+4*2] = scale * ( S[1+DIM*1] - S[0+DIM*0] - S[2+DIM*2] );
    N[2+4*3] = scale * ( S[1+DIM*2] + S[2+DIM*1] );
    N[3+4*3] = scale * ( S[2+DIM*2] - S[1+DIM*1] - S[0+DIM*0] );
    
    /* 
     Use lapack to find the largest Eigenvalue, and associated Eigenvector,
     which is the quaternion corresponding to the best rotation
     */
    
    int nbvalues;
    real eValue[4];
    Quaternion<real> quat;
    real work[8*4];
    int iwork[5*4];
    int ifail[4];
    
    int info = 0;
    lapack_xsyevx('V','I','U', 4, N, 4, 0, 0, 4, 4, REAL_EPSILON,
                  &nbvalues, eValue, quat, 4, work, 8*4, iwork, ifail, &info );
    
    //Cytosim::MSG("optimal LWORK = %i\n", work[0] );
    //Cytosim::MSG("eigenvalue %6.2f,", eValue[0]);
    //quat.println();
    
    if ( info ) {
        printf("Solid::reshapeReally(): lapack_xsyevx() failed with code %i\n", info);
        return;
    }
    
    //get the rotation matrix corresponding to the quaternion:
    quat.setMatrix3( S );
    //Matrix3( S ).print();
    
    //apply the transformation = rotation + translation:
    for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
    {
        psPos[DIM*pp+0] = cc[0] + S[0]*soShape[DIM*pp]+ S[3]*soShape[DIM*pp+1] + S[6]*soShape[DIM*pp+2];
        psPos[DIM*pp+1] = cc[1] + S[1]*soShape[DIM*pp]+ S[4]*soShape[DIM*pp+1] + S[7]*soShape[DIM*pp+2];
        psPos[DIM*pp+2] = cc[2] + S[2]*soShape[DIM*pp]+ S[5]*soShape[DIM*pp+1] + S[8]*soShape[DIM*pp+2];
    }
}
#endif


//------------------------------------------------------------------------------
/**
 
 getPoints() calls rescale() often and reshapeReally() occasionally, because
 - reshapeReally() corrects for all kind of numerical drift but is CPU expensive
 - rescale() corrects for 2d order numerical drift, which are dominant.
 .
 
 The calls for different solids are shifted by using the number() of each Solid.
 */
void Solid::getPoints(const real * x)
{
    PointSet::getPoints(x);
    
    // for one point, nothing should be done
    if ( nbPoints() < 2 )
        return;
    
    if ( ++soReshapeTimer > 7 )
    {
        reshape();
        soReshapeTimer = 0;
    }
    else
        rescale();
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 Stokes relations:
 Translation:
 muT = 6*M_PI*viscosity*radius;
 muT * dx/dt = force
 Rotation:
 muR = 8*M_PI*viscosity*cub(radius)
 muR * d(angle)/dt = force-torque
 */
void Solid::setDragCoefficient()
{
    soDrag    = 0;            //the total drag coef.
    soDragRot = 0;            //the total rotational drag coef.  
    soCenter.set(0,0,0);      //the centroid of the points weighted by their drag coefficients
#if ( DIM == 2 )
    real roti = 0;            //in 2D, the total rotation inertia 
#endif
    
    for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
    {
        if ( soRadius[pp] > 0 )
        {
            soDrag    += soRadius[pp];
            soDragRot += soRadius[pp] * soRadius[pp] * soRadius[pp];
            soCenter  += soRadius[pp] * posPoint(pp);
#if ( DIM == 2 )
            roti      += soRadius[pp] * posPoint(pp).normSqr();
#endif
        }
    }
    
    if ( soDrag < REAL_EPSILON )
        throw InvalidParameter("zero drag in Solid");
    
    soCenter  /= soDrag;
    soDrag    *= 6*M_PI*prop->viscosity;
    soDragRot *= 8*M_PI*prop->viscosity;
    
    //std::clog << "Solid " << reference() << " has drag " << soDrag << "\n";
    
#if ( DIM == 2 )
    soMom2D = soDragRot + 6*M_PI*prop->viscosity*roti - soDrag * soCenter.normSqr();
    //sanity checks:
    if ( soMom2D < REAL_EPSILON )
        throw InvalidParameter("zero rotational drag in Solid");
#endif
}


//------------------------------------------------------------------------------

void Solid::prepareMecable()
{
    setDragCoefficient();
    makeProjection();
}


real Solid::addBrownianForces(real* rhs, real sc) const
{    
    // Brownian amplitude
    real b = sqrt( 2 * sc * soDrag / nbPoints() );

    for ( unsigned int jj = 0; jj < DIM*nbPoints(); ++jj )
        rhs[jj] += b * RNG.gauss();
    
    return b / soDrag;
}


#pragma mark -

//------------------------------------------------------------------------------
#if ( DIM == 1 )

/**
 The projection in 1D is just summing all the forces,
 and distributing equally to all the points:
*/
void Solid::makeProjection() {}

void Solid::setSpeedsFromForces(const real* X, real* Y, const real sc, bool) const
{
    real T = 0;
    for ( unsigned int p = 0; p < nbPoints(); ++p )
        T += X[p];
    T *= sc / soDrag;
    for ( unsigned int p = 0; p < nbPoints(); ++p )
        Y[p] = T;
}

#elif ( DIM == 2 )

//------------------------------------------------------------------------------
/**
 To project in 2D or 3D, we calculate the resulting tensor by summing all
 the forces on all points, reducing it at the center of gravity.
 From this, we can deduce the forces compatible with solid motion, 
 which is a combination of translation and rotation.
*/

void Solid::makeProjection()
{
    soCenter = centroid();
}


void Solid::setSpeedsFromForces(const real* X, real* Y, const real sc, bool) const
{
    real  TX = 0, TY = 0;  //Translation
    real  R  = 0;          //Infinitesimal Rotation (a vector in Z)
    
    for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
    {
        TX += X[pp*DIM  ];
        TY += X[pp*DIM+1];
        R  += psPos[pp*DIM] * X[pp*DIM+1] - psPos[pp*DIM+1] * X[pp*DIM];
    }
    
    R = (sc/soMom2D) * (R + vecProd(Vector(TX,TY,0),soCenter));
    Vector T = (sc/soDrag) * Vector(TX,TY,0) + vecProd(soCenter,R);
    
    for ( unsigned int p = 0; p < nbPoints(); ++p )
    {
        Y[p*DIM  ] = T.XX - R * psPos[p*DIM+1];
        Y[p*DIM+1] = T.YY + R * psPos[p*DIM  ];
    }
}


#elif ( DIM == 3 )

//------------------------------------------------------------------------------
void Solid::makeProjection()
{
    soCenter = centroid();
    
    ///\todo: from reshape, we know the rotation matrix from the stored shape
    //to the current shape. We could use it to transform the inertia matrix
    real px, py, pz;
    real m0=0, m3=0, m6=0, m4=0, m7=0, m8=0;
    
    for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
    {
        if ( soRadius[pp] > 0 )
        {
            const Vector pos = posPoint(pp);
            px = soRadius[pp] * pos.XX;
            py = soRadius[pp] * pos.YY;
            pz = soRadius[pp] * pos.ZZ;
            m0 += px * pos.XX;
            m3 += px * pos.YY;
            m6 += px * pos.ZZ;
            m4 += py * pos.YY;
            m7 += py * pos.ZZ;
            m8 += pz * pos.ZZ;
        }
    }
    
    assert_true( prop->viscosity > REAL_EPSILON );
    //scale to get the correct mobility:
    const real sc = 6*M_PI*prop->viscosity;
    m0 *= sc;
    m3 *= sc;
    m6 *= sc;
    m4 *= sc;
    m7 *= sc;
    m8 *= sc;
    
    //calculate the diagonal term of the matrix:
    const real diag = m0+m4+m8 + soDragRot - soDrag*soCenter.normSqr();
    
    assert_true( soDrag > REAL_EPSILON );
    //finally set the matrix in front of R in setSpeedsFromForces()
    soMom[0+DIM*0] = diag - m0 + soDrag * soCenter[0] * soCenter[0];
    soMom[0+DIM*1] =      - m3 + soDrag * soCenter[0] * soCenter[1];
    soMom[0+DIM*2] =      - m6 + soDrag * soCenter[0] * soCenter[2];
    soMom[1+DIM*1] = diag - m4 + soDrag * soCenter[1] * soCenter[1];
    soMom[1+DIM*2] =      - m7 + soDrag * soCenter[1] * soCenter[2];
    soMom[2+DIM*2] = diag - m8 + soDrag * soCenter[2] * soCenter[2];
    
    // The matrix should be symmetric positive definite
    // we try to compute the cholesky factorization with LAPack:
    int info = 0;
    lapack_xpotf2('U', DIM, soMom, DIM, &info);
    
    if ( info )
    {
        Matrix3(soMom).write(std::cerr);
        ABORT_NOW("failed to factorize Solid momentum matrix");
    }
}

//------------------------------------------------------------------------------
/**
 This calculated Y <- P * X, where
 P is the projection associated with the constraints of motion without
 deformation (solid object)
 
 We calculate the total force and momentum in zero, and distribute
 it according to solid motion mechanics.
*/
void Solid::setSpeedsFromForces(const real* X, real* Y, const real sc, bool) const
{    
    real TX=0, TY=0, TZ=0;    //Translation
    real RX=0, RY=0, RZ=0;    //Rotation
    
    for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
    {
        TX += X[pp*DIM  ];
        TY += X[pp*DIM+1];
        TZ += X[pp*DIM+2];
        RX += psPos[pp*DIM+1] * X[pp*DIM+2] - psPos[pp*DIM+2] * X[pp*DIM+1];
        RY += psPos[pp*DIM+2] * X[pp*DIM  ] - psPos[pp*DIM  ] * X[pp*DIM+2];
        RZ += psPos[pp*DIM  ] * X[pp*DIM+1] - psPos[pp*DIM+1] * X[pp*DIM  ];
    }
    
    Vector R = sc * ( Vector(RX,RY,RZ) + vecProd( Vector(TX,TY,TZ), soCenter ));
    
    //solve the 3x3 linear system, using the factorization stored in soMom:
    int info;
    lapack_xpotrs('U', DIM, 1, soMom, DIM, R, DIM, &info );
    assert_true( info == 0 );
    
    Vector T = (sc/soDrag) * Vector(TX,TY,TZ) + vecProd( soCenter, R );
    
    for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
    {
        Y[pp*DIM  ] = T.XX + R.YY * psPos[pp*DIM+2] - R.ZZ * psPos[pp*DIM+1];
        Y[pp*DIM+1] = T.YY + R.ZZ * psPos[pp*DIM  ] - R.XX * psPos[pp*DIM+2];
        Y[pp*DIM+2] = T.ZZ + R.XX * psPos[pp*DIM+1] - R.YY * psPos[pp*DIM  ];
    }
}

#endif


//------------------------------------------------------------------------------
#pragma mark -

void Solid::write(OutputWrapper& out) const
{
    out.writeUInt16(nbPoints());
    for ( unsigned int pp = 0; pp < nbPoints() ; ++pp )
    {
        out.writeFloatVector(psPos + DIM * pp, DIM, '\n');
        out.writeSoftSpace(2);
        out.writeFloat(soRadius[pp]);
    }
}


void Solid::read(InputWrapper & in, Simul&)
{
    try {
                     
        unsigned int nbp = in.readUInt16();
        nbPoints( nbp );
        for ( unsigned int pp = 0; pp < nbp ; ++pp )
        {
            in.readFloatVector(psPos+DIM*pp, DIM);
            soRadius[pp] = in.readFloat();
        }
        
    }
    catch( Exception & e ) {
        
        e << ", in Solid::read()";
        clearPoints();
        throw;
        
    }
    
    fixShape();
}
