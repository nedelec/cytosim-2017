// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "sim.h"
#include "space.h"
#include "simul.h"
#include "modulo.h"
#include "point_exact.h"
#include "point_interpolated.h"
//#include "vecprint.h"


extern Modulo * modulo;


//==============================================================================
#pragma mark -
#pragma mark Display

/**
 Display interactions from the Meca
 */

#ifdef DISPLAY_INTERACTIONS


  #include "opengl.h"
  #include "gle.h"
  bool showInteractions = true;

void displayInteraction(gle_color c,
                        const Vector & p, const Vector & q)
{
    glLineWidth(6);
    glBegin(GL_LINES);
    c.colorT(2); gle::gleVertex(p);
    c.colorT(1); gle::gleVertex(q);
    glEnd();
}

void displayInteraction(gle_color c,
                        const Vector & p, Vector q,
                        real len)
{
    if ( modulo ) modulo->fold(q, p);
    Vector dx = ( q - p ).normalized(0.5*len);
    glLineWidth(3);
    glBegin(GL_LINES);
    c.colorT(1); gle::gleVertex(p);
    c.colorT(1); gle::gleVertex(p+dx);
    c.colorT(1); gle::gleVertex(q-dx);
    c.colorT(1); gle::gleVertex(q);
    glEnd();
    glLineWidth(6);
    glBegin(GL_LINES);
    c.colorT(2); gle::gleVertex(p+dx);
    c.colorT(1); gle::gleVertex(q-dx);
    glEnd();
    glPointSize(10);
    glBegin(GL_POINTS);
    c.colorT(1); gle::gleVertex(p+dx);
    c.colorT(1); gle::gleVertex(q-dx);
    glEnd();
}

void displayInteraction(gle_color c,
                        const Vector & p, Vector q,
                        Vector s)
{
    if ( modulo )
    {
        modulo->fold(q, p);
        modulo->fold(s, p);
    }
    glLineWidth(3);
    glBegin(GL_LINES);
    c.colorT(1); gle::gleVertex(p);
    c.colorT(1); gle::gleVertex(q);
    glEnd();
    glLineWidth(6);
    glBegin(GL_LINES);
    c.colorT(2); gle::gleVertex(q);
    c.colorT(1); gle::gleVertex(s);
    glEnd();
    glPointSize(10);
    glBegin(GL_POINTS);
    c.colorT(1); gle::gleVertex(q);
    glEnd();
}

void displayInteraction(gle_color c,
                        const Vector & p, const Vector & q,
                        const Vector & s, const Vector & t)
{
    glLineWidth(3);
    glBegin(GL_LINES);
    c.colorT(1); gle::gleVertex(p);
    c.colorT(1); gle::gleVertex(s);
    c.colorT(1); gle::gleVertex(t);
    c.colorT(1); gle::gleVertex(q);
    glEnd();
    glLineWidth(6);
    glBegin(GL_LINES);
    c.colorT(2); gle::gleVertex(s);
    c.colorT(1); gle::gleVertex(t);
    glEnd();
    glPointSize(10);
    glBegin(GL_POINTS);
    c.colorT(1); gle::gleVertex(s);
    c.colorT(1); gle::gleVertex(t);
    glEnd();
}

#endif


//==============================================================================
#pragma mark -
#pragma mark Force
/**
 Add constant force to \a pte
 */
void Meca::addPureForce(const PointExact & pte, const Vector & force)
{
    const index_type inx = DIM * pte.matIndex();
    
    vBAS[inx  ] += force.XX;
#if ( DIM > 1 )
    vBAS[inx+1] += force.YY;
#endif
#if ( DIM > 2 )
    vBAS[inx+2] += force.ZZ;
#endif
}


/**
Add constant force to \a pti
 */
void Meca::addPureForce(const PointInterpolated & pti, const Vector & force)
{
    const index_type inx1 = DIM * pti.matIndex1();
    const index_type inx2 = DIM * pti.matIndex2();
    const real a = pti.coef1(), b = pti.coef2();
    
    vBAS[inx1  ] += b * force.XX;
    vBAS[inx2  ] += a * force.XX;
#if ( DIM > 1 )
    vBAS[inx1+1] += b * force.YY;
    vBAS[inx2+1] += a * force.YY;
#endif
#if ( DIM > 2 )
    vBAS[inx1+2] += b * force.ZZ;
    vBAS[inx2+2] += a * force.ZZ;
#endif
}


//==============================================================================
#pragma mark -
#pragma mark Links


/**
 Update Meca to include an interaction between A and B
 The force is linear with a zero resting length:
 @code
 force_A = weight * ( B - A )
 force_B = weight * ( A - B )
 @endcode
 
 In practice, Meca::interLink() will update the matrix mB,
 adding \a weight at the indices corresponding to A and B.
 
 Note: with modulo, the position of the fibers may be shifted in space,
 and a correction is necessary to make the force calculation correct:
 
 @code
 force_A = weight * ( B - A - offset )
 force_B = weight * ( A - B + offset )
 @endcode

 Here 'offset' is a multiple of the space periodicity, corresponding to B-A:
 modulo->foldOffset( A - B, offset )

 In practice, Meca::interLink() will update the vector vBAS[]:
 @code
 vBAS[A] += weight * offset;
 vBAS[B] -= weight * offset;
 @endcode

 In principle, what goes to vBAS[] with modulo can be derived
 simply by multiplying the matrix block by 'offset'.
 */

void Meca::interLink(const PointExact & pta,
                     const PointExact & ptb, 
                     const real weight)
{
    assert_true( weight >= 0 );
    if ( pta.neighbors(ptb) )
        return;
    
    const index_type inxA = pta.matIndex();
    const index_type inxB = ptb.matIndex();
    assert_true( inxA != inxB );
    
    mB( inxA, inxA ) -= weight;
    mB( inxA, inxB ) += weight;
    mB( inxB, inxB ) -= weight;
    
    if ( modulo )
    {
        Vector offset, ab = pta.pos() - ptb.pos();
        modulo->foldOffset( ab, offset );
        for ( int dd = 0 ; dd < DIM; ++dd )
        {
            if ( offset[dd] )
            {
                vBAS[DIM*inxA+dd] += weight * offset[dd];
                vBAS[DIM*inxB+dd] -= weight * offset[dd];
            }
        }
    }
    
#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0x0000FFFF,pta.pos(),ptb.pos());
#endif    
}

//==============================================================================
/**
Update Meca to include an interaction between A and B
 The force is linear with a zero resting length:
 force_A = weight * ( B - A )
 force_B = weight * ( A - B )
 */

void Meca::interLink(const PointInterpolated & pta, 
                     const PointExact & ptb, 
                     const real weight)
{
    assert_true( weight >= 0 );
    if ( pta.overlapping(ptb) )
        return;
    
    //coefficients on the points:
    const real  c[] = { pta.coef2(), pta.coef1(), -1.0 };
    const real cw[] = { weight*c[0], weight*c[1], -weight };
    
    //the index of the points in the matrix mB:
    const index_type inx[] = { pta.matIndex1(), pta.matIndex2(), ptb.matIndex() };
    
    for ( int kk = 0;  kk < 3; ++kk )
        for ( int ll = kk; ll < 3; ++ll )
            mB( inx[kk], inx[ll]) -= c[kk] * cw[ll];
    
    if ( modulo )
    {
        Vector offset, ab = pta.pos() - ptb.pos();
        modulo->foldOffset( ab, offset );
        for ( int dd = 0 ; dd < DIM; ++dd )
        {
            if ( offset[dd] )
                for ( int kk = 0; kk < 3 ; ++kk )
                    vBAS[DIM*inx[kk]+dd] += cw[kk] * offset[dd];
        }
    }
    
#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0x00FF00FF,pta.pos(),ptb.pos());
#endif    
}

//==============================================================================
/**
Update Meca to include an interaction between A and B
 The force is linear with a zero resting length:
 force_A = weight * ( B - A )
 force_B = weight * ( A - B )
 */

void Meca::interLink(const PointInterpolated & pta,
                     const PointInterpolated & ptb,
                     const real weight)
{
    assert_true( weight >= 0 );
    if ( pta.overlapping(ptb) )
        return;
    
    //interpolation coefficients:
    const real  c[] = { pta.coef2(), pta.coef1(), -ptb.coef2(), -ptb.coef1() };
    const real cw[] = { weight*c[0], weight*c[1], weight*c[2], weight*c[3] };
    
    //the index of the points in the matrix mB:
    const index_type inx[] = { pta.matIndex1(), pta.matIndex2(), ptb.matIndex1(), ptb.matIndex2() };
    
    for ( int jj = 0; jj < 4; ++jj )
        for ( int ii = jj; ii < 4; ++ii )
            mB( inx[ii], inx[jj] ) -= c[jj] * cw[ii];

    if ( modulo )
    {
        Vector offset, ab = pta.pos() - ptb.pos();
        modulo->foldOffset( ab, offset );
        for ( int dd = 0 ; dd < DIM; ++dd )
        {
            if ( offset[dd] )
                for ( int kk = 0; kk < 4 ; ++kk )
                    vBAS[DIM*inx[kk]+dd] += cw[kk] * offset[dd];
        }
    }
    
#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0xFFFF00FF,pta.pos(),ptb.pos());
#endif
    
}

//==============================================================================
#pragma mark -
#pragma mark Long Links

/**
 Update Meca to include an interaction between A and B,
 The force is affine with non-zero resting length: 
 force_A = weight * ( B - A ) * ( len / |AB| - 1 )
 force_B = weight * ( A - B ) * ( len / |AB| - 1 )
 */

void Meca::interLongLink(const PointExact & pta, 
                         const PointExact & ptb,
                         real len, 
                         const real weight)
{
    assert_true( weight >= 0 );
    if ( pta.neighbors(ptb) )
        return;        
    
#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0xFFFF00FF, pta.pos(), ptb.pos(), len);
#endif
        
    const index_type inxA = DIM * pta.matIndex();  // coef is +weight
    const index_type inxB = DIM * ptb.matIndex();  // coef is -weight
    
    assert_true( inxA != inxB );
    
    Vector ab = ptb.pos() - pta.pos();
    Vector offset;
    if ( modulo )
        modulo->foldOffset( ab, offset );
    
    const real abn = ab.norm();
    if ( abn < REAL_EPSILON )
        return;
    ab /= abn;

    for ( int dd = 0; dd < DIM; ++dd )
    {
        vBAS[inxA+dd] -= weight * ab[dd] * len;
        vBAS[inxB+dd] += weight * ab[dd] * len;
    }
    
    real m;
    len /= abn;
    
    /* To stabilize the matrix with compression, we remove the negative eigenvalues
        This is done by using len = 1 in the formula if len > 1.0. */
    const bool cooked = ( len > 1.0 );
    
    for ( int ii = 0; ii < DIM; ++ii )
    {
        //diagonal elements
        if ( cooked )
            m = weight * ab[ii] * ab[ii]; //formula below with len=1
        else
            m = weight * ( 1.0 + len * ( ab[ii] * ab[ii] - 1.0 ));
        
        mC( inxA+ii, inxA+ii ) -= m;
        mC( inxA+ii, inxB+ii ) += m;
        mC( inxB+ii, inxB+ii ) -= m;
        
        //off-diagonal elements 
        for ( int jj = ii+1; jj < DIM; ++jj )
        {
            if ( cooked )
                m = weight * ab[ii] * ab[jj];
            else
                m = weight * len * ab[ii] * ab[jj];
            mC( inxA+ii, inxA+jj ) -= m;
            mC( inxA+ii, inxB+jj ) += m;
            mC( inxA+jj, inxB+ii ) += m;
            mC( inxB+ii, inxB+jj ) -= m;
        }
    }
    
    if ( modulo )
    {
        real s = offset * ab;  //scalar product
        for ( int ii = 0; ii < DIM; ++ii )
        {
            if ( cooked )
                m = weight * s * ab[ii];
            else
                m = weight * ( len * s * ab[ii] + ( 1.0 - len ) * offset[ii] );
            vBAS[inxA+ii] -= m;
            vBAS[inxB+ii] += m;
        }
    }
}

