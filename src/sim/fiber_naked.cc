// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "assert_macro.h"
#include "fiber_naked.h"
#include "iowrapper.h"
#include "messages.h"
#include "point_exact.h"
#include "point_interpolated.h"
#include "fiber_binder.h"
#include "exceptions.h"
#include "clapack.h"

extern Random RNG;

/**
 This return the number of point N+1,
 where N is the number of segments,
 that minimizes fabs( length / N - segmentation ),
 with ratio = length / segmentation
 */
unsigned FiberNaked::bestNbPoints(const real ratio)
{
    unsigned n = (int)ratio;
    
    if ( (2*n+1)*ratio > 2*n*(n+1) )
        return n+2;
    
    return n+1;
}


FiberNaked::FiberNaked()
{
    fnCut           = 0;
    fnCutWished     = 0;
    fnAbscissa      = 0;
    fnSignature     = RNG.pint();
}



//------------------------------------------------------------------------------
#pragma mark -

void FiberNaked::setStraight(Vector const& pos, Vector const& dir)
{
    assert_true( dir.norm() > 0.1 );
    // 'dir' is normalized for safety:
    Vector dpts = dir * ( fnCut / dir.norm() );
    //
    for ( unsigned int p = 0 ; p < nbPoints(); ++p )
        setPoint( p, pos + p * dpts );
}


void FiberNaked::setStraight(Vector const& pos, Vector const& dir, const FiberEnd ref)
{
    switch( ref )
    {
        case MINUS_END:
            setStraight( pos, dir );
            break;
            
        case PLUS_END:
            setStraight( pos + dir*length(), -dir );
            break;
            
        case CENTER:
            setStraight( pos - 0.5*dir*length(), dir );
            break;
            
        default:
            ABORT_NOW("invalid argument value");
    }
}


void FiberNaked::setStraight(Vector const& pos, Vector const& dir, real len, const FiberEnd ref)
{
    assert_true( fnCutWished > REAL_EPSILON );

    if ( len <= 0 )
        throw InvalidParameter("fiber:length must be > 0");

    int nbp = bestNbPoints(len/fnCutWished);
    assert_true( nbp > 1 );
    
    fnCut = len / real(nbp-1);
    nbPoints(nbp);
    
    setStraight(pos, dir, ref);
    updateRange();
}




/**
 This will set the Fiber with n points, unless ( n == 0 ) in which case
 the number of points will be set automatically from fnCutWished.
 pts[] should be of size DIM * n_pts.

 The given set of points do not need to be equally distributed.
 The length of the fiber will be roughly equal to the sum of all segment lengths.
 The MINUS_END and PLUS_END will be set to the first and last points provided,
 and intermediate points will be set by interpolating pts[].
 
 The length of the segments will only be approximately equal, 
 and reshape() should be called to equalize them if necessary
 */
void FiberNaked::setShape(const real pts[], unsigned n_pts, unsigned np)
{
    assert_true(n_pts > 1);
    Vector a, b;
    a.get(pts);
    
    //calculate the total length
    real len = 0;
    for ( unsigned n = 1; n < n_pts; ++n )
    {
        b.get(pts+DIM*n);
        len += (b-a).norm();
        a = b;
    }
    
    if ( np == 0 )
    {
        assert_true( fnCutWished > REAL_EPSILON );
        np = bestNbPoints(len/fnCutWished);
    }
    fnCut = len / real(np-1);
    nbPoints(np);
    
    a.get(pts);
    b.get(pts+DIM);
    setPoint(0, a);
    
    len = (b-a).norm();
    real h = 0;
    unsigned p = 1;
    --np;
    
    for ( unsigned n = 1; n < np; ++n )
    {
        h += fnCut;

        while ( h > len )
        {
            h -= len;
            a = b;
            ++p;
            assert_true(p<n_pts);
            b.get(pts+DIM*p);
            len = (b-a).norm();
        }
        
        setPoint(n, a+(h/len)*(b-a));
    }
    b.get(pts+DIM*n_pts-DIM);
    setPoint(np, b);
}


//===================================================================
#pragma mark -

/*
 This deals with Fiber having one segment only,
 where an iterative method is not necessary
 */
