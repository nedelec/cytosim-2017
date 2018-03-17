// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "space.h"
#include "space_prop.h"
#include "exceptions.h"
#include "point_interpolated.h"
#include "messages.h"
#include "iowrapper.h"
#include "meca.h"
extern Random RNG;


Space::Space(const SpaceProp* p) 
: nLength(0), prop(p)
{
    assert_true(prop);
    
    for ( unsigned int d = 0; d < DMAX; ++d )
        length(d, 0);
}


Space::~Space()
{
    prop = 0;
}


void Space::readLengths(const std::string& str)
{
    std::istringstream iss(str);
    unsigned int d = 0;
    while ( d < DMAX )
    {
        real s = 0;
        iss >> s;
        if ( iss.fail() )
            break;
        length(d++, s);
    }
    if ( d > nLength )
        nLength = d;
    resize();
}


/**
 Checks that the number of specified dimensions is >= \a required,
 and if \a positive == true, also check that they are positive values.
 */
void Space::checkLengths(unsigned int required, bool positive) const
{
    if ( nLength < required )
        throw InvalidParameter("shape '",prop->shape,"' requires more dimensions to be specified");

    if ( positive )
    {
        for ( unsigned int d = 0; d < required; ++d )
        {
            if ( length(d) < 0 )
                throw InvalidParameter("space:dimension[",d,"] must be >= 0");
        }        
    }
}


void Space::length(unsigned int d, const real v)
{
    if ( d < DMAX )
    {
        mLength[d]    = v;
        mLength2[d]   = 2*v;
        mLengthSqr[d] = v*v;
    }
}


void Space::resize(unsigned int d, const real v)
{
    if ( d < DMAX )
    {
        length(d, v);
        //std::cerr << " dim[" << d << "] = " << mLength[d] << std::endl;
        if ( nLength < d )
            nLength = d;
        resize();
    }
}


//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Random Places

/**
 Provide a uniform random distribution in the volume by Monte-Carlo.
 
 Algorithm: throw a point in the rectangular volume provided by extension()
 until inside() returns true.
*/
Vector Space::randomPlace() const
{
    Vector result, range = extension();
    
    unsigned long ouf = 0;
    do {
        
        result = range.e_mul(Vector::randBox());

        if ( ++ouf > 10000 )
        {
            return Vector(0,0,0);
            Cytosim::warning("placement failed after 10000 trials in Space::randomPlace()\n");
            //throw InvalidParameter("placement failed after 10000 trials in Space::randomPlace()");            
        }
        
    } while ( ! inside(result) );
    
    return result;
}


/**
 Return a \a point for which:
 - inside(point) = true
 - inside(point, radius) = false
 */
Vector Space::randomPlaceNearEdge( real rad ) const
{
    unsigned long ouf = 0;
    if ( rad <= 0 )
        throw InvalidParameter("edge:radius must be > 0");
    Vector res;
    do {
        res = randomPlace();
        assert_true( inside(res) );
        if ( ++ouf > 10000 )
            throw InvalidParameter("placement failed after 10000 trials in Space::randomPlaceNearEdge()");
    } while ( allInside( res, rad ) );
    return res;
}


/**
 Returns the projection of a point obtained by randomPlaceNearEdge() 
 */
Vector Space::randomPlaceOnEdge( real rad ) const
{
    Vector res, w = randomPlaceNearEdge(rad);
    project(w, res);
    //assert_true( inside(res) );
    return res;
}

//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Inside/Outside

/**
 A bead is entirely inside if:
 - its center is inside,
 - the minimal distance (center-to-edge) is greater than the radius
 .
 */
bool Space::allInside(const real center[], const real rad) const
{
    assert_true( rad > 0 );

    if ( ! inside(center) )
        return false;

    return ( distanceToEdgeSqr(center) >= rad * rad );
}

/**
 A bead is entirely outside if:
 - its center is outside,
 - the minimal distance (center-to-edge) is greater than the radius
 .
 
 Attention: this is not equivalent to !allInside(center, radius)
 */
bool Space::allOutside(const real center[], const real rad) const
{
    assert_true( rad > 0 );

    if ( inside(center) )
        return false;
        
    return ( distanceToEdgeSqr(center) >= rad * rad );
}

//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Project

/**
this code is equivalent to SpaceInflate::project(), with a negative radius
 */