//==============================================================================
/**
 Update Meca to include an interaction between A and B,
 The force is affine with non-zero resting length: 
 force_A = weight * ( B - A ) * ( len / |AB| - 1 )
 force_B = weight * ( A - B ) * ( len / |AB| - 1 )
 */

void Meca::interLongLink(const PointInterpolated & pta, 
                         const PointExact & pte, 
                         real len,
                         const real weight )
{
    assert_true( weight >= 0 );
    if ( pta.overlapping(pte) )
        return;
    
#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0xFFFF00FF, pta.pos(), pte.pos(), len);
#endif
    
    //force coefficients on the points:
    const real  c[] = { pta.coef2(), pta.coef1(), -1.0 };
    const real cw[] = { weight*c[0], weight*c[1], -weight };
    
    //index in the matrix mC:
    const index_type inx[] = { DIM*pta.matIndex1(), DIM*pta.matIndex2(), DIM*pte.matIndex() };
    
    Vector ab = pte.pos() - pta.pos();
    
    Vector offset;
    if ( modulo )
        modulo->foldOffset( ab, offset );

    const real abn = ab.norm();
    if ( abn < REAL_EPSILON ) return;
    ab /= abn;
    
    int dk, ii, jj, kk, ll;
    
    for ( ii = 0; ii < 3; ++ii )
        for ( dk = 0; dk < DIM; ++dk )
            vBAS[inx[ii]+dk ] -= cw[ii] * ab[dk] * len;
    
    real m;
    len /= abn;
    
    /* To stabilize the matrix with compression, we remove the negative eigenvalues
        This is done by using len = 1 in the formula if len > 1.0. */
    const bool cooked = ( len > 1.0 );
    
    for ( ii = 0; ii < DIM; ++ii )
    {
        if ( cooked )
            m = ab[ii] * ab[ii];
        else
            m = 1.0 + len * ( ab[ii] * ab[ii] - 1.0 );
        
        for ( kk = 0; kk < 3; ++kk )
            for ( ll = kk; ll < 3; ++ll )
                mC( inx[kk]+ii, inx[ll]+ii ) -= c[kk] * cw[ll] * m;
        
        for ( jj = ii+1; jj < DIM; ++jj )
        {
            if ( cooked )
                m = ab[ii] * ab[jj];
            else
                m = len * ab[ii] * ab[jj];
            for ( kk = 0; kk < 3; ++kk )
                for ( ll = 0; ll < 3; ++ll )
                    mC( inx[kk]+ii, inx[ll]+jj ) -= c[kk] * cw[ll] * m;
        }
    }
    
    if ( modulo )
    {
        real s = offset * ab;  //scalar product
        for ( ii = 0; ii < DIM; ++ii )
        {
            if ( cooked )
                m = s * ab[ii];
            else
                m = len * s * ab[ii] + ( 1.0 - len ) * offset[ii];
            if ( m ) for ( jj = 0; jj < 3; ++jj )
                vBAS[inx[jj]+ii] -= cw[jj] * m;
        }
    }
}


//==============================================================================
/**
 Update Meca to include an interaction between A and B,
 The force is affine with non-zero resting length: 
 force_A = weight * ( B - A ) * ( len / |AB| - 1 )
 force_B = weight * ( A - B ) * ( len / |AB| - 1 )
 */

void Meca::interLongLink(const PointInterpolated & pta, 
                         const PointInterpolated & ptb, 
                         real len, 
                         const real weight )
{
    assert_true( weight >= 0 );
    if ( pta.overlapping(ptb) )
        return;
    
#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0xFFFF00FF, pta.pos(),ptb.pos(), len);
#endif
    
    //force coefficients on the points:
    const real  c[] = { pta.coef2(), pta.coef1(), -ptb.coef2(), -ptb.coef1() };
    const real cw[] = { weight*c[0], weight*c[1], weight*c[2], weight*c[3] };
    
    //index in the matrix mC:
    const index_type inx[] = { DIM*pta.matIndex1(), DIM*pta.matIndex2(), DIM*ptb.matIndex1(), DIM*ptb.matIndex2() };
    
    Vector ab = ptb.pos() - pta.pos();
    
    Vector offset;
    if ( modulo )
        modulo->foldOffset( ab, offset );

    const real abn = ab.norm();
    if ( abn < REAL_EPSILON ) return;
    ab /= abn;

    int dk, ii, jj, kk, ll;
    
    for ( ii = 0; ii < 4; ++ii )
        for ( dk = 0; dk < DIM; ++dk )
            vBAS[inx[ii]+dk ] -= cw[ii] * ab[dk] * len;
    
    real m;
    len /= abn;
    
    /* To stabilize the matrix with compression, we remove the negative eigenvalues
        This is done by using len = 1 in the formula if len > 1.0. */
    const bool cooked = ( len > 1.0 );
    
    for ( ii = 0; ii < DIM; ++ii )
    {
        if ( cooked )
            m = ab[ii] * ab[ii];
        else
            m = 1.0 + len * ( ab[ii] * ab[ii] - 1.0 );
        
        for ( kk = 0; kk < 4; ++kk )
            for ( ll = kk; ll < 4; ++ll )
                mC( inx[kk]+ii, inx[ll]+ii ) -= c[kk] * cw[ll] * m;
        
        for ( jj = ii+1; jj < DIM; ++jj )
        {
            if ( cooked )
                m = ab[ii] * ab[jj];
            else
                m = len * ab[ii] * ab[jj];
            for ( kk = 0; kk < 4; ++kk )
                for ( ll = 0; ll < 4; ++ll )
                    mC( inx[kk]+ii, inx[ll]+jj ) -= c[kk] * cw[ll] * m;
        }
    }
    
    if ( modulo )
    {
        real s = offset * ab;  //scalar product
        for ( ii = 0; ii < DIM; ++ii )
        {
            if ( cooked )
                m = s * ab[ii];
            else
                m = len * s * ab[ii] + ( 1.0 - len ) * offset[ii];
            if ( m ) for ( jj = 0; jj < 4; ++jj )
                vBAS[inx[jj]+ii] -= cw[jj] * m;
        }
    }
}


//==============================================================================
#pragma mark -
#pragma mark Side Links

/**
 Update Meca to include an interaction between A and B,
 but it is taken between B and a point S located on the side of A:
 S = A + len * N,
 where N is a normalized vector orthogonal to the fiber in A.
 S is linarly related to the two model points on the sides of an.
 The force is linear of zero resting length:
 force_S = weight * ( S - B )
 force_B = weight * ( B - S )
 */


#if ( DIM == 2 )

void Meca::interSideLink2D(const PointInterpolated & pta, 
                           const PointExact & ptb, 
                           const real arm,
                           const real weight)
{
    assert_true(weight >= 0);
    if ( pta.overlapping(ptb) )
        return;
    
    //force coefficients on the points:
    const real aa = pta.coef1(), bb = pta.coef2();
    const real ee = arm / pta.len();
    
    //index in the matrix mB:
    index_type inx1 = pta.matIndex1();
    index_type inx2 = pta.matIndex2();
    index_type inxE = ptb.matIndex();
    
    const real aaw = aa * weight, bbw = bb * weight;
    const real eew = weight * ee, eeeew = ee * eew;
    
    //we put the isotropic terms in mB
    mB( inx1, inx1 ) -=  bbw * bb + eeeew;
    mB( inx1, inx2 ) -=  aaw * bb - eeeew;
    mB( inx2, inx2 ) -=  aaw * aa + eeeew;
    
    mB( inxE, inxE ) -=  weight;
    mB( inx1, inxE ) +=  bbw;
    mB( inx2, inxE ) +=  aaw;
    
    //index in the matrix mC:
    inx1 *= DIM;
    inx2 *= DIM;
    inxE *= DIM;
    
    mC(inx1  , inx2+1 ) += eew;
    mC(inx1+1, inx2   ) -= eew;
    
    mC(inx1  , inxE+1 ) -= eew;
    mC(inx1+1, inxE   ) += eew;
    mC(inx2,   inxE+1 ) += eew;
    mC(inx2+1, inxE   ) -= eew;
    
#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0x00FF00FF, pta.pos(), pta.pos()+vecProd(arm,pta.dir()), ptb.pos());
#endif
    
    if ( modulo )
        throw Exception("interSideLink2D not valid with periodic boundary conditions");
}

#elif ( DIM == 3 )

void Meca::interSideLinkS(const PointInterpolated & pta, 
                          const PointExact & ptb, 
                          const Vector & arm,
                          const real len,
                          const real weight)
{
    assert_true(weight >= 0);
    if ( pta.overlapping(ptb) )
        return;
    
    assert_true( len > 0 );
    // vector 'a' is parallel to first Fiber
    Vector a = pta.dir();
    Vector b = arm / len;
    
    // we can set directly the interaction coefficient matrix:
    const real xx=a.XX*a.XX+b.XX*b.XX, xy=a.XX*a.YY+b.XX*b.YY, xz=a.XX*a.ZZ+b.XX*b.ZZ;
    const real yy=a.YY*a.YY+b.YY*b.YY, yz=a.YY*a.ZZ+b.YY*b.ZZ, zz=a.ZZ*a.ZZ+b.ZZ*b.ZZ;
    const real T[9] = { xx, xy, xz, xy, yy, yz, xz, yz, zz };
    
    // we set directly the transformed offset vector:
    const real RB[3] = { arm.XX, arm.YY, arm.ZZ };
    
    // weights and indices:
    const real  c[3] = { pta.coef2(), pta.coef1(), -1.0 };
    const real cw[3] = { -weight*c[0], -weight*c[1], weight };
    const index_type inx[3] = { DIM*pta.matIndex1(), DIM*pta.matIndex2(), DIM*ptb.matIndex() };
    
    // fill the matrix mC
    for ( int ii=0; ii<3; ++ii )
    {
        for ( int x = 0; x < DIM; ++x )
            vBAS[inx[ii]+x] += cw[ii] * RB[x];
        
        const real g = c[ii] * cw[ii];
        for ( int x = 0; x < DIM; ++x )
            for ( int y = x; y < DIM; ++y )
                mC( inx[ii]+x, inx[ii]+y ) += g * T[x+3*y];
        
        for ( int jj=ii+1; jj<3; ++jj )
        {
            const real h = c[ii] * cw[jj];
            for ( int x = 0; x < DIM; ++x )
                for ( int y = 0; y < DIM; ++y )
                    mC( inx[ii]+x, inx[jj]+y ) += h * T[x+3*y];
        }
    }
    
#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0x00FFFFFF, pta.pos(), pta.pos()+arm, ptb.pos());
#endif
    
    if ( modulo )
        throw Exception("interSideLinkS not valid with periodic boundary conditions");
}
#endif