void FiberNaked::reshape_two(const real* src, real* dst, real cut)
{
#if ( DIM == 1 )
    Vector dif(src[1]-src[0], 0);
#elif ( DIM == 2 )
    Vector dif(src[2]-src[0], src[3]-src[1]);
#elif ( DIM == 3 )
    Vector dif(src[3]-src[0], src[4]-src[1], src[5]-src[2]);
#endif
    dif *= 0.5 * ( 1 - cut/dif.norm() );
    
    dst[0    ] = src[0    ] + dif.XX;
    dst[  DIM] = src[  DIM] - dif.XX;
#if ( DIM > 1 )
    dst[1    ] = src[1    ] + dif.YY;
    dst[1+DIM] = src[1+DIM] - dif.YY;
#endif
#if ( DIM > 2 )
    dst[2    ] = src[2    ] + dif.ZZ;
    dst[2+DIM] = src[2+DIM] - dif.ZZ;
#endif
}

/**
 Strasbourg, 22 Feb 2015
 
 Shorten segments to restore their length to 'cut'.
 We use a multidimensional Newton's method, to find iteratively the scalar
 coefficients that define the amount of displacement of each point.
 
 X[i] = vector of position
 We note 'dif' the differences between consecutive points:  dif[i] = X[i+1] - X[i]
 
 Given one scalar per segment: sca[i], the displaced point is:
 Y[i] = X[i] + sca[i] * dif[i] - sca[i-1] * dif[i-1]
 
 We want 'sca' to fulfill a set of constraints:
 F(sca) = length of segment - target = 0, with:  F[i] = ( Y[i+1] - Y[i] )^2 - cut^2
 
 
 Use all zero as first guess for 'sca', and apply Newton's method
 to calculate `sca_next` from `sca_now`:
 
 J * ( sca_next - sca_now ) = -F(sca_now)
 
 Where J is the Jacobian matrix: J[i][j] = dF[i] / dX[j]
 For this problem, J is square and tri-diagonal
 */

int FiberNaked::reshape_it(const unsigned ns, const real* src, real* dst, real cut)
{
    assert_true( ns > 1 );
    int info = 0;
    const real alphaSqr = cut * cut;
    
    Vector * dif = new Vector[ns];
    Vector * vec = new Vector[ns];
    real * sca = new real[ns];
    real * val = new real[ns];
    
    real * dia = new real[ns];
    real * low = new real[ns];
    real * upe = new real[ns];
    
    // calculate differences
    for ( unsigned pp = 0; pp < ns; ++pp )
    {
        dif[pp] = diffPoints(src, pp);
        sca[pp] = 0;
    }
    
    real err = 0;
    unsigned ii = 0;
    do {
#if ( 0 )
        printf("\n   %i sca  ", ii);
        for ( unsigned pp = 0; pp < ns; ++pp )
            printf("%+6.4f ", sca[pp]);
#endif
        
        // calculate all values of 'vec'
        vec[0] = (1-2*sca[0])*dif[0] + sca[1]*dif[1];
        for ( unsigned pp = 1; pp+1 < ns; ++pp )
            vec[pp] = sca[pp-1]*dif[pp-1] + (1-2*sca[pp])*dif[pp] + sca[pp+1]*dif[pp+1];
        vec[ns-1] = sca[ns-2]*dif[ns-2] + (1-2*sca[ns-1])*dif[ns-1];
        
        // calculate the matrix elements and RHS of system
        val[0] = vec[0].normSqr() - alphaSqr;
        dia[0] = -2 * ( vec[0] * dif[0] );
        for ( unsigned pp = 1; pp < ns; ++pp )
        {
            val[pp] = vec[pp].normSqr() - alphaSqr;
            low[pp] = vec[pp] * dif[pp-1];
            dia[pp] = -2 * ( vec[pp] * dif[pp] );
            upe[pp-1] = vec[pp-1] * dif[pp];
        }
        
#if ( 0 )
        printf("\n   %i val  ", ii);
        for ( unsigned pp = 0; pp < ns; ++pp )
            printf("%+6.4f ", val[pp]);
        printf("\n   %i upe  ", ii);
        for ( unsigned pp = 0; pp+1 < ns; ++pp )
            printf("%+6.4f ", upe[pp]);
        printf("\n   %i dia  ", ii);
        for ( unsigned pp = 0; pp < ns; ++pp )
            printf("%+6.4f ", dia[pp]);
        printf("\n   %i low  ", ii);
        for ( unsigned pp = 1; pp < ns; ++pp )
            printf("%+6.4f ", low[pp]);
#endif
        
        lapack_xgtsv(ns, 1, low+1, dia, upe, val, ns, &info);
        if ( info )
        {
            std::cerr << " LAPACK dgtsv failed " << info << std::endl;
            goto finish;
        }
        
        err = 0;
        for ( unsigned pp = 0; pp < ns; ++pp )
        {
            sca[pp] += -0.5 * val[pp];
            err += fabs(val[pp]);
        }
        if ( ++ii > 32 )
        {
            info = 1;
            goto finish;
        }
    } while ( err > 0.001 );
    
    
#if ( 0 )
    printf("\n%2i err %e", ii, err);
    printf("\n%2i sca  ", ii);
    for ( unsigned pp = 0; pp < ns; ++pp )
        printf("%+6.4f ", sca[pp]);
    printf("\n");
#endif
    
    //apply corrections:
    {
        Vector d, e = sca[0] * dif[0];
        dst[0] = src[0] + e.XX;
#if ( DIM > 1 )
        dst[1] = src[1] + e.YY;
#endif
#if ( DIM > 2 )
        dst[2] = src[2] + e.ZZ;
#endif
        for ( unsigned pp = 1; pp < ns; ++pp )
        {
            d = sca[pp] * dif[pp];
            dst[DIM*pp  ] = src[DIM*pp  ] + d.XX - e.XX;
#if ( DIM > 1 )
            dst[DIM*pp+1] = src[DIM*pp+1] + d.YY - e.YY;
#endif
#if ( DIM > 2 )
            dst[DIM*pp+2] = src[DIM*pp+2] + d.ZZ - e.ZZ;
#endif
            e = d;
        }
        dst[DIM*ns+0] = src[DIM*ns+0] - d.XX;
#if ( DIM > 1 )
        dst[DIM*ns+1] = src[DIM*ns+1] - d.YY;
#endif
#if ( DIM > 2 )
        dst[DIM*ns+2] = src[DIM*ns+2] - d.ZZ;
#endif
    }
    
finish:
    delete [] sca;
    delete [] dif;
    delete [] vec;
    delete [] val;
    delete [] dia;
    delete [] low;
    delete [] upe;
    
    return info;
}

