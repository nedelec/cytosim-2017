// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/** 
 This file implements a safe 'dummy' grid using STL code, which is safe.
 For each position, it calculates the geometrical distance to all fiber segments.
 This is the slowest method with many segments, but it is simple and most likely correct!
 It is useful to evaluate the validity of other methods.
 */


#include <algorithm>

typedef std::vector <FiberLocus const*> SegmentVector;

/// a list containing all segments, as a global variable
SegmentVector allSegments;


int FiberGrid::setGrid(const Space *, const Modulo * mod, real, unsigned long) 
{
    MSG_ONCE("Cytosim is not using a grid to find attachement to Fiber!\n");
    gridRange = 0;
    return 0;
}

void FiberGrid::paintGrid(const Fiber * first, const Fiber * last, real) 
{
    allSegments.clear();
    //we go through all the segments
    for ( const Fiber * fb = first ; fb != last ; fb=fb->next() )
    {
        for ( unsigned int sg = 0; sg < fb->nbSegments(); ++sg )
            allSegments.push_back( &(fb->segment(sg)) );
    }
}

bool FiberGrid::hasGrid() const
{
    return true;
}

void FiberGrid::clear()
{
}

//------------------------------------------------------------------------------
bool FiberGrid::tryToAttach(Vector const& place, Hand& ha) const
{
    // randomize the list order
    std::random_shuffle( allSegments.begin(), allSegments.end() );

    
    // test all segments:
    
    for ( SegmentVector::iterator seg = allSegments.begin(); seg < allSegments.end(); ++seg )
    {
        FiberLocus const* loc = *seg;
        
        // Compute the distance from the hand to the segment:
        
        real abs, dis = INFINITY;
        // Compute the distance from the hand to the rod:
        loc->projectPoint(place, abs, dis);
        
        // compare to the maximum attach distance of the hand:
        if ( dis < ha.prop->binding_range_sqr )
        {
            Fiber * fib = const_cast<Fiber*>(loc->fiber());
        
            FiberBinder site(fib, fib->abscissaP(loc->point())+abs);
        
            if ( ha.attachmentAllowed(site) )
            {
                ha.attach(site);
                return true;
            }
        }
    }
    return false;
}




//======================================================================
/** 
 This function is limited to the range given in paintGrid();
 */
FiberGrid::SegmentList FiberGrid::nearbySegments( Vector const& place, const real D, Fiber * exclude )
{
    SegmentList res;
    
    const real DD = D * D;
    for ( SegmentVector::iterator seg = allSegments.begin(); seg < allSegments.end(); ++seg )
    {
        FiberLocus const* loc = *seg;
             
        if ( loc->fiber() == exclude )
            continue;
        
        real abs, dis = INFINITY;
        // Compute the distance from the hand to the rod:
        loc->projectPoint(place, abs, dist);      // always works
        
        if ( dis < DD )
            res.push_back(loc);
    }
    
    return res;
}