void Meca::interSideLink(const PointInterpolated & pta, 
                         const PointExact & ptb, 
                         const real len,
                         const real weight )
{
    assert_true( weight >= 0 );
    if ( pta.overlapping(ptb) )
        return;
    
#if ( DIM == 1 )
    
    throw Exception("interSideLink() is meaningless in 1D");
    
#elif ( DIM == 2 )
    
    real arm = len * RNG.sign_exc( vecProd(pta.diff(), ptb.pos()-pta.pos()));
    interSideLink2D(pta, ptb, arm, weight);

#elif ( DIM == 3 )
    
    // 'arm' is perpendicular to A-Fiber and link:
    Vector a   = pta.diff();
    Vector as  = ptb.pos() - pta.pos();
    Vector arm = as - ( ( as * a ) / a.normSqr() ) * a;
    real n = arm.norm();
    if ( n > REAL_EPSILON )
        interSideLinkS(pta, ptb, arm * (len / n), len, weight);
    
#endif
}

//==============================================================================
#if ( DIM == 2 )

void Meca::interSideLink2D(const PointInterpolated & pta, 
                           const PointInterpolated & ptb, 
                           const real arm,
                           const real weight)
{
    assert_true(weight >= 0);
    if ( pta.overlapping(ptb) )
        return;
 
    const real aa1 = pta.coef1(), bb1 = pta.coef2();
    const real aa2 = ptb.coef1(), bb2 = ptb.coef2();
    const real ee1 = arm / pta.len();
    
    //index in the matrix mB:
    int inx11 = pta.matIndex1(), inx12 = pta.matIndex2();
    int inx21 = ptb.matIndex1(), inx22 = ptb.matIndex2();
    
    const real aa1w = aa1 * weight, aa2w = aa2 * weight;
    const real bb1w = bb1 * weight, bb2w = bb2 * weight;
    const real ee1w = ee1 * weight, ee1ee1w = ee1 * ee1w;
    
    //we put the isotropic terms in mB
    mB( inx11, inx11 ) -=  bb1w * bb1 + ee1ee1w;
    mB( inx11, inx12 ) += -aa1w * bb1 + ee1ee1w;
    mB( inx12, inx12 ) -=  aa1w * aa1 + ee1ee1w;
    
    mB( inx21, inx21 ) -=  bb2w * bb2;
    mB( inx21, inx22 ) += -aa2w * bb2;
    mB( inx22, inx22 ) -=  aa2w * aa2;
    
    mB( inx11, inx21 ) +=  bb1w * bb2;
    mB( inx11, inx22 ) +=  bb1w * aa2;
    mB( inx12, inx22 ) +=  aa1w * aa2;
    mB( inx12, inx21 ) +=  aa1w * bb2;
    
    //index in the matrix mC:
    inx11 *= DIM;
    inx12 *= DIM;
    inx21 *= DIM;
    inx22 *= DIM;
    
    mC( inx11  , inx12+1 ) += ee1w;
    mC( inx11+1, inx12   ) -= ee1w;
    
    const real ee1bb2w = ee1w*bb2;
    const real ee1aa2w = ee1w*aa2;
    
    mC( inx11, inx21+1 ) -=  ee1bb2w;
    mC( inx11, inx22+1 ) -=  ee1aa2w;
    
    mC( inx11+1, inx21 ) +=  ee1bb2w;
    mC( inx11+1, inx22 ) +=  ee1aa2w;
    
    mC( inx12, inx21+1 ) +=  ee1bb2w;
    mC( inx12, inx22+1 ) +=  ee1aa2w;
    
    mC( inx12+1, inx21 ) -=  ee1bb2w;
    mC( inx12+1, inx22 ) -=  ee1aa2w;
    
#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0x0000FFFF, pta.pos(), pta.pos()+vecProd(arm,pta.diff()), ptb.pos());
#endif
    
    if ( modulo )
        throw Exception("interSideLink2D not valid with periodic boundary conditions");
}

#elif ( DIM == 3 )

void Meca::interSideLinkS(const PointInterpolated & pta, 
                          const PointInterpolated & ptb,
                          const Vector & arm,
                          const real len,
                          const real weight)
{
    assert_true( len > REAL_EPSILON );
    assert_true(weight >= 0);
    if ( pta.overlapping(ptb) )
        return;

    Vector a = pta.dir();
    Vector b = arm / len;
    // Vector c = vecProd(a, b);
    
    // we can set directly the interaction coefficient matrix:
    const real xx=a.XX*a.XX+b.XX*b.XX, xy=a.XX*a.YY+b.XX*b.YY, xz=a.XX*a.ZZ+b.XX*b.ZZ;
    const real yy=a.YY*a.YY+b.YY*b.YY, yz=a.YY*a.ZZ+b.YY*b.ZZ, zz=a.ZZ*a.ZZ+b.ZZ*b.ZZ;
    const real T[9] = { xx, xy, xz, xy, yy, yz, xz, yz, zz };
    
    // we set directly the transformed offset vector:
    const real RB[3] = { arm.XX, arm.YY, arm.ZZ };
    
    // weights and indices:
    const real  c[4] = { pta.coef2(), pta.coef1(), -ptb.coef2(), -ptb.coef1() };
    const real cw[4] = { -weight*c[0], -weight*c[1], -weight*c[2], -weight*c[3] };
    const index_type inx[4] = { DIM*pta.matIndex1(), DIM*pta.matIndex2(), DIM*ptb.matIndex1(), DIM*ptb.matIndex2() };
    
    // fill the matrix mC
    for ( int ii=0; ii<4; ++ii )
    {
        for ( int x = 0; x < DIM; ++x )
            vBAS[inx[ii]+x] += cw[ii] * RB[x];
        
        const real g = c[ii] * cw[ii];
        for ( int x = 0; x < DIM; ++x )
            for ( int y = x; y < DIM; ++y )
                mC( inx[ii]+x, inx[ii]+y ) += g * T[x+3*y];
        
        for ( int jj=ii+1; jj<4; ++jj )
        {
            const real h = c[ii] * cw[jj];
            for ( int x = 0; x < DIM; ++x )
                for ( int y = 0; y < DIM; ++y )
                    mC( inx[ii]+x, inx[jj]+y ) += h * T[x+3*y];
        }
    }
    
#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0x0000FFFF, pta.pos(), pta.pos()+arm, ptb.pos());
#endif
}

#endif


/**
 Update Meca to include an interaction between A and B,
 Which is taken between B and a point S located on the side of A:
 S = A + len * N,
 where N is a normalized vector orthogonal to the fiber in an.
 S is linarly related to the two model points on the sides of A, P1 and P2
 In 3D S is choosen in the plane of P1, P2 and B.
 The force is linear of zero resting length:
 force_S = weight * ( S - B )
 force_B = weight * ( B - S )
 */

void Meca::interSideLink(const PointInterpolated & pta, 
                         const PointInterpolated & ptb, 
                         const real len,
                         const real weight)
{
    assert_true( weight >= 0 );
    if ( pta.overlapping(ptb) )
        return;
    
#if ( DIM == 1 )
    
    throw Exception("interSideLink() is meaningless in 1D");

#elif ( DIM == 2 )
    
    real arm = len * RNG.sign_exc( vecProd(pta.diff(), ptb.pos()-pta.pos()) );
    interSideLink2D(pta, ptb, arm, weight);
    
#elif ( DIM == 3 )
    
    // 'arm' is perpendicular to A-Fiber and link:
    Vector a   = pta.diff();
    Vector as  = ptb.pos() - pta.pos();
    Vector arm = as - ( ( as * a ) / a.normSqr() ) * a;
    real n = arm.norm();
    if ( n > REAL_EPSILON )
        interSideLinkS(pta, ptb, arm * (len / n), len, weight);
    
#endif
    
    if ( modulo )
        throw Exception("interSideLink not valid with periodic boundary conditions");
}

//==============================================================================
#pragma mark -
#pragma mark Side Side Links

#if ( DIM == 2 )

