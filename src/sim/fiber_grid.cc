// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "assert_macro.h"
#include "rasterizer.h"
#include "fiber_grid.h"
#include "exceptions.h"
#include "fiber_locus.h"
#include "fiber_binder.h"
#include "messages.h"
#include "space.h"
#include "modulo.h"
#include "hand.h"
#include "hand_prop.h"
#include "simul.h"
#include "sim.h"
extern Random RNG;

#if ( 0 )
// this includes a naive implementation, which is slow but helpful for debugging
#   include "fiber_grid2.cc"
#else

/**
 Creates a grid where the dimensions of the cells are \a max_step at most.
 If the numbers of cells that need to be created is greater than \a max_nb_cells,
 the function returns 1 without building the grid.
 The return value is zero in case of success.
 
 The algorithm works with any value of \a max_step (the results are always correct),
 but \a max_steps affects the efficiency (speed) of the algorithm:
 -if max_step is too slow, paintGrid() will be slow,
 -if max_step is too large, tryToAttach() will be slow.
 A good compromise is to set max_step equivalent to the attachment distance,
 or at least to the size of the segments of the Fibers.
 */
int FiberGrid::setGrid(const Space * space, const Modulo * mod, real max_step, unsigned long max_nb_cells)
{
    if ( max_step <= 0 )
        throw InvalidParameter("simul:binding_grid_step should be > 0");
    
    //set gridRange=0 to trigger an error if paintGrid() is not called:
    gridRange = 0;
    
    modulo = mod;
    Vector range = space->extension();
    
    bool periodic = false;
    int nCells[3] = { 1, 1, 1 };
    
    for ( int d = 0; d < DIM; ++d )
    {
        if ( range[d] < 0 )
            throw InvalidParameter("space:dimension should be >= 0");

        real n = 2 * range[d] / max_step;
        nCells[d] = (int) ceil(n);
        
        if ( modulo  &&  modulo->isPeriodic(d) )
        {
            //adjust the grid to match the edges exactly
            periodic = true;
            if ( nCells[d] <= 0 )
                nCells[d] = 1;
        }
        else
        {
            //extend the grid beyond the borders of the space
            nCells[d] += 2;
            n = nCells[d] * 0.5 * max_step;
            assert_true( n >= range[d] );
            range[d] = n;
        }
    }

    //create the grid using the calculated dimensions:
    if ( periodic )
        mGrid.periodic(true);
    else
        modulo = 0;
    
    mGrid.setDimensions(-range, range, nCells);
    
    // we check the number of cells, to avoid crazy memory requirements
    if ( mGrid.nbCells() > max_nb_cells )
        return 1;
    
    mGrid.createCells();
    
    //report the grid size used
    Cytosim::MSG(5, "FiberGrid set with %i cells", mGrid.nbCells());
    for ( int ii = 0; ii < DIM; ++ii )
        Cytosim::MSG(5, ",  %.1fum / %i bins", 2*range[ii], nCells[ii]);
    Cytosim::MSG(5, " (binding_grid_step=%.3f)\n", max_step);
        
    return 0;
}


//-----------------------------------------------------------------------

bool FiberGrid::hasGrid() const
{
    return mGrid.hasCells();
}


void FiberGrid::clear()
{
    // this is to be able to detect if paintGrid() is not called:
    gridRange = 0;
    
    mGrid.clear();
}


//------------------------------------------------------------------------------
/** 
 paintCell(x,y,z) adds a Segment to the SegmentList associated with
 the grid point (x,y,z). 
 It is called by the rasterizer function paintFatLine().
 
 This version uses the fact that cells with consecutive
 X-coordinates should be consecutive also in the Grid
 */