/**
 Move the model-points relative to each other, such that when this is done,
 all segments have the same distance `fnCut` ( =segmentation() ).
 This is operation does not change the center of gravity of the fiber.
 
 
 NOTE: if two consecutive points overlap, there is no unique way to
 restore the constraints! We do nothing in that case, because most
 likely, the Brownian motion will push the points appart soon.
 */

void FiberNaked::reshape_sure(const unsigned ns, real* vec, real cut)
{
    Vector dp(0,0,0), sum(0,0,0);
    Vector seg = diffPoints(vec, 0);
    real   dis = seg.norm();
    
    // translation needed to restore first segment
    if ( dis > REAL_EPSILON )
        dp = ( cut/dis - 1.0 ) * seg;
    
    for ( unsigned pp = 1; pp < ns; ++pp )
    {
        seg = diffPoints(vec, pp);
        dis = seg.norm();
        
        //move the left point by dp:
        dp.add_to(vec+DIM*pp);
        //update the uniform motion of the points:
        sum += dp;
        
        //add to the translation needed to restore this segment
        if ( dis > REAL_EPSILON )
            dp += ( cut/dis - 1.0 ) * seg;
    }
    
    //move the last point by dy[]:
    dp.add_to(vec+DIM*ns);
    
    // calculte a uniform motion to conserve the center of gravity:
    sum = ( sum + dp ) * ( -1.0 / ( ns + 1 ) );
    
    //translate the entire fiber uniformly:
    for ( unsigned pp = 0; pp <= ns; ++pp )
        sum.add_to(vec+DIM*pp);
}



void FiberNaked::reshape()
{
    if ( nbPoints() == 2 )
        reshape_two(psPos, psPos, fnCut);
    else if ( reshape_it(nbSegments(), psPos, psPos, fnCut) )
        reshape_sure(nbSegments(), psPos, fnCut);
}


void FiberNaked::getPoints(const real * x)
{
    if ( nbPoints() == 2 )
        reshape_two(x, psPos, fnCut);
    else if ( reshape_it(nbSegments(), x, psPos, fnCut) )
    {
        //std::clog << "Note that `reshape_it` failed, but it was recovered" << std::endl;
        PointSet::getPoints(x);
        reshape_sure(nbSegments(), psPos, fnCut);
    }
    //dump(std::cerr);
}