void Meca::interSideSideLink2D(const PointInterpolated & pta,
                               const PointInterpolated & ptb, 
                               const real len,
                               const real weight,
                               int side1, int side2 )
{
    assert_true(weight >= 0);
    if ( pta.overlapping(ptb) )
        return;

    const real aa1 = pta.coef1(), bb1 = pta.coef2();
    const real aa2 = ptb.coef1(), bb2 = ptb.coef2();
    const real ee1 = side1 * len / ( 2 * pta.len() );
    const real ee2 = side2 * len / ( 2 * ptb.len() );

    //index in the matrix mB:
    int inx11 = pta.matIndex1(), inx12 = pta.matIndex2();
    int inx21 = ptb.matIndex1(), inx22 = ptb.matIndex2();
    
    const real aa1w = aa1 * weight, aa2w = aa2 * weight;
    const real bb1w = bb1 * weight, bb2w = bb2 * weight;
    const real ee1w = ee1 * weight, ee1ee1w = ee1 * ee1w;
    const real ee2w = ee2 * weight, ee2ee2w = ee2 * ee2w;
    const real ee1ee2w = ee1 * ee2w;
    
    //we put the isotropic terms in mB
    mB( inx11, inx11 ) -=  bb1w * bb1 + ee1ee1w;
    mB( inx11, inx12 ) -=  aa1w * bb1 - ee1ee1w;
    mB( inx12, inx12 ) -=  aa1w * aa1 + ee1ee1w;
    
    mB( inx21, inx21 ) -=  bb2w * bb2 + ee2ee2w;
    mB( inx21, inx22 ) -=  aa2w * bb2 - ee2ee2w;
    mB( inx22, inx22 ) -=  aa2w * aa2 + ee2ee2w;
    
    mB( inx11, inx21 ) +=  bb1w * bb2 + ee1ee2w;
    mB( inx11, inx22 ) +=  bb1w * aa2 - ee1ee2w;
    mB( inx12, inx22 ) +=  aa1w * aa2 + ee1ee2w;
    mB( inx12, inx21 ) +=  aa1w * bb2 - ee1ee2w;
    
    
    //index in the matrix mC:
    inx11 *= DIM;
    inx12 *= DIM;
    inx21 *= DIM;
    inx22 *= DIM;
    
    mC( inx11  , inx12+1 ) += ee1w;
    mC( inx11+1, inx12   ) -= ee1w;
    
    mC( inx21  , inx22+1 ) += ee2w;
    mC( inx21+1, inx22   ) -= ee2w;
    
    const real ee1aa2w = ee1w*aa2;
    const real ee1bb2w = ee1w*bb2;
    const real ee2aa1w = ee2w*aa1;
    const real ee2bb1w = ee2w*bb1;
    
    mC( inx11, inx21+1 ) +=  ee2bb1w - ee1bb2w;
    mC( inx11, inx22+1 ) -=  ee2bb1w + ee1aa2w;
    
    mC( inx11+1, inx21 ) +=  ee1bb2w - ee2bb1w;
    mC( inx11+1, inx22 ) +=  ee1aa2w + ee2bb1w;
    
    mC( inx12, inx21+1 ) +=  ee2aa1w + ee1bb2w;
    mC( inx12, inx22+1 ) +=  ee1aa2w - ee2aa1w;
    
    mC( inx12+1, inx21 ) -=  ee1bb2w + ee2aa1w;
    mC( inx12+1, inx22 ) -=  ee1aa2w - ee2aa1w;
    
#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0x0000FFFF,
                           pta.pos(), pta.pos()+vecProd(ee1,pta.diff()),
                           ptb.pos()+vecProd(ee2,ptb.diff()), ptb.pos());
#endif
    
    if ( modulo )
        throw Exception("interSideSideLink2D not valid with periodic boundary conditions");
}

#endif


/**
 Update Meca to include an interaction between A and B,
 but the links are maded between SA and SB which are located
 on the side of A and B, respectively:
 SA = A + len * N_A,
 SB = B + len * N_B,
 N_X is a normalized vector orthogonal to the fiber carrying X, in X:
 The force is linear of zero resting length,
 force_SA = weight * ( SA - SB )
 force_SB = weight * ( SB - SA )
 */

void Meca::interSideSideLink(const PointInterpolated & pt1,
                             const PointInterpolated & pt2, 
                             const real len,
                             const real weight )
{
#if ( DIM == 1 )
    
    throw Exception("interSideSideLink() meaningless in 1D");
    
#elif ( DIM == 2 )
    
    Vector dir = pt2.pos() - pt1.pos();
    int side1 = RNG.sign_exc( vecProd(pt1.diff(), dir) );
    int side2 = RNG.sign_exc( vecProd(dir, pt2.diff()) );
    interSideSideLink2D(pt1, pt2, len, weight, side1, side2);
    
#elif ( DIM == 3 )
    
    throw Exception("interSideSideLink() was not implemented in 3D");
    
#endif
}

//==============================================================================
#pragma mark -
#pragma mark Sliding Links

/**
 Update Meca to include an interaction between A and B,
 The force is linear of zero resting length, but is anisotropic:
 The component of the force parallel to the fiber in A is removed

 If T is the normalized direction of the fiber in A:
 force_A = weight * ( 1 - T T' ) ( A - B )
 force_B = weight * ( 1 - T T' ) ( B - A )
 */

void Meca::interSlidingLink(const PointInterpolated & pta,
                            const PointExact & pte, 
                            const real weight)
{
    assert_true( weight >= 0 );
    if ( pta.overlapping(pte) )
        return;
    
    //force coefficients on the points:
    const real aa = pta.coef1();
    const real bb = pta.coef2();
    const real bbbb = bb * bb, aaaa = aa * aa, aabb = aa * bb;
    
    Vector dir = pta.dir();
    
    //index in the matrix mC:
    const index_type inx1 = DIM * pta.matIndex1();
    const index_type inx2 = DIM * pta.matIndex2();
    const index_type inxE = DIM * pte.matIndex();
    
    // on points (a, b, e), (ab) being the PointInterpolated, and e the PointExact,
    // P is the projection on the plane perpendicular to (ab): P.v= (v - (T.v)T/normSqr(T))
    // the interaction is  -weigth * transpose(bb, aa, -1) * P * ( bb, aa, -1 )
    // we set only the upper part of this symmetric matrix:
    real P;
    for ( int xx=0; xx<DIM; ++xx )
    {
        for ( int yy=xx; yy<DIM; ++yy )
        {
            P = weight * ((xx==yy) - dir[xx]*dir[yy] );
            
            mC( inx1+xx, inx1+yy ) -= bbbb * P;
            mC( inx1+xx, inx2+yy ) -= aabb * P;
            mC( inx2+xx, inx2+yy ) -= aaaa * P;
            mC( inx1+xx, inxE+yy ) += bb * P;
            mC( inx2+xx, inxE+yy ) += aa * P;
            mC( inxE+xx, inxE+yy ) -= P;
            if ( xx != yy )
            {
                mC( inx1+yy, inx2+xx ) -= aabb * P;
                mC( inx1+yy, inxE+xx ) += bb * P;
                mC( inx2+yy, inxE+xx ) += aa * P;
            }
        }
    }
    
    if ( modulo )
    {
        real ov[DIM];
        Vector offset, ab = pta.pos() - pte.pos();
        modulo->foldOffset(ab, offset);
        for ( int xx=0; xx<DIM; ++xx )
        {
            ov[xx] = 0;
            for ( int yy=0; yy<DIM; ++yy )
            {
                P = weight*((xx==yy) - dir[xx]*dir[yy] );
                ov[xx] += P*offset[yy]; 
            }
            vBAS[inx1+xx] += bb * ov[xx];
            vBAS[inx2+xx] += aa * ov[xx];
            vBAS[inxE+xx] -= ov[xx];
        }
    }

}

//==============================================================================
/**
Update Meca to include an interaction between A and B,
 The force is linear of zero resting length, but is anisotropic:
 The component of the force parallel to the fiber in A is removed
 
 If T is the normalized direction of the fiber in A:
 force_A = weight * ( 1 - T T' ) ( A - B )
 force_B = weight * ( 1 - T T' ) ( B - A )
 */

void Meca::interSlidingLink(const PointInterpolated & pta,
                            const PointInterpolated & ptb, 
                            const real weight)
{
    assert_true( weight >= 0 );
    if ( pta.overlapping(ptb) )
        return;
    
    //interpolation coefficients
    const real  c[4] = { pta.coef2(), pta.coef1(), -ptb.coef2(), -ptb.coef1() };
    const real cw[4] = { -weight*c[0], -weight*c[1], -weight*c[2], -weight*c[3] };

    //the index of the points in the matrix mB:
    const index_type inx[] = { DIM*pta.matIndex1(), DIM*pta.matIndex2(), DIM*ptb.matIndex1(), DIM*ptb.matIndex2() };
    
    // on points (a, b, e), (ab) being the PointInterpolated, and e the PointExact,
    // P is the projection on the plane perpendicular to (ab): P.v= (v - (T.v)T/normSqr(T))
    // the interaction is  -wh' * P * h
    // we set only the upper part of this symmetric matrix:
    
    Vector dir = pta.dir();

#if ( DIM == 1 )
    
    throw Exception("interSlidingLink() meaningless in 1D");
    
#elif ( DIM == 2 )
    
    real dd[] = { dir.XX*dir.XX, dir.XX*dir.YY, dir.YY*dir.YY };

    for ( int jj = 0; jj < 4; ++jj )
    {
        const real g = c[jj] * cw[jj];
        mC( inx[jj],   inx[jj]   ) += ( 1.0 - dd[0] ) * g;
        mC( inx[jj]+1, inx[jj]+1 ) += ( 1.0 - dd[2] ) * g;
        mC( inx[jj],   inx[jj]+1 ) += (     - dd[1] ) * g;
        
        for ( int ii = jj+1; ii < 4; ++ii )
        {
            const real h = c[jj] * cw[ii];
            mC( inx[ii],   inx[jj]   ) += ( 1.0 - dd[0] ) * h;
            mC( inx[ii]+1, inx[jj]+1 ) += ( 1.0 - dd[2] ) * h;
            mC( inx[ii],   inx[jj]+1 ) += (     - dd[1] ) * h;
            mC( inx[ii]+1, inx[jj]   ) += (     - dd[1] ) * h;
        }
    }
    
    if ( modulo )
    {
        Vector offset, ab = pta.pos() - ptb.pos();
        modulo->foldOffset(ab, offset);
        // matrix(dd)*offset
        real ov[2] = { ( 1.0 - dd[0] ) * offset[0] - dd[1] * offset[1], -dd[1] * offset[0] + ( 1.0 - dd[2] ) * offset[1]};
        for ( int ii = 0; ii < 4; ++ii )
        {
            vBAS[inx[ii]]   -= cw[ii] * ov[0];
            vBAS[inx[ii]+1] -= cw[ii] * ov[1];
        }
    }
    
#elif ( DIM == 3 )
    
    real dd[] = { dir.XX*dir.XX, dir.XX*dir.YY, dir.XX*dir.ZZ, dir.YY*dir.YY, dir.YY*dir.ZZ, dir.ZZ*dir.ZZ };

    for ( int jj = 0; jj < 4; ++jj )
    {
        const real g = c[jj] * cw[jj];
        mC( inx[jj],   inx[jj]   ) += ( 1.0 - dd[0] ) * g;
        mC( inx[jj]+1, inx[jj]+1 ) += ( 1.0 - dd[3] ) * g;
        mC( inx[jj]+2, inx[jj]+2 ) += ( 1.0 - dd[5] ) * g;
        mC( inx[jj],   inx[jj]+1 ) += (     - dd[1] ) * g;
        mC( inx[jj],   inx[jj]+2 ) += (     - dd[2] ) * g;
        mC( inx[jj]+1, inx[jj]+2 ) += (     - dd[4] ) * g;
        
        for ( int ii = jj+1; ii < 4; ++ii )
        {
            const real h = c[jj] * cw[ii];
            mC( inx[ii],   inx[jj]   ) += ( 1.0 - dd[0] ) * h;
            mC( inx[ii]+1, inx[jj]+1 ) += ( 1.0 - dd[3] ) * h;
            mC( inx[ii]+2, inx[jj]+2 ) += ( 1.0 - dd[5] ) * h;
            mC( inx[ii],   inx[jj]+1 ) += (     - dd[1] ) * h;
            mC( inx[ii],   inx[jj]+2 ) += (     - dd[2] ) * h;
            mC( inx[ii]+1, inx[jj]+2 ) += (     - dd[4] ) * h;
            mC( inx[ii]+1, inx[jj]   ) += (     - dd[1] ) * h;
            mC( inx[ii]+2, inx[jj]   ) += (     - dd[2] ) * h;
            mC( inx[ii]+2, inx[jj]+1 ) += (     - dd[4] ) * h;
        }
    }
    
    if ( modulo )
    {
        Vector offset, ab = pta.pos() - ptb.pos();
        modulo->foldOffset(ab, offset);
        const real ov[3] = { ( 1.0 - dd[0] ) * offset[0] - dd[1] * offset[1] - dd[2]*offset[2], 
                       -dd[1] * offset[0] + ( 1.0 - dd[3] ) * offset[1] - dd[4] * offset[2],
                       -dd[2] * offset[0] - dd[4] * offset[1] + ( 1.0 - dd[5] ) * offset[2]};
        for ( unsigned int ii = 0; ii < 4; ++ii )
        {
            for( unsigned int x = 0; x < DIM; ++x ) 
                vBAS[inx[ii]+x]   -= cw[ii] * ov[x];        
        }
    }
#endif
    

}