void paintCell(const int x_inf, const int x_sup, const int y, const int z, void * arg1, void * arg2)
{
    FiberLocus const* seg = static_cast<FiberLocus const*>(arg1);
    FiberGrid::grid_type * mGrid = static_cast<FiberGrid::grid_type *>(arg2);
    //printf("paint %p in (%i to %i, %i, %i)\n", seg, x_inf, x_sup, y, z);

#if   ( DIM == 1 )
    FiberGrid::SegmentList & inf = mGrid->cell1D( x_inf );
    FiberGrid::SegmentList & sup = mGrid->cell1D( x_sup );
#elif ( DIM == 2 )
    FiberGrid::SegmentList & inf = mGrid->cell2D( x_inf, y );
    FiberGrid::SegmentList & sup = mGrid->cell2D( x_sup, y );
#elif ( DIM == 3 )
    FiberGrid::SegmentList & inf = mGrid->cell3D( x_inf, y, z );
    FiberGrid::SegmentList & sup = mGrid->cell3D( x_sup, y, z );
#endif
    
    for ( FiberGrid::SegmentList * list = &inf; list <= &sup; ++list )
        list->push_back(seg);
}


/** 
 paintCellPeriodic(x,y,z) adds a Segment in the SegmentList associated with
 the grid point (x,y,z). 
 It is called by the rasterizer function paintFatLine()
 */

void paintCellPeriodic(const int x_inf, const int x_sup, const int y, const int z, void * arg1, void * arg2)
{
    FiberLocus const* seg = static_cast<FiberLocus const*>(arg1);
    FiberGrid::grid_type * mGrid = static_cast<FiberGrid::grid_type *>(arg2);
    //printf("paint %p in (%i to %i, %i, %i)\n", seg, x_inf, x_sup, y, z);
    
    for ( int x = x_inf; x <= x_sup; ++x )
    {
#if   ( DIM == 1 )
        mGrid->cell1D( x ).push_back(seg);
#elif ( DIM == 2 )
        mGrid->cell2D( x, y ).push_back(seg);
#elif ( DIM == 3 )
        mGrid->cell3D( x, y, z ).push_back(seg);
#endif
    }
}

//------------------------------------------------------------------------------
/**
paintGrid( first_fiber, last_fiber, max_range ) links all segments found in 'fiber' and its
 descendant, in the point-list GP that match distance(GP, segment) < H.
 
 'H' is calculated such that tryToAttach() finds any segment closer than 'max_range':
 
 To determine H, we start from a relation on the sides of a triangle:
 (A) distance( GP, segment ) < distance( GP, X ) + distance( X, segment )
 where GP (grid-point) is the closest point on the grid to X.
 
 Since GP in tryToAttach() is the closest point on mGrid to X, we have:
 (B) distance( GP, X ) < 0.5 * mGrid.diagonalLength()
 
 Thus to find all rods for which:
 (B) distance( X, segment ) < max_range
 we simply use:
 H =  max_range + 0.5 * mGrid.diagonalLength();
 
 Note: H is calculated by paintGrid(), and the user only provides 'max_range'.
 
 Linking all segments is done in an inverse way:
 for each segment, we cover all points of the grid inside a volume obtained
 by inflating the segment by the length H. We use for that the raterizer which
 calls the function paint() above.
 */

void FiberGrid::paintGrid(const Fiber * first, const Fiber * last, const real max_range)
{
    clear();
    gridRange = max_range;
    
    assert_true(hasGrid());
    
    const real* offset = mGrid.inf();
    const real* deltas = mGrid.delta();
    real width = gridRange + 0.5 * mGrid.diagonalLength();
    
    //define the painting function used:
    void (*paint)(int, int, int, int, void*, void*) = modulo ? paintCellPeriodic : paintCell;
    
    for ( const Fiber * fib = first; fib != last ; fib=fib->next() )
    {
        Vector Q, P = fib->posPoint(0);
        real S = fib->segmentation();
        
        for ( unsigned pp = 1; pp < fib->nbPoints(); ++pp )
        {
            FiberLocus * seg = &(fib->segment(pp-1));
            
            if ( pp & 1 )
                Q = fib->posPoint(pp);
            else
                P = fib->posPoint(pp);
            
#if   (DIM == 1)
            Rasterizer::paintFatLine1D(paint, seg, &mGrid, P, Q, width, offset, deltas);
#elif (DIM == 2)
            Rasterizer::paintFatLine2D(paint, seg, &mGrid, P, Q, width, offset, deltas, S);
#elif (DIM == 3)
            //Rasterizer::paintHexLine3D(paint, seg, &mGrid, P, Q, width, offset, deltas, S);
            Rasterizer::paintFatLine3D(paint, seg, &mGrid, P, Q, width, offset, deltas, S);
            //Rasterizer::paintBox3D(paint, seg, &mGrid, P, Q, width, offset, deltas);
#endif
        }
    }
}