//========================================================================
//=====================GROWING/SHRINKING==================================
//========================================================================
#pragma mark -

/**
 The argument 'dlen' can be positive or negative:
 - dlen > 0 : growth,
 - dlen < 0 : shrinkage
 .
 
 Note: This works nicely only if `dlen` is small compared to segmentation().
 For large decrease in length, use cutM().
 */
void FiberNaked::growM(const real dlen)
{
    assert_true( length() + dlen > 0 );
    real a = -dlen / length();
    
    if ( dlen > 0 )
    {
        unsigned p = 0, n = nbSegments();
        Vector dp0 = diffPoints(0), dp1;
        movePoint(p, ( a * n ) * dp0);
        ++p;
        --n;
        
        if ( n > 0  &&  ( n & 1 ) )
        {
            dp1 = diffPoints(p);
            movePoint(p, ( a * n ) * dp0);
            dp0 = dp1;
            ++p;
            --n;
        }
        
        while ( n > 1 )
        {
            //assert_true( 0 == (p & 1) );
            dp1 = diffPoints(p);
            movePoint(p, ( a * n ) * dp0);
            ++p; --n;
            //assert_true( 1 == (p & 1) );
            dp0 = diffPoints(p);
            movePoint(p, ( a * n ) * dp1);
            ++p; --n;
        }
    }
    else if ( dlen < 0 )
    {
        for ( unsigned p = 0, n = nbSegments(); n > 0; ++p, --n )
            movePoint(p, ( a * n ) * diffPoints(p));
    }
    
    fnCut += dlen / real( nbSegments() );
    fnAbscissa -= dlen;
    updateRange();
}

/**
 This extends the fiber by adding one segment at the MINUS_END.
 Thus  segmentation() is not changed, and the existing points are not displaced.
 */
void FiberNaked::addSegmentM()
{
    unsigned int pp = 1+nbPoints();
    nbPoints(pp);
    
    pp *= DIM;
    while ( --pp >= DIM )
        psPos[pp] = psPos[pp-DIM];
    
    for ( pp = 0; pp < DIM; ++pp )
        psPos[pp] += psPos[pp] - psPos[pp+2*DIM];
    
    fnAbscissa -= fnCut;
    updateRange();
}


/**
 The Fiber length is reduced by `dlen` ( which must be >= 0 ).
 The portion of size `dlen` near the MINUS_END is removed,
 the (fewer) model-points are recalculated.
 
 Note: after cutM(), the distance between the points is not
 equal to segmentation(). This is true only if the fiber is straight.
 */
void FiberNaked::cutM(const real dlen)
{
    real len = length();
    assert_true( 0 <= dlen );
    assert_true( dlen < len );
    
    const unsigned int nbp = bestNbPoints((len-dlen)/fnCutWished);
    const real cut = (len-dlen) / (nbp-1);
    real* tmp = new real[DIM*nbp];

    // calculate intermediate points into tmp[]:
    for ( unsigned int pp=0; pp+1 < nbp; ++pp )
    {
        Vector w = interpolateM(dlen+pp*cut).pos();
        w.put(tmp+DIM*pp);
    }
    
    // copy the position of plus-end into tmp[]:
    const unsigned int lp = lastPoint();
    for ( unsigned int d = 0 ; d < DIM; ++d )
        tmp[DIM*(nbp-1)+d] = psPos[DIM*lp+d];
    
    nbPoints(nbp);
    
    // copy calculated points to psPos[]
    for ( unsigned int pp = 0; pp < DIM*nbp; ++pp )
        psPos[pp] = tmp[pp];
    
    delete [] tmp;
    fnAbscissa += dlen;
    fnCut = cut;
    updateRange();
}


/**
 The argument 'dlen' can be positive or negative:
 - dlen > 0 : growth,
 - dlen < 0 : shrinkage
 .
 
 Note: This works nicely only if `dlen` is small compared to segmentation().
 For large decrease in length, use cutP().
 */