//==============================================================================
#pragma mark -
#pragma mark Side Sliding Links

#if ( DIM == 2 )

void Meca::interSideSlidingLink2D(const PointInterpolated & pta,
                                  const PointExact & pte, 
                                  const real arm,
                                  const real weight)
{
    assert_true(weight >= 0);
    if ( pta.overlapping(pte) )
        return;
#if ( 1 )
    
    Vector dir = pta.dir();
    const real aa = pta.coef1(), bb = pta.coef2();
    real ee = arm / pta.len();
    
    //this is done numerically by multiplying the matrices
    
    // matrix of coefficients:
    real T[2*6] = {  bb, -ee, ee, bb,  aa, ee, -ee, aa,  -1,  0,  0, -1  };
    
    // the projection matrix:
    const real P[4] = { 1-dir.XX*dir.XX, -dir.XX*dir.YY, -dir.XX*dir.YY, 1-dir.YY*dir.YY };
    
    real PT[2*6], TPT[6*6];
    blas_xgemm('N','N', 2, 6, 2, -weight, P, 2, T, 2, 0.0, PT, 2);
    blas_xgemm('T','N', 6, 6, 2, 1.0, T, 2, PT, 2, 0.0, TPT, 6);
    
    //printf("\n");  VecPrint::matPrint(6,6, TPT);
    const index_type inx[] = {
        DIM*pta.matIndex1(), DIM*pta.matIndex1()+1, 
        DIM*pta.matIndex2(), DIM*pta.matIndex2()+1,
        DIM*pte.matIndex(),  DIM*pte.matIndex()+1
    };
    
    for ( int ii=0; ii<6; ++ii )
        for ( int jj=ii; jj<6; ++jj )
            mC(inx[ii], inx[jj]) += TPT[ii+6*jj];
    
    
    if ( modulo )
    {
        Vector offset, ab = pte.pos() - pta.pos();
        modulo->foldOffset(ab, offset);
        
        for ( int ii=0; ii<6; ++ii )
        {
            vBAS[inx[ii]] -= TPT[ii+6*4] * offset[0];
            vBAS[inx[ii]] -= TPT[ii+6*5] * offset[1];
        }
    }
    
#else
    
    // choose vector 'a' parallel to the first Fiber:
    Vector a = pta.dir();
    
    const int side = RNG.sign_exc(arm);
    const real bXX = -side*a.YY, bYY = side*a.XX;
    const real T[4] = { bXX*bXX, bXX*bYY, bXX*bYY, bYY*bYY };
    
    // we set directly the transformed offset vector:
    const real RB[2] = { -arm*a.YY, arm*a.XX };
    
    // weights and indices:
    const real  c[3] = { pta.coef2(), pta.coef1(), -1.0 };
    const real cw[3] = { -weight*c[0], -weight*c[1], -weight*c[2] };
    const index_type inx[3] = { DIM*pta.matIndex1(), DIM*pta.matIndex2(), DIM*pte.matIndex() };
    
    /* We can optimize with a function to set two elements of the matrix at a time:
     A(i,j) and A(i,j+1). We can write a special function for the diagonal terms */
    
    // fill the matrix mC (
    for ( unsigned int ii=0; ii<3; ++ii )
    {
        vBAS[inx[ii]  ] += cw[ii] * RB[0];
        vBAS[inx[ii]+1] += cw[ii] * RB[1];
        
        const real g = c[ii] * cw[ii];
        mC( inx[ii]  , inx[ii]   ) += g * T[0];
        mC( inx[ii]  , inx[ii]+1 ) += g * T[2];
        mC( inx[ii]+1, inx[ii]+1 ) += g * T[3];
        
        for ( int jj=ii+1; jj<3; ++jj )
        {
            const real h = c[ii] * cw[jj];
            mC( inx[ii]  , inx[jj]   ) += h * T[0];
            mC( inx[ii]  , inx[jj]+1 ) += h * T[2];
            mC( inx[ii]+1, inx[jj]   ) += h * T[1];
            mC( inx[ii]+1, inx[jj]+1 ) += h * T[3];
        }
    }
    
    if ( modulo )
        throw Exception("interSideSlidingLink2D not valid with periodic boundary conditions");
#endif
    
    
#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0x0000FFFF,pta.pos(),pta.pos()+vecProd(arm, pta.dir()),pte.pos());
#endif
}

#elif ( DIM == 3 )

/**
 arm must be parallel to link
 */

void Meca::interSideSlidingLinkS(const PointInterpolated & pta,
                                 const PointExact & pte,
                                 const Vector & arm,
                                 const real len,
                                 const real weight)
{    
    assert_true( len > 0 );
    assert_true(weight >= 0);
    if ( pta.overlapping(pte) )
        return;
    // choose vector 'a' parallel to the first Fiber:
    Vector a = pta.dir();
    // vector 'b' aligned with the link:
    Vector b = arm / len;
    
    /*      
     Without tangential force, a 'long link' is in the perpendicular direction.
     In the local reference frame, the matrix of interaction coefficients would be:
     real T[9] = { 0, 0, 0, 0, -weight, 0, 0, 0, 0 };
     we could transform it with a change-of-coordinates matrix R:
     Vector c = vecProd(a, b);
     real R[9] = { a.XX, a.YY, a.ZZ, b.XX, b.YY, b.ZZ, c.XX, c.YY, c.ZZ };
     real TR[3*3];
     blas_xgemm('N','T', 3, 3, 3, 1.0, T, 3, R, 3, 0.0, TR, 3);
     blas_xgemm('N','N', 3, 3, 3, 1.0, R, 3, TR, 3, 0.0, T, 3);
     equivalently, we can set directly the interaction coefficient matrix: 
     */
    
    const real xx=b.XX*b.XX, xy=b.XX*b.YY, xz=b.XX*b.ZZ;
    const real yy=b.YY*b.YY, yz=b.YY*b.ZZ, zz=b.ZZ*b.ZZ;
    const real T[9] = { xx, xy, xz, xy, yy, yz, xz, yz, zz };
    
    // we set directly the transformed offset vector:
    const real RB[3] = { len*b.XX, len*b.YY, len*b.ZZ };
    
    // weights and indices:
    const real  c[3] = { pta.coef2(), pta.coef1(), -1.0 };
    const real cw[3] = { -weight*c[0], -weight*c[1], -weight*c[2] };
    const index_type inx[3] = { DIM*pta.matIndex1(), DIM*pta.matIndex2(), DIM*pte.matIndex() };
    
    // fill the matrix mC
    for ( unsigned int ii=0; ii<3; ++ii )
    {
        for ( int x = 0; x < DIM; ++x )
            vBAS[inx[ii]+x] += cw[ii] * RB[x];
        
        const real g = c[ii] * cw[ii];
        for ( unsigned int x = 0; x < DIM; ++x )
            for ( int y = x; y < DIM; ++y )
                mC( inx[ii]+x, inx[ii]+y ) += g * T[x+3*y];
        
        for ( unsigned int jj=ii+1; jj<3; ++jj )
        {
            const real h = c[ii] * cw[jj];
            for ( int x = 0; x < DIM; ++x )
                for ( int y = 0; y < DIM; ++y )
                    mC( inx[ii]+x, inx[jj]+y ) += h * T[x+3*y];
        }
    }
        
    if ( modulo )
    {
        Vector offset, ab = pte.pos() - pta.pos();
        modulo->foldOffset(ab, offset);
        const real ov[3] = { xx * offset[0] + xy * offset[1] + xz * offset[2], 
                             xy * offset[0] + yy * offset[1] + yz * offset[2],
                             xz * offset[0] + yz * offset[1] + zz * offset[2]};
        for ( unsigned int ii = 0; ii < 3; ++ii )
        {
            for ( unsigned int x = 0; x < DIM; ++x )
                vBAS[inx[ii]+x]  +=  cw[ii] * ov[x];
        }
    }
    
#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0x0000FFFF,pta.pos(),pta.pos()+Vector(RB),pte.pos());
#endif
    
}
#endif

/**
 Update Meca to include an interaction between A and B,
 This is a combination of a SideLink with a Sliding Link:
 The force is linear of zero resting length, but it is taken between B,
 and another point S located on the side of A:
 S = A + len * N,
 where N is a normalized vector orthogonal to the fiber in A, in the direction of B.
 In addition, the tangential part of the force is removed.
 
 If T is the normalized direction of the fiber in A:
 force_S = weight * ( 1 - T T' ) ( S - B )
 force_B = weight * ( 1 - T T' ) ( B - S )
 */
void Meca::interSideSlidingLink(const PointInterpolated & pta, 
                                const PointExact & ptb, 
                                const real len,
                                const real weight)
{
#if ( DIM == 1 )
    
    throw Exception("interSideLink() is meaningless in 1D");
    
#elif ( DIM == 2 )
    
    Vector as = ptb.pos()-pta.pos();
    if ( modulo )
        modulo->fold(as);
    real arm  = len * RNG.sign_exc( vecProd(pta.diff(), as) );
    interSideSlidingLink2D(pta, ptb, arm, weight);
    
#elif ( DIM == 3 )
    
    // 'arm' is perpendicular to Fiber and parallel to link:
    Vector a   = pta.diff();
    Vector as  = ptb.pos() - pta.pos();
    if ( modulo ) 
        modulo->fold(as);
    Vector arm = ( as - ( ( as * a ) / a.normSqr() ) * a );
    real n = arm.norm();
    if ( n > REAL_EPSILON )
        interSideSlidingLinkS(pta, ptb, arm * ( len / n ), len, weight);
    
#endif
}



//==============================================================================

#if ( DIM == 2 )