//============================================================================
#pragma mark -

/**
 The range at which Hand will the the Fibers is limited to the range given in paintGrid()
 */
bool FiberGrid::tryToAttach(Vector const& place, Hand& ha) const
{
    assert_true( hasGrid() );
    
    if ( gridRange < ha.prop->binding_range )
    {
        printf("Warning: the FiberGrid range was exceeded:\n");
        //printf("  gridRange = %.3e < Hand::binding_range = %.3f\n", gridRange, ha.prop->binding_range);
        //throw InvalidParameter("the FiberGrid range was exceeded");
    }
    
    //get the grid node list index closest to the position in space:
    const unsigned int indx = mGrid.index(place, 0.5);
    
    //get the list of rods associated with this cell:
    SegmentList & segments = mGrid.cell(indx);
   
    //randomize the list, to make attachments more fair:
    //this might not be necessary, since the MT list is already mixed
    segments.mix(RNG);
    

    for ( SegmentList::iterator si = segments.begin(); si < segments.end(); ++si )
    {
        FiberLocus const* loc = *si;
        
        real abs, dis = INFINITY;
        // Compute the distance from the hand to the rod:
        loc->projectPoint(place, abs, dis);      // always works
        //loc->projectPointF(place, abs, dis);    // faster, but not compatible with periodic boundaries
        
        // compare to the maximum attach distance of the hand:
        if ( dis > ha.prop->binding_range_sqr )
            continue;
        
        Fiber * fib = const_cast<Fiber*>(loc->fiber());
        
        FiberBinder site(fib, fib->abscissaP(loc->point())+abs);
        
        if ( ha.attachmentAllowed(site) )
        {
            ha.attach(site);
            return true;
        }
    }
    
    return false;
}


//------------------------------------------------------------------------------
/** 
 This function is limited to the range given in paintGrid();
 */
FiberGrid::SegmentList FiberGrid::nearbySegments( Vector const& place, const real D, Fiber * exclude)
{
    if ( gridRange <= 0 )
        throw InvalidParameter("the Grid was not initialized");
    if ( gridRange < D )
    {
        printf("gridRange = %.4f < range = %.4f\n", gridRange, D);
        throw InvalidParameter("the Grid maximum distance was exceeded");
    }
    
    SegmentList res;
    
    //get the grid node list index closest to the position in space:
    const unsigned indx = mGrid.index( place, 0.5 );
    
    //get the list of rods associated with this cell:
    SegmentList & segments = mGrid.cell(indx);
    
    const real DD = D*D;
    for ( SegmentList::iterator si = segments.begin(); si < segments.end(); ++si )
    {
        FiberLocus const* loc = *si;
        
        if ( loc->fiber() == exclude ) 
            continue;
        
        real abs, dis = INFINITY;
        loc->projectPoint(place, abs, dis);
        
        if ( dis < DD )
            res.push_back(loc);
    }
    
    return res;
}


//============================================================================
//===                            ACCESSORY                                ====
//============================================================================