void FiberNaked::growP(const real dlen)
{
    assert_true( length() + dlen > 0 );
    real a = dlen / length();
    
    if ( dlen > 0 )
    {
        unsigned p = lastPoint();
        Vector dp0 = diffPoints(p-1), dp1;
        movePoint(p, ( a * p ) * dp0);
        --p;
        
        if ( p > 0  &&  ( p & 1 ) )
        {
            dp1 = diffPoints(p-1);
            movePoint(p, ( a * p ) * dp0);
            dp0 = dp1;
            --p;
        }
        
        while ( p > 1 )
        {
            //assert_true( 0 == (p & 1) );
            dp1 = diffPoints(p-1);
            movePoint(p, ( a * p ) * dp0);
            --p;
            //assert_true( 1 == (p & 1) );
            dp0 = diffPoints(p-1);
            movePoint(p, ( a * p ) * dp1);
            --p;
        }
    }
    else if ( dlen < 0 )
    {
        for ( unsigned p = lastPoint() ; p > 0 ; --p )
            movePoint(p, ( a * p ) * diffPoints(p-1));
    }
    
    fnCut += dlen / real( nbSegments() );
    updateRange();
}


/**
 This extends the fiber by adding one segment at the PLUS_END.
 Thus  segmentation() is not changed, and the existing points are not displaced.
 */
void FiberNaked::addSegmentP()
{
    unsigned int pp = nbPoints();
    nbPoints(pp+1);
    
    real * psp = psPos + pp * DIM;
    for ( unsigned int dd = 0; dd < DIM; ++dd )
        psp[dd] = 2 * psp[dd-DIM] - psp[dd-2*DIM];
    
    updateRange();
}


/**
 The Fiber length is reduced by `dlen` ( which must be >= 0 ).
 The portion of size `dlen` near the PLUS_END is removed,
 the (fewer) model-points are recalculated.

 Note: after cutP(), the distance between the points is not
 equal to segmentation(). This is true only if the fiber is straight.
*/
void FiberNaked::cutP(const real dlen)
{
    real len = length();
    assert_true( 0 <= dlen );
    assert_true( dlen < len );
    
    const unsigned int nbp = bestNbPoints((len-dlen)/fnCutWished);
    const real cut = (len-dlen) / (nbp-1);
    real* tmp = new real[DIM*nbp];
    
    // calculate intermediate points into tmp[]:
    for ( unsigned int pp = 1; pp < nbp; ++pp )
    {
        Vector w = interpolateM(pp*cut).pos();
        w.put(tmp+DIM*pp);
    }
    
    nbPoints(nbp);
    
    // copy calculated points to psPos[]
    // point at minus-end has not changed
    for ( unsigned int pp = DIM; pp < DIM*nbp; ++pp )
        psPos[pp] = tmp[pp];
    
    delete [] tmp;
    fnCut = cut;
    updateRange();
}

//------------------------------------------------------------------------------

void FiberNaked::setLength(real len, FiberEnd ref)
{
    assert_true( len > 0 );
    
    if ( ref == PLUS_END )
    {
        if ( len < length() )
            cutP(length()-len);
        else
            growP(len-length());
    }
    else if ( ref == MINUS_END )
    {
        if ( len < length() )
            cutM(length()-len);
        else
            growM(len-length());
    }
}


void FiberNaked::truncateM(const unsigned int p)
{
    PointSet::truncateM(p);
    fnAbscissa = abscissaP(p);
    updateRange();
}


void FiberNaked::truncateP(const unsigned int p)
{
    PointSet::truncateP(p);
    updateRange();
}


/**
 The model-point are reinterpolated linearly, and the length of the
 segments will not fullfil the constraints of segmentation.
 If this is a problem, FiberNaked::reshape() should be called.
 
 `fib` should generally be destroyed afterward.
 */
void FiberNaked::join(FiberNaked const* fib)
{
    const real len1 = fib->length();
    const real len2 = length();
    const unsigned int nbp = bestNbPoints((len1+len2)/fnCutWished);
    const unsigned int nbr = nbp - 1;
    const real cut = (len1+len2) / real(nbr);
    real* tmp = new real[DIM*nbp];
    
    // calculate new points into tmp[]:
    for ( unsigned int pp = 1; pp < nbr; ++pp )
    {
        Vector w;
        if ( pp*cut < len1 )
            w = fib->interpolateM(pp*cut).pos();
        else
            w = interpolateM(pp*cut-len1).pos();
        
        w.put(tmp+DIM*pp);
    }
    
    // copy the position of MINUS_END and PLUS_END:
    const unsigned int lp = lastPoint();
    for ( unsigned int d = 0 ; d < DIM; ++d )
    {
        psPos[d]       = fib->psPos[d];
        tmp[DIM*nbr+d] = psPos[DIM*lp+d];
    }
    
    nbPoints(nbp);
    
    // copy point back in place:
    for ( unsigned int pp = DIM; pp < DIM*nbp; ++pp )
        psPos[pp] = tmp[pp];
    
    delete [] tmp;
    fnCut = cut;
}