void Space::project(const real point[], real proj[], const real rad) const
{
    if ( rad < 0 )
        ABORT_NOW("radius should not be negative");

    project(point, proj);

    
    ///\todo problem in project() with radius if point is exactly on the box (n==0)
    //if (n==0) we do not know the orthogonal direction to follow. We should
    //take another point near by, and project from there.
    

    real n = 0, x, pw[DIM];
    for ( int d = 0; d < DIM; ++d )
    {
        x = point[d] - proj[d];
        pw[d] = x;
        n += x * x;
    }
            
    if ( n > 0 )
        n = ( inside(point) ? +rad : -rad ) / sqrt(n);
    else {
        throw Exception("in project(..., radius): the point is on the edge");
        //printf("point % .3f % .3f % .3f :", point[0], point[1], point[2]);
        //printf("inside = %i :", inside(point));
        //printf("proj  % .3f % .3f % .3f\n", proj[0], proj[1], proj[2]);
    }
    
    for ( int d=0; d<DIM; ++d )
        proj[d] += n * pw[d];
}


void Space::project( real point[] ) const
{
    real c[DIM];
    for ( int d = 0; d < DIM; ++d )
        c[d] = point[d];
    project( c, point );
}

//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Misc

/**
 The volume is estimated ONCE with a simple monte-carlo approach:
 - throw points in the rectangular volume provided by extension()
 - count how many are inside the volume with inside()
 .
 Then
 @code
 volume ~ ( number-of-points-inside / number-of-point ) * volume-of-rectangle
 @endcode
 */
real Space::estimateVolume(unsigned long cnt) const
{
    Vector rec = extension();

    real vol = 1.0;
    for ( int d = 0; d < DIM; ++d )
        vol *= 2 * rec[d];
    
    unsigned long in = 0;
    for ( unsigned long i = 0; i < cnt; ++i )
    {
        real w[3];
        w[0] = rec.XX * RNG.sreal();
#if ( DIM > 1 )
        w[1] = rec.YY * RNG.sreal();
#endif
#if ( DIM > 2 )
        w[2] = rec.ZZ * RNG.sreal();
#endif
        in += inside(w);
    }
    
    vol *= in / real(cnt);
    
    Cytosim::MSG(0, "Monte-Carlo estimated volume = %.6f +/- %.6f\n", vol, sqrt(in)/real(cnt) );

    return vol;
}


/**
 This uses Space::project to reflect \a w on the edge of the Space,
 until the result eventually falls inside.
 
 In most geometries, this works well, but if the distance that the point
 is outside is very large compared to the width of the space, the number
 of iterations can be large.
*/
void Space::bounce(real w[]) const
{
    real p[DIM];

    // bounce on the edge, and return if inside
    int cnt = 0;
    do {
        project(w, p);
        for ( int d = 0; d < DIM; ++d )
            w[d] = p[d] + p[d] - w[d];
        if ( inside(w) )
            return;
    } while ( ++cnt < 50 );
    
    // Project if the iterations have failed to bring back in:
    project(w, p);
    for ( int d = 0; d < DIM; ++d )
        w[d] = p[d];
}


/** 
 normalToEdge(const real point[]) uses an iterative method to find
 the normal to the edge, using the project() method.
 
 If you know for certain that \a point[] is far from the edge,
 the normal can be more directly obtained from the projection:
 @code 
 project(point, proj);
 normal = ( proj - point ).normalized()
 @endcode
 
 */
Vector Space::normalToEdge(const real point[]) const
{
    const real goal = 10000*REAL_EPSILON*REAL_EPSILON;
    
    Vector P, M, proj, res;
    project(point, proj);

    real H = 1;
    for ( unsigned i = 0; i < 12; ++i )
    {
        H /= 2;
        for ( unsigned j = 0; j < 16; ++j )
        {
            //start from a random vector:
            res = Vector::randUnit(H);
            
            for ( unsigned n = 0; n < 32; ++n )
            {
                project(proj+res, P);
                project(proj-res, M);
                
                // refine the estimate:
                Vector ref = 0.5 * ( M - P );
                res += ref;
                
                // check convergence:
                if ( ref.normSqr() < goal )
                {
                    if ( 2 * res.norm() < H )
                        res.normalize(H);
                    else
                    {
                        if ( inside(proj+res) )
                            return res.normalized(-1);
                        else
                            return res.normalized();
                    }
                }
            }
        }
    }
    
    printf("warning: convergence failure in normalToEdge()\n");
    printf("         error = %e at height = %e\n", P.distance(proj), H);
    if ( inside(proj+res) )
        return res.normalized(-1);
    else
        return res.normalized();
}


