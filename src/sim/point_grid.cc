// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "assert_macro.h"
#include "point_grid.h"
#include "exceptions.h"
#include "messages.h"
#include "modulo.h"
#include "space.h"
#include "meca.h"

extern Modulo const* modulo;

//------------------------------------------------------------------------------

PointGrid::PointGrid()
: max_diameter(0)
{
}


void PointGrid::setGrid(Space const* space, Modulo const* modulo, real min_step)
{
    if ( min_step <= REAL_EPSILON )
        return;
    
    Vector ext = space->extension();
    
    bool periodic = false;
    int cellNb[3];
    for ( int d = 0; d < DIM; ++d )
    {
        if ( ext[d] < 0 )
            throw InvalidParameter("space:dimension should be >= 0");

        real n = 2 * ext[d] / min_step;
        
        if ( modulo  &&  modulo->isPeriodic(d) )
        {
            //adjust the grid to match the edges
            cellNb[d] = (int)floor(n);
            if ( cellNb[d] <= 0 )
                cellNb[d] = 1;
            periodic = true;
        }
        else
        {
            //add a border in any dimension which is not periodic
            cellNb[d] = (int)ceil(n) + 2;
            n = cellNb[d] * 0.5 * min_step;
            assert_true( n >= ext[d] );
            ext[d] = n;
        }
    }
    
    //create the grid using the calculated dimensions:
    mGrid.periodic(periodic);
    mGrid.setDimensions(-ext, ext, cellNb);
    mGrid.createCells();

    //Create side regions suitable for pairwise interactions:
    mGrid.createSideRegions(1);

    //The maximum allowed diameter of particles is half the minimum cell width
    max_diameter = mGrid.minimumWidth(1);

    //report the grid size used
    Cytosim::MSG(5, "PointGrid set with %i cells:", mGrid.nbCells());
    for ( int ii = 0; ii < DIM; ++ii )
        Cytosim::MSG(5, "  %.1fum / %i bins,", 2*ext[ii], cellNb[ii]);
    Cytosim::MSG(5, "  max_diameter %.3f\n", max_diameter);
}


//------------------------------------------------------------------------------
#pragma mark -

/// include verifications that the grid is appropriate for the particule radius
#define CHECK_RANGE 1



void PointGrid::add(const PointExact & p, real rd) const
{
    Vector w = p.pos();
    point_list(w).new_val().set(p, rd, w);
    
#if ( CHECK_RANGE )
    //we check that the grid would correctly detect collision of two particles
    if ( max_diameter < 2 * rd )
    {
        std::ostringstream oss;
        oss << "simul:steric_max_range is too short" << std::endl;
        oss << PREF << "steric_max_range should be greater than 2 * particle-radius" << std::endl;
        oss << PREF << "= " << 2 * rd << " for some particles" << std::endl;
        throw InvalidParameter(oss.str());
    }
#endif
}


void PointGrid::add(const FiberLocus & p, real rd, real rg) const
{
    //we use the middle of the segment (interpolation coefficient is ignored)
    Vector w = p.center();
    locus_list(w).new_val().set(p, rd, rg);
    
#if ( CHECK_RANGE )
    //we check that the grid would correctly detect collision of two segments
    //along the diagonal, which corresponds to the worst-case scenario
    real diag = sqrt( p.len() * p.len() + rg * rg );
    if ( max_diameter < diag )
    {
        std::ostringstream oss;
        oss << "simul:steric_max_range is too short" << std::endl;
        oss << PREF << "steric_max_range should be greater than sqrt( sqr(segment_length) + 4*sqr(range) )" << std::endl;
        oss << PREF << "where normally segment_length ~ 4/3 segmentation" << std::endl;
        oss << PREF << "= " << diag << " for some fibers" << std::endl;
        throw InvalidParameter(oss.str());
    }
#endif
}


//------------------------------------------------------------------------------
#pragma mark - Steric functions


/**
 This is used to check two spherical objects:
 Solid/Bead/Sphere or Fiber-tip
 
 The force is applied if the objects are closer than the
 sum of their radiuses.
 */
void PointGrid::checkPP(Meca& meca, PointGridParam const& pam, FatPoint const& aa, FatPoint const& bb) const
{
    const real len = aa.radius + bb.radius;
    Vector vab = bb.pos - aa.pos;
    
    if ( modulo )
        modulo->fold(vab);
    
    if ( vab.normSqr() < len*len )
        meca.interLongLink(aa.pe, bb.pe, len, pam.stiff_push);
}


/**
 This is used to check a segment of a fiber against a spherical object:
 Solid/Bead/Sphere or Fiber-tip.
 
 The force is applied if the objects are closer to the sum of their radiuses.
 */