void Meca::interSideSlidingLink2D(const PointInterpolated & pta,
                                  const PointInterpolated & ptb, 
                                  const real arm,
                                  const real weight)
{
    assert_true(weight >= 0);
    if ( pta.overlapping(ptb) )
        return;
#if ( 1 )
    
    Vector dir = pta.dir();
    const real aa = pta.coef1(), bb = pta.coef2();
    const real cc = ptb.coef1(), dd = ptb.coef2();
    
    real ee = arm / pta.len();

    //this is done the 'hard' way by multiplying all matrices
    //coefficient matrix:
    real T[2*8] = { bb, -ee, ee, bb, aa, ee, -ee, aa, -dd, 0, 0, -dd, -cc, 0, 0, -cc };
    
    //the projection matrix:
    const real P[4] = { 1-dir.XX*dir.XX, -dir.XX*dir.YY, -dir.XX*dir.YY, 1-dir.YY*dir.YY };
    
    real PT[2*8], TPT[8*8];
    blas_xgemm('N','N', 2, 8, 2, -weight, P, 2, T, 2, 0.0, PT, 2);
    blas_xgemm('T','N', 8, 8, 2, 1.0, T, 2, PT, 2, 0.0, TPT, 8);
    
    //printf("\n");  VecPrint::matPrint(8,8, TPT);
    const index_type inx[] = {
        DIM*pta.matIndex1(),  DIM*pta.matIndex1()+1,
        DIM*pta.matIndex2(),  DIM*pta.matIndex2()+1,
        DIM*ptb.matIndex1(),  DIM*ptb.matIndex1()+1,
        DIM*ptb.matIndex2(),  DIM*ptb.matIndex2()+1 };
    
    for ( unsigned int ii=0; ii<8; ++ii )
        for ( unsigned int jj=ii; jj<8; ++jj )
            mC(inx[ii], inx[jj]) += TPT[ii+8*jj];
    
    if ( modulo )
    {
        Vector offset, ab = ptb.pos() - pta.pos();
        modulo->foldOffset(ab, offset);
        
        for ( int ii=0; ii<8; ++ii )
        {
            vBAS[inx[ii]] -= TPT[ii+8*4] * offset[0];
            vBAS[inx[ii]] -= TPT[ii+8*5] * offset[1];
            vBAS[inx[ii]] -= TPT[ii+8*6] * offset[0];
            vBAS[inx[ii]] -= TPT[ii+8*7] * offset[1];
        }
    }
    
#else
    
    // vector 'a' parallel to the first Fiber:
    Vector a = pta.dir();
    
    // vector 'b' perpendicular to 'a', and aligned with the link:
    const int side = RNG.sign_exc(arm);
    const real bXX = -side*a.YY, bYY = side*a.XX;
    const real T[4] = { bXX*bXX, bXX*bYY, bXX*bYY, bYY*bYY };
    
    // we set directly the transformed offset vector:
    const real RB[2] = { -arm*a.YY, arm*a.XX };

    // weights and indices:
    const real  c[4] = { pta.coef2(), pta.coef1(), -ptb.coef2(), -ptb.coef1() };
    const real cw[4] = { -weight*c[0], -weight*c[1], -weight*c[2], -weight*c[3] };
    const index_type inx[4] = { DIM*pta.matIndex1(), DIM*pta.matIndex2(), DIM*ptb.matIndex1(), DIM*ptb.matIndex2() };
    
    // fill the matrix mC
    for ( unsigned int ii=0; ii<4; ++ii )
    {
        vBAS[inx[ii]  ] += cw[ii] * RB[0];
        vBAS[inx[ii]+1] += cw[ii] * RB[1];
        
        const real g = c[ii] * cw[ii];
        mC( inx[ii]  , inx[ii]   ) += g * T[0];
        mC( inx[ii]  , inx[ii]+1 ) += g * T[2];
        mC( inx[ii]+1, inx[ii]+1 ) += g * T[3];
        
        for ( unsigned int jj=ii+1; jj<4; ++jj )
        {
            const real h = c[ii] * cw[jj];
            mC( inx[ii]  , inx[jj]   ) += h * T[0];
            mC( inx[ii]  , inx[jj]+1 ) += h * T[2];
            mC( inx[ii]+1, inx[jj]   ) += h * T[1];
            mC( inx[ii]+1, inx[jj]+1 ) += h * T[3];
        }
    }
    
    if ( modulo )
    {
        real dd[] = { a.XX*a.XX, a.XX*a.YY, a.YY*a.YY };
        Vector offset, ab = pta.pos() - ptb.pos();
        modulo->foldOffset(ab, offset);
        real ov[2] = { ( 1.0 - dd[0] ) * offset[0] - dd[1] * offset[1], -dd[1] * offset[0] + ( 1.0 - dd[2] ) * offset[1]};
        for ( int ii = 0; ii < 4; ++ii )
        {
            for ( int x = 0; x < 2 ; ++x )
            vBAS[inx[ii] + x] -= cw[ii] * ov[x];
        }
    }
#endif
    
    
#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0x0000FFFF,pta.pos(),pta.pos()+vecProd(arm, pta.dir()),ptb.pos());
#endif
}


#elif ( DIM == 3 )

/**
 arm must be parallel to link
 */
void Meca::interSideSlidingLinkS(const PointInterpolated & pta,
                                 const PointInterpolated & ptb, 
                                 const Vector & arm,
                                 const real len,
                                 const real weight)
{
    assert_true( len > 0 );
    assert_true(weight >= 0);
    if ( pta.overlapping(ptb) )
        return;

    // choose vector 'a' parallel to the first Fiber:
    Vector a = pta.dir();
    // make vector 'b' perpendicular to 'a', and aligned with the link:
    Vector b = arm / len;
    
    /*
     Without tangential force, a 'long link' is in the perpendicular direction.
     In the local reference frame, the matrix of interaction coefficients would be:
     real T[9] = { 0, 0, 0, 0, -weight, 0, 0, 0, 0 };
     we could transform it with a change-of-coordinates matrix R:
     Vector c = vecProd(a, b);
     real R[9] = { a.XX, a.YY, a.ZZ, b.XX, b.YY, b.ZZ, c.XX, c.YY, c.ZZ };
     real TR[3*3];
     blas_xgemm('N','T', 3, 3, 3, 1.0, T, 3, R, 3, 0.0, TR, 3);
     blas_xgemm('N','N', 3, 3, 3, 1.0, R, 3, TR, 3, 0.0, T, 3);
     equivalently, we can set directly the interaction coefficient matrix: 
     */
    
    const real xx=b.XX*b.XX, xy=b.XX*b.YY, xz=b.XX*b.ZZ;
    const real yy=b.YY*b.YY, yz=b.YY*b.ZZ, zz=b.ZZ*b.ZZ;
    const real T[9] = { xx, xy, xz, xy, yy, yz, xz, yz, zz };
    
    // we set directly the transformed offset vector:
    const real RB[3] = { len*b.XX, len*b.YY, len*b.ZZ };
    
    // weights and indices:
    const real  c[4] = { pta.coef2(), pta.coef1(), -ptb.coef2(), -ptb.coef1() };
    const real cw[4] = { -weight*c[0], -weight*c[1], -weight*c[2], -weight*c[3] };
    const index_type inx[4] = { DIM*pta.matIndex1(), DIM*pta.matIndex2(), DIM*ptb.matIndex1(), DIM*ptb.matIndex2() };
    
    // fill the matrix mC
    for ( unsigned int ii=0; ii<4; ++ii )
    {
        for ( int x = 0; x < DIM; ++x )
            vBAS[inx[ii]+x] += cw[ii] * RB[x];
        
        const real g = c[ii] * cw[ii];
        for ( int x = 0; x < DIM; ++x )
            for ( int y = x; y < DIM; ++y )
                mC( inx[ii]+x, inx[ii]+y ) += g * T[x+3*y];
        
        for ( int jj=ii+1; jj<4; ++jj )
        {
            const real h = c[ii] * cw[jj];
            for ( int x = 0; x < DIM; ++x )
                for ( int y = 0; y < DIM; ++y )
                    mC( inx[ii]+x, inx[jj]+y ) += h * T[x+3*y];
        }
    }

    if ( modulo )
    {
        Vector offset, ab = pta.pos() - ptb.pos();
        modulo->foldOffset(ab, offset);
        const real ov[3] = { T[0] * offset[0] + T[3] * offset[1] + T[6] * offset[2], 
            T[1] * offset[0] + T[4] * offset[1] + T[7] * offset[2],
            T[2] * offset[0] + T[5] * offset[1] + T[8] * offset[2]};
        for ( int ii = 0; ii < 4; ++ii )
        {
            for ( int x = 0; x < DIM; ++x )
                vBAS[inx[ii]+x]  -= cw[ii] * ov[x];
        }
    }
      
#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0x0000FFFF,pta.pos(),pta.pos()+Vector(RB),ptb.pos());
#endif
    
}

#endif

/**
 Update Meca to include an interaction between A and B,
 This is a combination of Side- and Sliding Links:
 The force is linear of zero resting length, but it is taken between B
 and another point S which is located on the side of A:
 S = A + len * N,
 where N is a normalized vector orthogonal to the fiber in A, in the direction of B.
 In addition, the part of the force tangential to A is removed.
 
 If T is the normalized direction of the fiber in A:
 force_S = weight * ( 1 - T T' ) ( S - B )
 force_B = weight * ( 1 - T T' ) ( B - S )
 */

void Meca::interSideSlidingLink(const PointInterpolated & pta, 
                                const PointInterpolated & ptb, 
                                const real len,
                                const real weight)
{
#if ( DIM == 1 )
    
    throw Exception("interSideSlidingLink() is meaningless in 1D");
    
#elif ( DIM == 2 )
    
    real arm = len * RNG.sign_exc( vecProd(pta.diff(), ptb.pos()-pta.pos()) );
    interSideSlidingLink2D(pta, ptb, arm, weight);
    
#elif ( DIM == 3 )
    
    // 'arm' is perpendicular to A-Fiber and parallel to link:
    Vector a   = pta.diff();
    Vector as  = ptb.pos() - pta.pos();
    if ( modulo ) 
        modulo->fold(as);
    Vector arm = ( as - ( ( as * a ) / a.normSqr() ) * a );
    real n = arm.norm();
    if ( n > REAL_EPSILON )
        interSideSlidingLinkS(pta, ptb, arm * ( len / n ), len, weight);
    
#endif
}




//==============================================================================
#pragma mark -
#pragma mark Torque

#if (DIM == 2)
/**
 Update Meca to include torque between segment A-B and C-D containing pt1 and pt2.
 Implicit version with linearized force 2D
 Angle is between AB and CD. Force is along normal N_A and N_C pointing to the other filament
 L_AB and L_CD is the length of the segments AB and CD
 force_A = torque_weight * ( Delta angle ) * N_A/L_AB =-force_B
 force_C = torque_weight * ( Delta angle ) * N_C/L_CD =-force_D
 Delta_angle is the difference between actual angle and resting angle between AB and CD
 
 Antonio Politi, 2013
 */