FiberLocus FiberGrid::closestSegment(Vector const& place)
{
    //get the cell index from the position in space:
    const unsigned indx = mGrid.index( place, 0.5 );
    
    //get the list of rods associated with this cell:
    SegmentList & segments =  mGrid.cell(indx);
    
    FiberLocus const* res = 0;
    real closest = 4 * gridRange * gridRange;
    
    for ( SegmentList::iterator si = segments.begin(); si < segments.end(); ++si )
    {
        FiberLocus const* loc = *si;
        
        //we compute the distance from the hand to the candidate rod,
        //and compare it to the best we have so far.
        real abs, dis = INFINITY;
        loc->projectPoint(place, abs, dis);
        
        if ( dis < closest )
        {
            closest = dis;
            res = loc;
        }
    }
    return *res;
}


#endif


//============================================================================
//===                       DEBUG  ATTACHMENT                             ====
//============================================================================
#pragma mark -

#include <map>
#include "simul.h"

/**
Function testAttach() is given a position in space,
 it calls tryToAttach() from this position to check that:
 - attachement has equal probability to all targets,
 - no target is missed,
 - attachment are not made to targets that are beyond binding_range_max
 */
void FiberGrid::testAttach(FILE * out, const Vector pos, Fiber * start, HandProp const* hp)
{
    //create a test motor with a dummy HandMonitor:
    HandMonitor hm;
    Hand ha(hp, &hm);
    real dsq = hp->binding_range_sqr;
    
    typedef std::map < FiberLocus const*, int > map_type;
    map_type hits;
    
    //go through all the segments to find those close enough from pos:
    for ( Fiber * fib=start; fib; fib=fib->next() )
    {
        for ( unsigned int p = 0; p < fib->nbSegments(); ++p )
        {
            FiberLocus const& loc = fib->segment(p);
            real abs, dis = INFINITY;
            loc.projectPoint(pos, abs, dis);
            
            if ( dis < dsq )
                hits[&loc] = 0;
        }
    }
    
    const size_t targets = hits.size();
    
    if ( targets == 0 )
    {
        //fprintf(out, "no target here\n");
        return;
    }
    
    //call tryTyAttach NB times to check to which rods the Hand binds:
    const size_t NB = (int)ceil( 100 * targets / hp->binding_rate_dt );
    for ( size_t n = 0; n < NB; ++n )
    {
        if ( tryToAttach(pos, ha) )
        {
            PointInterpolated inter = ha.fiber()->interpolate(ha.abscissa());
            FiberLocus const& loc = ha.fiber()->segment(inter.point1());
            
            if ( hits.find(&loc) != hits.end() )
                ++hits[&loc];
            else
                hits[&loc] = -2;
            
            ha.detach();
        }
    }
    
    //detect segments that have been missed or mistargeted:
    int verbose = 0;
    for ( map_type::const_iterator it = hits.begin(); it != hits.end(); ++it )
    {
        if ( it->second <= 50 )
            verbose = 1;
        if ( it->second < 0 )
            verbose = 2;
    }
    
    if ( verbose )
    {
        // print a summary of all targets:
        fprintf(out, "testAttach\n");
        fprintf(out, "   %lu target(s) within %.3f um\n", targets, hp->binding_range);
        fprintf(out, "   %lu trials\n", NB);
        real avg = NB * hp->binding_rate_dt / targets;
        fprintf(out, "   binding_prob = %.2f, expected_hits / target = %.3f\n", hp->binding_rate_dt, avg);

        //go through all the rods that were targeted:
        for (map_type::const_iterator it = hits.begin(); it != hits.end(); ++it )
        {
            FiberLocus const* loc = it->first;
            Fiber const* fib = loc->fiber();
            real abs, dis = INFINITY;
            loc->projectPoint(pos, abs, dis);
            
            fprintf(out, "    fib%-3lx:%-3i dist %5.3f um, abs %+.2f : ", fib->number(), loc->point(), dis, abs);
            if ( hits[loc] == 0 )
                fprintf(out, "missed\n");
            else if ( hits[loc] < 0 )
                fprintf(out, "found, although out-of-range\n");
            else if ( hits[loc] > 0 )
                fprintf(out, "%-3i hits, hits/expected = %.3f\n", hits[loc], hits[loc]/avg);
        }
    }
}