void PointGrid::checkPL(Meca& meca, PointGridParam const& pam, FatPoint const& aa, FatLocus const& bb) const
{
    const real len = aa.radius + bb.radius;
    
    // get position of point with respect to segment:
    real a, d;
    bb.fl.projectPoint0(aa.pos, a, d);
    
    if ( 0 <= a )
    {
        if ( a <= bb.fl.len() )
        {
            if ( d < len*len )
            {
                PointInterpolated bi(bb.fl, a);
                meca.interSideSlidingLink(bi, aa.pe, len, pam.stiff_push);
            }
        }
        else
        {
            if ( bb.isLast() )
                checkPP(meca, pam, aa, bb.point2());
        }
    }
    else
    {
        if ( bb.isFirst() )
            checkPP(meca, pam, aa, bb.point1());
        else
        {
            /* we check the projection to the previous segment,
             and if it falls on the right of it, then we interact with the node */
            Vector vab = aa.pos - bb.fl.pos1();
            
            if ( modulo )
                modulo->fold(vab);
            
            if ( vab * bb.fl.fiber()->diffPoints(bb.fl.point()-1) >= 0 )
            {
                if ( vab.normSqr() < len*len )
                    meca.interLongLink(aa.pe, bb.fl.exact1(), len, pam.stiff_push);
            }
        }
    }
}


/**
 This is used to check a segment of a fiber against the non-end model-point of a fiber.
 
 The interaction is applied only if the model-point projects 'inside' the segment.
 */
void PointGrid::checkLL1(Meca& meca, PointGridParam const& pam, FatLocus const& aa, FatLocus const& bb) const
{
    const real ran = aa.range + bb.radius;
    
    // get position of bb.point1() with respect to segment 'aa'
    real a, d = INFINITY;
    aa.fl.projectPoint0(bb.fl.pos1(), a, d);
    
    if ( d < ran*ran )
    {
        /*
         bb.point1() projects inside segment 'aa'
         */
        assert_true( 0 <= a  &&  a <= aa.fl.len() );
        const real len = aa.radius + bb.radius;
        PointInterpolated ai(aa.fl, a);
        if ( d > len*len )
            meca.interSideSlidingLink(ai, bb.fl.exact1(), len, pam.stiff_pull);
        else
            meca.interSideSlidingLink(ai, bb.fl.exact1(), len, pam.stiff_push);
    }
    else if ( a < 0 )
    {
        if ( aa.isFirst() )
        {
            /*
             Check the projection of aa.point1(),
             on the segment represented by 'bb'
             */
            if ( &bb < &aa  &&  bb.isFirst() )
            {
                Vector vab = bb.fl.pos1() - aa.fl.pos1();
                
                if ( modulo )
                    modulo->fold(vab);
                
                const real len = aa.radius + bb.radius;
                if ( vab.normSqr() < len*len  &&  vab * bb.fl.diff() >= 0 )
                    meca.interLongLink(aa.fl.exact1(), bb.fl.exact1(), len, pam.stiff_push);
            }
        }
        else
        {
            /*
             Check the projection to the segment located before 'aa',
             and interact if 'bb.point1()' falls on the right side of it
             */
            Vector vab = bb.fl.pos1() - aa.fl.pos1();
            
            if ( modulo )
                modulo->fold(vab);
            
            if ( vab * aa.fl.fiber()->diffPoints(aa.fl.point()-1) >= 0 )
            {
                const real d = vab.normSqr();
                if ( d < ran*ran )
                {
                    const real len = aa.radius + bb.radius;
                    if ( d > len*len )
                        meca.interLongLink(aa.fl.exact1(), bb.fl.exact1(), len, pam.stiff_pull);
                    else
                        meca.interLongLink(aa.fl.exact1(), bb.fl.exact1(), len, pam.stiff_push);
                }
            }
        }
    }
}


/**
 This is used to check a segment of a fiber against the non-end model-point of a fiber.
 
 The interaction is applied only if the model-point projects 'inside' the segment.
 */
void PointGrid::checkLL2(Meca& meca, PointGridParam const& pam, FatLocus const& aa, FatLocus const& bb) const
{
    const real ran = aa.range + bb.radius;
    
    // get position of bb.point2() with respect to segment 'aa'
    real a, d = INFINITY;
    aa.fl.projectPoint0(bb.fl.pos2(), a, d);
    
    if ( d < ran*ran )
    {
        /*
         bb.point2() projects inside segment 'aa'
         */
        assert_true( 0 <= a  &&  a <= aa.fl.len() );
        const real len = aa.radius + bb.radius;
        PointInterpolated ai(aa.fl, a);
        if ( d > len*len )
            meca.interSideSlidingLink(ai, bb.fl.exact2(), len, pam.stiff_pull);
        else
            meca.interSideSlidingLink(ai, bb.fl.exact2(), len, pam.stiff_push);
    }
    else if ( a < 0 )
    {
        /*
         Check the projection to the segment located before 'aa',
         and interact if 'bb.point1()' falls on the right side of it
         */
        Vector vab = bb.fl.pos2() - aa.fl.pos1();
        
        if ( modulo )
            modulo->fold(vab);
        
        if ( aa.isFirst() )
        {
            assert_true(bb.isLast());
            const real len = aa.radius + bb.radius;
            if ( vab.normSqr() < len*len  &&  vab * bb.fl.diff() <= 0 )
                meca.interLongLink(aa.fl.exact1(), bb.fl.exact2(), len, pam.stiff_push);
        }
        else
        {
            if ( vab * aa.fl.fiber()->diffPoints(aa.fl.point()-1) >= 0 )
            {
                const real d = vab.normSqr();
                if ( d < ran*ran )
                {
                    const real len = aa.radius + bb.radius;
                    if ( d > len*len )
                        meca.interLongLink(aa.fl.exact1(), bb.fl.exact2(), len, pam.stiff_pull);
                    else
                        meca.interLongLink(aa.fl.exact1(), bb.fl.exact2(), len, pam.stiff_push);
                }
            }
        }
    }
    else if ( &bb < &aa  &&  aa.isLast()  &&  a > aa.fl.len() )
    {
        /*
         Check the projection of aa.point2(),
         on the segment represented by 'bb'
         */
        assert_true(bb.isLast());

        Vector vab = bb.fl.pos2() - aa.fl.pos2();
            
        if ( modulo )
            modulo->fold(vab);
            
        const real len = aa.radius + bb.radius;
        if ( vab.normSqr() < len*len  &&  vab * bb.fl.diff() <= 0 )
            meca.interLongLink(aa.fl.exact2(), bb.fl.exact2(), len, pam.stiff_push);
    }
}