void Meca::interTorque2D(const PointInterpolated & pt1,
                         const PointInterpolated & pt2,
                         const real cosinus, const real sinus,
                         const real weight)
{
    assert_true( weight >= 0 );
    if ( pt1.overlapping(pt2) )
        return;
    
    //index in the matrix mC:
    const index_type index[] = { DIM*pt1.matIndex1(),DIM*pt1.matIndex1()+1, DIM*pt1.matIndex2(),
        DIM*pt1.matIndex2()+1, DIM*pt2.matIndex1(), DIM*pt2.matIndex1()+1,  DIM*pt2.matIndex2(),
        DIM*pt2.matIndex2()+1 };
    
    //Vectors and points of torque
    Vector ab = pt1.diff();
    Vector cd = pt2.diff();
    Vector a = pt1.pos1();
    Vector b = pt1.pos2();
    Vector c = pt2.pos1();
    Vector d = pt2.pos2();
    const real coord[]={a.XX, a.YY, b.XX, b.YY, c.XX, c.YY, d.XX, d.YY};
    //Helping vector this vector is at torque_angle from cd.
    //Therefore in resting state angle difference between ab and ce is zero. This vector is used to compute the strength of torque
    Vector ce;
    ce.XX =  cd.XX*cosinus + cd.YY*sinus;
    ce.YY = -cd.XX*sinus   + cd.YY*cosinus;
    //normalize
    const real abn = ab.norm();
    const real abnS= ab.normSqr();
    const real cdn = cd.norm();
    const real cdnS= cd.normSqr();
    if (abn < REAL_EPSILON || cdn < REAL_EPSILON ) return;
    
    //normalize the vectors
    ab /= abn; cd /= cdn; ce /= cdn;
    
    //Coordinates of normal vectors yielding the direction of the force
    //fa = torque_weight*dangle*(h[0], h[1]) = torque_weight*dangle*na/la
    const real h[]={ ab.YY/abn, -ab.XX/abn, -ab.YY/abn, ab.XX/abn, -cd.YY/cdn, cd.XX/cdn, cd.YY/cdn, -cd.XX/cdn };
    
    //dangle = angle - torque_angle
    //real dangle = atan2( vecProd(ab, ce), ab * ce );
    real dangle = atan2( ab.XX*ce.YY - ab.YY*ce.XX, ab * ce );
    //Computation of the jacobian for the linearization
    //M = d_x f = M1 + M2
    //M1 = w1/l normal d_x dangle
    //M2 = w2 * dangle  d_x normal/l
    real w1 = weight;
    real w2 = weight*dangle;
    
    
    //Matrix M1 with k*hxh (outer product) this yieald a matrix stored with its lower triangular part in m. The -w1 is because ab = b-a
    real m[36] = { 0 };
    //blas_xspr('U', 8, -w1, h, 1, m);
    blas_xspr('L', 8, -w1, h, 1, m);
    
    
    
    //Matrix M2
    real Da = w2*( -2*ab.XX*ab.YY )/abnS;
    real da = w2*( ab.XX*ab.XX-ab.YY*ab.YY )/abnS;
    real Dc = w2*( -2*cd.XX*cd.YY )/cdnS;
    real dc = w2*(  cd.XX*cd.XX-cd.YY*cd.YY )/cdnS;
    real entrya[] = {-Da, -da, Da,  da}; //={d(na_x/la)/dxa, d(na_x/la)/dya, d(na_x/l)/dxb, ...}
    real entryc[] = { Dc,  dc,  -Dc,  -dc};//={ d(nc_x/lc)/dxc, d(nc_x/lc)/dyc, d(nc_x/l)/dxd, ...}
    int shifta = 0;
    int shiftc= 26;
    int mm;
    
    //Add second part of matrix.
    //The pos(-1, jj) accounts for the different signs of the matrix
    for ( int jj=0; jj <  4; ++jj) {
        for ( int ii=jj ; ii < 4; ++ii ) {
            m[ii + shifta] += pow(-1,jj)*entrya[ii-jj];
            m[ii + shiftc] += pow(-1,jj)*entryc[ii-jj];
        }
        shifta += 7 - jj;
        shiftc += 3 - jj;
    }
    
    
    //very Cumbersome!!!
    //Entries for Matrix mC  and vector vBas
    //vBas = fa - M*P0
    for ( int ii = 0; ii < 8; ++ii ) {
        vBAS[index[ii]] += w2*h[ii];
        for (int jj = 0; jj < 8; ++jj) {
            if (jj < ii)
                mm = jj*(7.5-0.5*jj)+ii;
            else {
                mm = ii*(7.5-0.5*ii)+jj;
                mC( index[ii], index[jj] ) += m[mm];
            }
            vBAS[index[ii]] -= m[mm]*coord[jj];
        }
    }
}
#endif

//==============================================================================
#pragma mark -
#pragma mark Clamps

/**
 Update Meca to include a link between a point A and a fixed position G
 The force is linear:  
 force_A = weight * ( G - A );
 There is no counter-force in G, since G is immobile.
 */

void Meca::interClamp(const PointExact & pta, 
                      const real g[], 
                      const real weight)
{
    assert_true( weight >= 0 );
    const index_type inx = pta.matIndex();
    
    mB( inx, inx ) -=  weight;
    
    if ( modulo )
    {
        real gm[DIM];
        for ( int dd=0; dd < DIM; ++dd )
            gm[dd] = g[dd];
        modulo->fold( gm, pta.pos() );
        
        for ( unsigned int dd = 0; dd < DIM; ++dd )
            vBAS[DIM*inx+dd] += weight * gm[dd];
    }
    else
    {
        for ( unsigned int dd = 0; dd < DIM; ++dd )
            vBAS[DIM*inx+dd] += weight * g[dd];
    }
}


//==============================================================================
/**
 Update Meca to include a link between a point A and a fixed position G
 The force is linear:  
 force_A = weight * ( G - A );
 The point G is not associated to a Mecable, and there is no counter-force in G.
 */

void Meca::interClamp(const PointInterpolated & pti, 
                      const real g[], 
                      const real weight)
{
    assert_true( weight >= 0 );
    
    index_type inx1 = pti.matIndex1();
    index_type inx2 = pti.matIndex2();

    assert_true( inx1 != inx2 );
    
    const real c2 = pti.coef1(),   c2w = weight * c2;
    const real c1 = pti.coef2(),   c1w = weight * c1;
    
    assert_true( 0 <= c1  &&  c1 <= 1 );
    assert_true( 0 <= c2  &&  c2 <= 1 );
    
    mB( inx1, inx1 ) -=  c1w * c1;
    mB( inx1, inx2 ) -=  c2w * c1;
    mB( inx2, inx2 ) -=  c2w * c2;
    
    inx1 *= DIM;
    inx2 *= DIM;
    
    if ( modulo )
    {
        real gm[DIM];
        for ( int d = 0; d < DIM; ++d )
            gm[d] = g[d];
        modulo->fold( gm, pti.pos() );
        
        for ( int dd=0; dd < DIM; ++dd )
        {
            vBAS[inx1+dd] += c1w * gm[dd];
            vBAS[inx2+dd] += c2w * gm[dd];
        }
    }
    else
    {
        for ( int dd=0; dd < DIM; ++dd )
        {
            vBAS[inx1+dd] += c1w * g[dd];
            vBAS[inx2+dd] += c2w * g[dd];
        }
    }
}


//==============================================================================
#pragma mark -
#pragma mark Long Clamps

/**
Update Meca to include a non-zero resting force between A (pta) and G (center).
 The force is affine with non-zero resting length: 
 force_A = (G-A) * ( len / |AG| - 1 )
 There is no force on G, which is an immobile position.
 */

void Meca::interLongClamp(const PointExact & pta, 
                          const Vector & center, 
                          real len, 
                          const real weight)
{
    assert_true( weight >= 0 );
    const index_type inx = DIM * pta.matIndex();
    
    Vector axis = pta.pos() - center;
    const real axis_n = axis.norm();
    if ( axis_n < REAL_EPSILON ) return;
    axis /= axis_n;
    
    if ( len < axis_n )
    {
        len /= axis_n;
        
        real facX = weight * len * ( axis_n + axis * center );
        real facC = weight * ( 1.0 - len );
        
        for ( unsigned int ii = 0; ii < DIM; ++ii )
        {
            mC( inx+ii, inx+ii ) += weight * ( len * ( 1.0 - axis[ii] * axis[ii] ) - 1.0 );
            
            for ( unsigned int jj = ii+1; jj < DIM; ++jj )
                mC( inx+ii, inx+jj ) -= weight * len * axis[ii] * axis[jj];
            
            vBAS[inx+ii] += facX * axis[ii] + facC * center[ii];
        }
    }
    else
    {
        real facX = weight * ( len + axis * center );
        
        for ( unsigned int ii = 0; ii < DIM; ++ii )
        {
            for ( unsigned int jj = ii; jj < DIM; ++jj )
                mC( inx+ii, inx+jj ) -= weight * axis[ii] * axis[jj];
            
            vBAS[inx+ii] += facX * axis[ii];
        }
    }
    
    if ( modulo )
        throw Exception("interLongClamp not valid with periodic boundary conditions");
}


//==============================================================================
#pragma mark -
#pragma mark Side Clamps


#if ( DIM == 2 )

void Meca::interSideClamp2D(const PointInterpolated & pta,
                            const real g[], 
                            const real arm, 
                            const real weight)
{
    //force coefficients on the points:
    const real aa = pta.coef1(),   aaw = weight * aa;
    const real bb = pta.coef2(),   bbw = weight * bb;
    
    const real ee = arm / pta.len();
    const real eew = weight * ee;
    const real ee2w = weight * ee * ee;
    
    //index in the matrix mB:
    index_type inx1 = pta.matIndex1();
    index_type inx2 = pta.matIndex2();
    
    //we put the isotropic terms in mB
    mB( inx1, inx1 ) -=  bb * bbw + ee2w;
    mB( inx1, inx2 ) -=  aa * bbw - ee2w;
    mB( inx2, inx2 ) -=  aa * aaw + ee2w;
    
    //index in the matrix mC:
    inx1 *= DIM;
    inx2 *= DIM;
    
    mC(inx1  , inx2+1) += eew;
    mC(inx1+1, inx2  ) -= eew;
    
    //it seems to works also fine without the term in eew* below:
    vBAS[inx1  ] += bbw * g[0] - eew * g[1];
    vBAS[inx1+1] += bbw * g[1] + eew * g[0];
    vBAS[inx2  ] += aaw * g[0] + eew * g[1];
    vBAS[inx2+1] += aaw * g[1] - eew * g[0];

#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0x00FFFFFF, pta.pos(), pta.pos()+vecProd(arm, pta.dir()), Vector::make(g));
#endif
    
    if ( modulo )
        throw Exception("interSideClamp2D not valid with periodic boundary conditions");
}