//------------------------------------------------------------------------------
#pragma mark -


/**
 Returns the minimum and maximum distance between consecutive points
 */
void FiberNaked::minMaxSegments(real& mn, real& mx) const
{
    real r = diffPoints(0).norm();
    mn = r;
    mx = r;
    for ( unsigned n = 1; n < lastPoint(); ++n )
    {
        real r = diffPoints(n).norm();
        if ( r > mx )
            mx = r;
        if ( r < mn )
            mn = r;
    }
}

/**
 Returns the average and variances of segment length
 */
void FiberNaked::infoSegments(real& avg, real& var) const
{
    avg = 0;
    var = 0;
    unsigned cnt = nbSegments();
    for ( unsigned n = 0; n < cnt; ++n )
    {
        real r = diffPoints(n).norm();
        avg += r;
        var += r*r;
    }
    var = var - avg * avg / cnt;
    avg /= cnt;
}


real FiberNaked::minCosinus() const
{
    if ( nbSegments() == 2 )
        return ( diffPoints(0)*diffPoints(1) ) / ( fnCut*fnCut );
    
    real result;
    Vector dir1, dir2;
    
    unsigned int ps = nbSegments() % 2;
    if ( ps )
    {
        dir1   = diffPoints(0);
        result = fnCut * fnCut;
    }
    else
    {
        assert_true( nbSegments() > 2 );
        dir1   = diffPoints(1);
        result = diffPoints(0) * dir1;
        ps = 2;
    }
    
    for ( ; ps < nbSegments(); ps+=2 )
    {
        dir2 = diffPoints(ps);
        real s = dir1 * dir2;
        if ( s < result ) result = s;
        dir1 = diffPoints(ps+1);
        real t = dir1 * dir2;
        if ( t < result ) result = t;
    }
    
    return result / ( fnCut * fnCut );
}


/**
 Returns the minimum and maximum distance between consecutive points
 */