/**
 This is used to check two FiberLocus, that each represent a segment of a Fiber.
 The segments are tested for intersection in 3D.
 */
void PointGrid::checkLL(Meca& meca, PointGridParam const& pam, FatLocus const& aa, FatLocus const& bb) const
{
    checkLL1(meca, pam, aa, bb);
    
    if ( aa.isLast() )
        checkLL2(meca, pam, bb, aa);
    
    checkLL1(meca, pam, bb, aa);
    
    if ( bb.isLast() )
        checkLL2(meca, pam, aa, bb);

#if ( DIM == 3 )
    
    const real ran = std::max(aa.range+bb.radius, aa.radius+bb.range);

    /* in 3D, we use projectSegment() to calculate the closest distance
     between two segments, and use the result to build an interaction */
    real a, b, d;
    if ( aa.fl.projectSegment(bb.fl, a, b, d)  &&  d < ran*ran )
    {
        const real len = aa.radius + bb.radius;
        
        PointInterpolated ai(aa.fl, a);
        PointInterpolated bi(bb.fl, b);
        
        if ( d > len*len )
            meca.interSideSlidingLink(ai, bi, len, pam.stiff_pull);
        else
            meca.interSideSlidingLink(ai, bi, len, pam.stiff_push);
    }

#endif
}

//------------------------------------------------------------------------------
#pragma mark -


/**
 Check interactions between the FatPoints contained in Pane `pan1`, and `pan2`.
 */
void  PointGrid::setInteractions(Meca& meca, PointGridParam const& pam) const
{
    assert_true(pam.stiff_push >= 0);
    assert_true(pam.stiff_pull >= 0);

    // scan all cells to examine each pair of particles:
    for ( unsigned indx = 0; indx < mGrid.nbCells(); ++indx )
    {
        int * region;
        int nr = mGrid.getRegion(region, indx);
        assert_true(region[0] == 0);
        /*
         We should consider each pair of objects (ii, jj) only once.
         If we are handling the list of particles that are in the same cell (reg==0),
         then index jj starts at ii+1. If however, we handle lists from different cells,
         then all possible values of jj are considered.
         */
        
        FatPointList & baseP = point_list(indx);
        FatLocusList & baseL = locus_list(indx);
        
        for ( FatPoint* ii = baseP.begin(); ii < baseP.end(); ++ii )
        {
            for ( FatPoint* jj = ii+1; jj < baseP.end(); ++jj )
                checkPP(meca, pam, *ii, *jj);
            
            for ( FatLocus* jj = baseL.begin(); jj < baseL.end(); ++jj )
                checkPL(meca, pam, *ii, *jj);
        }
        
        for ( FatLocus* ii = baseL.begin(); ii < baseL.end(); ++ii )
            for ( FatLocus* jj = ii+1; jj < baseL.end(); ++jj )
                checkLL(meca, pam, *ii, *jj);
        
        for ( int reg = 1; reg < nr; ++reg )
        {
            FatPointList & sideP = point_list(indx+region[reg]);
            FatLocusList & sideL = locus_list(indx+region[reg]);
            
            for ( FatPoint* ii = baseP.begin(); ii < baseP.end(); ++ii )
            {
                for ( FatPoint* jj = sideP.begin(); jj < sideP.end(); ++jj )
                    checkPP(meca, pam, *ii, *jj);
                
                for ( FatLocus* jj = sideL.begin(); jj < sideL.end(); ++jj )
                    checkPL(meca, pam, *ii, *jj);
            }
            
            for ( FatLocus* ii = baseL.begin(); ii < baseL.end(); ++ii )
            {
                for ( FatPoint* jj = sideP.begin(); jj < sideP.end(); ++jj )
                    checkPL(meca, pam, *jj, *ii);
                
                for ( FatLocus* jj = sideL.begin(); jj < sideL.end(); ++jj )
                    checkLL(meca, pam, *ii, *jj);
            }
        }
    }
}