#elif ( DIM == 3 )

/**
 A link of stiffness weight, between offset_point on the side of pta, and the fixed position \a g.

 This uses the vector product x -> arm ^ x to offset the point on which the link is acting:
 offset_point = fiber_point + arm ^ fiber_dir,
 with fiber_point = pta.pos() and fiber_dir = pta.diff().normalized.
 
 arm must be perpendicular to link ( g - pta.pos() )

 F. Nedelec, March 2011
 */
void Meca::interSideClamp3D(const PointInterpolated & pta,
                            const real g[], 
                            const Vector & arm,
                            const real weight)
{
    real aa = pta.coef2();
    real bb = pta.coef1();
    
    real s = 1.0 / pta.len();

    real ex = s * arm.XX;
    real ey = s * arm.YY;
    real ez = s * arm.ZZ;
    
    // indices to mC:
    const index_type inx1 = DIM * pta.matIndex1();
    const index_type inx2 = DIM * pta.matIndex2();
    const index_type inx[6] = { inx1, inx1+1, inx1+2, inx2, inx2+1, inx2+2 };
    
    /* The transfer matrix transforms the two PointExact in pta,
     to the side point S:
     S = aa * pt1 + bb * pt2 + arm ^ ( pt2 - pt1 ).normalized
     
     It was generated in Maxima:
     MVP: matrix([0, -ez, ey], [ez, 0, -ex], [-ey, ex, 0]);
     MD: addcol(-ident(3), ident(3));
     MC: addcol(aa*ident(3), bb*ident(3));
     T: MC+MVP.MD;
     */
    const real T[18] = {
         aa,  ez, -ey,  bb, -ez,  ey,
        -ez,  aa,  ex,  ez,  bb, -ex,
         ey, -ex,  aa, -ey,  ex,  bb
    };
    
#if ( 0 )
    
    real TT[36];
    // TT = transpose(T) * T
    blas_xsyrk('U','N', 6, 3, 1.0, T, 6, 0.0, TT, 6);
    
#else
    
    real a2 = aa * aa;
    real b2 = bb * bb;
    real ab = aa * bb;
    
    real exx = ex * ex, exy = ex*ey, exz = ex*ez;
    real eyy = ey * ey, eyz = ey*ez;
    real ezz = ez * ez;
    
    // TT = transpose(T) * T is symmetric, and thus we only set half of it:
    /* Maxima code:
    TT: expand(transpose(T) . T);
     */
    real TT[36] = {
        eyy+ezz+a2,  0,           0,           0,           0,           0,
        -exy,        exx+ezz+a2,  0,           0,           0,           0,
        -exz,       -eyz,         exx+eyy+a2,  0,           0,           0,
        -ezz-eyy+ab, ez+exy,      exz-ey,      eyy+ezz+b2,  0,           0,
        -ez+exy,    -ezz-exx+ab,  eyz+ex,     -exy,         exx+ezz+b2,  0,
        exz+ey,      eyz-ex,     -eyy-exx+ab, -exz,        -eyz,         exx+eyy+b2
    };
    
#endif
    
    // we project to bring all forces in the plane perpendicular to 'arm'
    real sca = 1.0 / arm.norm();
    real aan = aa * sca;
    real bbn = bb * sca;
    real TP[6] = { aan*ex, aan*ey, aan*ez, bbn*ex, bbn*ey, bbn*ez };    
    //blas_xgemm('N','N', 6, 1, 3, sca, T, 6, arm, 3, 0.0, TP, 6);

    blas_xsyrk('U','N', 6, 1, weight, TP, 6, -weight, TT, 6);
    
    for ( int ii=0; ii<6; ++ii )
    {
        for ( int jj=ii; jj<6; ++jj )
            mC(inx[ii], inx[jj]) += TT[ii+6*jj];
    }
    
    // { gx, gy, gz } is the projection of g[] in the plane perpendicular to 'arm'
    real ws = ( arm * Vector::make(g) ) * sca * sca;
    real gx = weight * ( g[0] - ws * arm.XX );
    real gy = weight * ( g[1] - ws * arm.YY );
    real gz = weight * ( g[2] - ws * arm.ZZ );
    
    for ( int ii=0; ii<6; ++ii )
        vBAS[inx[ii]] += T[ii] * gx + T[ii+6] * gy + T[ii+12] * gz;
                 
#ifdef DISPLAY_INTERACTIONS
    if ( showInteractions )
        displayInteraction(0x00FFFFFF, pta.pos(), pta.pos()+vecProd(arm, pta.dir()), Vector(g));
#endif
    
    if ( modulo )
        throw Exception("interSideClamp3D not valid with periodic boundary conditions");
}

#endif  

/**
 Update Meca to include a connection between A and a fixed position G.
 The force is of zero resting length, but it is taken between B
 and another point S which is located on the side of A:
 S = A + len * N,
 The force is linear:  
 force_S = weight * ( G - S )
 There is no counter-force in G, since G is immobile.
 */

void Meca::interSideClamp(const PointInterpolated & pta,
                          const real g[], 
                          const real len, 
                          const real weight)
{
    assert_true( weight >= 0 );
    
#if ( DIM == 1 )
    
    throw Exception("interSideClamp() meaningless in 1D");
    
#elif ( DIM == 2 )
    
    // 'arm' is a vector in the Z direction
    real arm = len * RNG.sign_exc( vecProd(pta.diff(), Vector::make(g)-pta.pos()));
    interSideClamp2D(pta, g, arm, weight);
   
#elif ( DIM == 3 )
    
    // 'arm' perpendicular to link and fiber is obtained by vector product:
    Vector arm = vecProd( pta.pos()-Vector::make(g), pta.diff() );
    real n = arm.norm();
    if ( n > REAL_EPSILON )
        interSideClamp3D(pta, g, arm * ( len / n ), weight);

#endif  
}


//==============================================================================
#pragma mark -
#pragma mark Sliding Clamps

/**
 Update Meca to include a link between a point A and a fixed position G
 The force is linear and the parallel component is removed:  
 force_A = weight * ( 1 - T T' )( G - A )
 T is the vector tangent to the fiber in an.
 There is no counter-force in G, since G is immobile.
 */

void Meca::interSlidingClamp( const PointInterpolated & pta,
                              const Vector & g,
                              const real weight )
{
    assert_true( weight >= 0 );
    
    //force coefficients on the points:
    const real aa = pta.coef1();
    const real bb = pta.coef2();
    const real bbbb = bb * bb, aaaa = aa * aa, aabb = aa * bb;
    
    Vector dir = pta.dir();
    
    //index in the matrix mC:
    const index_type inx1 = DIM * pta.matIndex1();
    const index_type inx2 = DIM * pta.matIndex2();
    
    //build the upper part of the projection on ga:
    for ( int xx=0; xx<DIM; ++xx )
    {
        for ( int yy=xx; yy<DIM; ++yy )
        {
            real P = weight * ((xx==yy) - dir[xx]*dir[yy]);
            
            mC( inx1+xx, inx1+yy ) -= bbbb * P;
            mC( inx2+xx, inx2+yy ) -= aaaa * P;
            mC( inx1+xx, inx2+yy ) -= aabb * P;
            if ( xx != yy )
                mC( inx1+yy, inx2+xx ) -= aabb * P;
        }
    }
    
    //add the constant term:
    real pr = g * dir;
    for ( int xx=0; xx<DIM; ++xx )
    {
        vBAS[inx1+xx] += weight * bb * ( g[xx] - pr * dir[xx] );
        vBAS[inx2+xx] += weight * aa * ( g[xx] - pr * dir[xx] );
    }
    
    if ( modulo )
        throw Exception("interSlidingClamp not valid with periodic boundary conditions");
}


//==============================================================================
#pragma mark -
#pragma mark Plane

/**
 Add interaction between \a pta and the plane defined by \a G and the normal \a dir
 The force is linear and the parallel components are removed:  
 force(A) = weight * ( dir dir' )( G - A )
 
 This corresponds to a frictionless plane.
 
 The vector \a dir should be of norm = 1, or alternatively
 \a weight can be divided by the norm of \a dir.
 */

void Meca::interPlane(const PointExact & pta, 
                      const Vector & dir,
                      const Vector & g,
                      const real weight )
{
    assert_true( weight >= 0 );
    
    const index_type inx = DIM * pta.matIndex();
    const real pr = ( g * dir ) * weight;
    
    mC(inx  , inx   ) -= dir.XX * dir.XX * weight;
    vBAS[inx  ] += pr * dir.XX;
    
#if ( DIM >= 2 )
    mC(inx  , inx+1 ) -= dir.XX * dir.YY * weight;
    mC(inx+1, inx+1 ) -= dir.YY * dir.YY * weight;
    vBAS[inx+1] += pr * dir.YY;
#endif
    
#if ( DIM >= 3 )
    mC(inx  , inx+2 ) -= dir.XX * dir.ZZ * weight;
    mC(inx+1, inx+2 ) -= dir.YY * dir.ZZ * weight;
    mC(inx+2, inx+2 ) -= dir.ZZ * dir.ZZ * weight;
    vBAS[inx+2] += pr * dir.ZZ;
#endif
}


/**
 Add interaction between \a pta and the plane defined by \a G and the normal \a dir
 The force is linear and the parallel components are removed:
 force(A) = weight * ( dir dir' )( G - A )
 
 This corresponds to a frictionless plane.
 
 The vector \a dir should be of norm = 1, or alternatively 
 \a weight can be divided by the norm of \a dir.
 */

void Meca::interPlane( const PointInterpolated & pta,
                       const Vector & dir,
                       const Vector & g,
                       const real weight )
{
    assert_true( weight >= 0 );
    
    //force coefficients on the points:
    const real aa = pta.coef1();
    const real bb = pta.coef2();
    const real bbbb = bb * bb, aaaa = aa * aa, aabb = aa * bb;
    
    //index in the matrix mC:
    const index_type inx1 = DIM * pta.matIndex1();
    const index_type inx2 = DIM * pta.matIndex2();
    
    //build the upper part of the projection on ga:
    for ( unsigned int xx=0; xx<DIM; ++xx )
    {
        for ( unsigned int yy=xx; yy<DIM; ++yy )
        {
            real P = weight * dir[xx] * dir[yy];
            
            mC( inx1+xx, inx1+yy ) -= bbbb * P;
            mC( inx2+xx, inx2+yy ) -= aaaa * P;
            mC( inx1+xx, inx2+yy ) -= aabb * P;
            if ( xx != yy )
                mC( inx1+yy, inx2+xx ) -= aabb * P;
        }
    }
    
    //add the constant term:
    const real pr = weight * ( g * dir );
    for ( unsigned int xx=0; xx<DIM; ++xx )
    {
        vBAS[inx1+xx ] += pr * bb * dir[xx];
        vBAS[inx2+xx ] += pr * aa * dir[xx];
    }
}