//------------------------------------------------------------------------------
real  Space::distanceToEdgeSqr(const real point[]) const
{
    real proj[DIM];
    
    project(point, proj);
    
    real res = (point[0]-proj[0]) * (point[0]-proj[0]);
    for ( unsigned int dd=1; dd < DIM; ++dd )
        res += (point[dd]-proj[dd]) * (point[dd]-proj[dd]);
    
    return res;
}


real  Space::signedDistanceToEdge(const real point[]) const
{
    if ( inside(point) )
        return -distanceToEdge(point);
    else
        return +distanceToEdge(point);
}



//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Interactions

/**
 Call the appropriate interaction from \a meca, to confine \a pe.
 
 This implementation uses \a pos to find the local normal to the edge of the Space.
 and then calls Meca::interPlane, with the approprimate aguments.
 This generates a friction-less potential centered on the edge.
 */

void Space::setInteraction(Vector const& pos, PointExact const& pe, Meca & meca, real stiff) const
{
    Vector proj;
    project( pos, proj );
    Vector dir = pos - proj;
    real n = dir.normSqr();
    meca.interPlane( pe, dir, proj, stiff/n );
}

/**
 Call the appropriate interaction from \a meca, to confine \a pe, which is at position \a pos.
 
 The default implementation projects \a pos,
 to calculate the direction of the normal to the edge of the Space,
 and then calls Meca::interPlane, with the approprimate aguments.
 This generates a friction-less potential centered on the edge.
 */

void Space::setInteraction(Vector const& pos, PointExact const& pe, real rad, Meca & meca, real stiff) const
{
    Vector proj;
    project( pos, proj, rad );
    Vector dir = pos - proj;
    real n = dir.normSqr();
    meca.interPlane( pe, dir, proj, stiff/n );
}


//------------------------------------------------------------------------------
#pragma mark -
#pragma mark IO


void Space::write(OutputWrapper& out) const
{
    out.put_line(prop->shape, ' ');
    out.writeUInt8(nLength);
    for ( unsigned int d = 0; d < nLength; ++d )
        out.writeFloat(length(d));
}


void Space::read(InputWrapper& in, Simul&)
{
#ifdef BACKWARD_COMPATIBILITY
    if ( in.formatID() < 35 )
        return;
    
    if ( in.formatID() < 36 )
    {
        for ( unsigned int d = 0; d < nLength; ++d )
            length(d, in.readFloat());
        resize();
        return;
    }
#endif
    
    if ( in.formatID() > 40 )
    {
        // read the 'shape' stored as a string
        std::string str;
        in.get_line(str, ' ');
        
        // check that it matches current Space:
        if ( str.compare(0, prop->shape.size(), prop->shape) )
        {
            std::cerr << PREF << "file: " << str << std::endl;
            std::cerr << PREF << "mem : " << prop->shape << std::endl;
            throw InvalidIO("Missmatch in Space:shape");
        }
    }
    
    nLength = in.readUInt8();
    for ( unsigned int d = 0; d < nLength; ++d )
        length(d, in.readFloat());
    
    resize();
}

//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Display


#ifdef DISPLAY
#include "gle.h"


void Space::displaySection(const int dim, const real pos, const real step) const
{
    Vector q, p( pos, pos, pos );
    int xx = ( dim + 1 ) % DIM;
    int yy = ( xx + 1 ) % DIM;
    real xs = extension()[xx];
    real ys = extension()[yy];
    real inc = step * ( xs > ys ? xs : ys );

    glBegin(GL_LINE_LOOP);
    p[yy] = ys;
    for ( real a = -xs; a < xs; a += inc )
    {
        p[xx] = a;
        project(p, q);
        gle::gleVertex(q);
    };
    p[xx] = xs;
    for ( real a = -ys; a < ys; a += inc )
    {
        p[yy] = -a;
        project(p, q);
        gle::gleVertex(q);
    };
    p[yy] = -ys;
    for ( real a = -xs; a < xs; a += inc )
    {
        p[xx] = -a;
        project(p, q);
        gle::gleVertex(q);
    };
    p[xx] = -xs;
    for ( real a = -ys; a < ys; a += inc )
    {
        p[yy] = a;
        project(p, q);
        gle::gleVertex(q);
    };
    glEnd();
}

#else

void Space::displaySection(const int dim, const real pos, const real step) const
{
    //you will get this output if objects for play was not compiled properly:
    //DISPLAY should be defined on the compiler command, with: -DDISPLAY
    printf("dummy Space::displaySection()");
}

#endif