unsigned FiberNaked::nbKinks(real threshold) const
{
    threshold *= fnCut * fnCut;
    unsigned res = 0;
    Vector d = diffPoints(0);
    
    for ( unsigned n = 1; n < lastPoint(); ++n )
    {
        Vector r = diffPoints(n);
        if ( d * r < threshold )
            ++res;
        d = r;
    }
    return res;
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 Note: Unless the Fiber is straight, the segments will not be exactly of length `fnCut`
 after the reinterpolation, and calling reshape() may be necessary.
 
 @todo 2d-order interpolation in FiberNaked::resegment()
 */
void FiberNaked::resegment(unsigned nps)
{
    assert_true( nps > 1 );
    
    //now nps is the number of segment
    --nps;
    
    real cut = length() / real(nps);
    
    // calculate new intermediate points in tmp[]:
    real* tmp = new real[DIM*nps];
    Vector a = posPoint(0), b = posPoint(1);
    
    real h = 0;
    unsigned p = 1;
    
    for ( unsigned n = 1; n < nps; ++n )
    {
        h += cut;
        
        while ( h > fnCut )
        {
            h -= fnCut;
            a = b;
            ++p;
            assert_true(p<nbPoints());
            b.get(psPos+DIM*p);
        }
        
        Vector w = a + ( h / fnCut ) * ( b - a );
        w.put(tmp+DIM*n);
    }
    
    // save index of PLUS_END
    p = DIM*lastPoint();
    
    // resize array:
    nbPoints(nps+1);

    // move coordinates of last point
    for ( unsigned d = 0; d < DIM; ++d )
        psPos[DIM*nps+d] = psPos[p+d];

    // copy calculated coordinates back into psPos
    for ( unsigned d = DIM; d < DIM*nps; ++d )
        psPos[d] = tmp[d];

    delete [] tmp;
    fnCut = cut;
    reshape();
}



/**
 A fiber is segmented as a function of its length.

 The number of segments will be such that:
 @code
 FiberNaked::segmentation() < 4/3 * FiberProp::segmentation
 FiberNaked::segmentation() > 2/3 * FiberProp::segmentation
 @endcode
 */
void FiberNaked::adjustSegmentation()
{
    assert_true( fnCutWished > REAL_EPSILON );
    
    unsigned best = bestNbPoints(length()/fnCutWished);
    
    if ( best != nbPoints() )
        resegment(best);
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 return the abscissa with respect to the ORIGIN.
 */
real FiberNaked::abscissa(const FiberEnd end) const
{
    switch( end )
    {
        case ORIGIN:    return 0;
        case MINUS_END: return fnAbscissa;
        case PLUS_END:  return fnAbscissa + fnCut * nbSegments();
        case CENTER:    return fnAbscissa + 0.5 * fnCut * nbSegments();
        default:        ABORT_NOW("invalid argument value"); return 0;
    }
}


/**
 convert the abscissa that is specified from the given end,
 to the abscissa from the ORIGIN.
 
 ATTENTION: the direction is inverted when `from = PLUS_END`
 */
real FiberNaked::abscissa(const real ab, const FiberEnd from) const
{
    switch( from )
    {
        case ORIGIN:     return ab;
        case MINUS_END:  return ab + fnAbscissa;
        case CENTER:     return ab + fnAbscissa + 0.5 * fnCut * nbSegments();
        case PLUS_END:   return fnAbscissa + fnCut * nbSegments() - ab;
        default:         ABORT_NOW("invalid argument value"); return 0;
    }
}


/**
 The Fiber is partitionned by this function in three regions:
 - a MINUS_END part, which is of length 'lambda'
 - a PLUS_END part, also of length 'lambda'
 - and a NOT_END section in between
 .
 A Fiber shorter than 2*len does not have a central region,
 and is composed of PLUS_END and MINUS_END parts of equal size.
 */    
FiberEnd FiberNaked::whichEndDomain(const real ab, const real lambda) const
{
    const real abs = ab - fnAbscissa;
    const real len = length();
    
    if ( 2 * abs > len )
    {
        if ( abs >= len - lambda )
            return PLUS_END;
    }
    else
    {
        if ( abs <= lambda )
            return MINUS_END;
    }
    return NOT_END;
}


//------------------------------------------------------------------------------
#pragma mark -


PointExact FiberNaked::exactEnd(const FiberEnd which) const
{
    if ( which == MINUS_END )
        return PointExact(this, 0);
    else
    {
        assert_true( which == PLUS_END );
        return PointExact(this, lastPoint());
    }
}


PointInterpolated FiberNaked::interpolateEnd(const FiberEnd which) const
{
    if ( which == MINUS_END )
        return PointInterpolated(this, 0, 1, 0);
    else
    {
        assert_true( which == PLUS_END );
        return PointInterpolated(this, nbPoints()-2, nbPoints()-1, 1);
    }
}


PointInterpolated FiberNaked::interpolateCenter() const
{
    unsigned int n = lastPoint() / 2;
    if ( 2*n == lastPoint() )
        return PointInterpolated(this, n, n+1, 0);
    else
        return PointInterpolated(this, n, n+1, 0.5);
}


/**
 Same as interpolate(), but the abscissa `ab' is taken from the MINUS_END of the Fiber.
 */
PointInterpolated FiberNaked::interpolateM(const real ab) const
{
    if ( ab <= 0 )
        return PointInterpolated(this, 0, 1, 0.0);
    
    double n, co = modf( ab / fnCut, &n );
    unsigned int rd = (unsigned int)n;
    
    //beyond the last point, we interpolate the PLUS_END
    if ( rd < lastPoint() )
        return PointInterpolated(this, rd, rd+1, co);
    else
        return PointInterpolated(this, nbPoints()-2, nbPoints()-1, 1.0);
}


/**
 Convert abscissa `ab' into a PointInterpolated = ( a model-point `r' + a coefficient `a' ).
 The corresponding point X = P(r) * (1-a) + P(r+1) * a:
 - `r' is an integer: 0 <= r < lastPoint(),
 - `a' is a positive real coefficient: 0 <= a <= 1
 .
 In this function, the abscissa `ab` is taken from the ORIGIN of the Fiber.
 */
PointInterpolated FiberNaked::interpolate(const real abo) const
{
    real ab = abo - fnAbscissa;
    
    if ( ab <= 0 )
        return PointInterpolated(this, 0, 1, 0.0);
    
    double n, co = modf( ab / fnCut, &n );
    unsigned int rd = (unsigned int)n;
    
    //beyond the last point, we interpolate the PLUS_END
    if ( rd < lastPoint() )
        return PointInterpolated(this, rd, rd+1, co);
    else
        return PointInterpolated(this, nbPoints()-2, nbPoints()-1, 1.0);
}


PointInterpolated FiberNaked::interpolate(const real ab, const FiberEnd from) const
{
    switch( from )
    {
        case ORIGIN:
            return interpolate(ab);
            
        case MINUS_END:
            return interpolateM(ab);
            
        case CENTER:
            return interpolateM(ab + 0.5*fnCut*nbSegments());
            
        case PLUS_END:  //this is counted from the plus towards the minus end
            return interpolateM(fnCut*nbSegments() - ab);
        
        default:
            ABORT_NOW("invalid argument value");
    }
    return interpolate(0);
}

//------------------------------------------------------------------------------
#pragma mark -

Vector FiberNaked::posEnd(FiberEnd which) const
{
    if ( which == MINUS_END )
        return posPoint(0);
    else if ( which == PLUS_END )
        return posPoint(lastPoint());
    else
        return interpolate(0, which).pos();
}


Vector FiberNaked::pos(const real abo) const
{
#if ( 0 )
    return interpolate(ab).pos();
#else
    real ab = abo - fnAbscissa;
    
    if ( ab <= 0 )
        return posPoint(0);
    
    double n, co = modf( ab / fnCut, &n );
    unsigned int rd = (unsigned int)n;
    
    //beyond the last point, we interpolate the PLUS_END
    if ( rd < lastPoint() )
        return interpolatePoints(rd, rd+1, co);
    else
        return posPoint(lastPoint());
#endif
}


Vector FiberNaked::posM(const real ab) const
{
#if ( 0 )
    return interpolateM(ab).pos();
#else
    if ( ab <= 0 )
        return posPoint(0);
    
    double n, co = modf( ab / fnCut, &n );
    unsigned int rd = (unsigned int)n;
    
    //beyond the last point, we interpolate the PLUS_END
    if ( rd < lastPoint() )
        return interpolatePoints(rd, rd+1, co);
    else
        return posPoint(lastPoint());
#endif    
}

Vector FiberNaked::pos(const real ab, const FiberEnd from) const
{
    return interpolate(ab, from).pos();
}


//------------------------------------------------------------------------------
#pragma mark -

Vector FiberNaked::dir(const real ab) const
{
    return dirPoint(interpolate(ab).point1());
}

Vector FiberNaked::dir(const real ab, const FiberEnd from) const
{
    return dirPoint(interpolate(ab, from).point1());
}


Vector FiberNaked::dirEnd(const FiberEnd which) const
{
    if ( which == MINUS_END )
        return dirPoint(0);
    else if ( which == PLUS_END )
        return dirPoint(lastSegment());
    else
        return dirPoint(interpolate(0, which).point1());
}


/**
 The returned value is negative when the force antagonizes elongation,
 and this is true at both ends. 
 */
real FiberNaked::projectedForceOnEnd(const FiberEnd which) const
{
    if ( which == PLUS_END )
        return    netForce(lastPoint()) * dirPoint(lastSegment());
    else
    {
        assert_true( which == MINUS_END );
        return  - netForce(0) * dirPoint(0);
    }
}


//------------------------------------------------------------------------------
#pragma mark -

void FiberNaked::write(OutputWrapper& out) const
{
    out.writeUInt32(fnSignature);
    out.writeFloat(length());
    out.writeFloat(fnCutWished);
    out.writeFloat(fnAbscissa);
    PointSet::write(out);
}

/**
 The fiber will be re-segmented if its current desired segmentation 
 does not match the one stored in the file.
 */
void FiberNaked::read(InputWrapper & in, Simul& sim)
{    
    unsigned int s = in.readUInt32();
    if ( s ) fnSignature = s;
    
    real len   = in.readFloat();
    real seg   = in.readFloat();
    fnAbscissa = in.readFloat();
    
    PointSet::read(in, sim);
    
    if ( nbPoints() < 2 )
        throw InvalidIO("Invalid Fiber with 0 or 1 point");

    if ( in.formatID() > 37 )
        fnCut = len / nbSegments();
    else
        fnCut = len;
    
    updateRange();
    if ( fnCutWished != seg )
        adjustSegmentation();
    
#if ( 1 )
    // verify the validity of the data:
    real mn, mx;
    minMaxSegments(mn, mx);
    
    if ( mx - mn > 0.01 * mn )
        std::cerr << "Warning: non-uniform Fiber segments: min=" << mn << ", max=" << mx << std::endl;
#endif
}

