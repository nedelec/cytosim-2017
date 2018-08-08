// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FIBER_GRID_H
#define FIBER_GRID_H

#include "dim.h"
#include "vector.h"
#include "array.h"
#include "grid.h"
#include <vector>

class FiberLocus;
class Space;
class Modulo;
class Fiber;
class HandProp;
class Hand;
class Node;
class Simul;


/// Divide-and-Conquer method to find all FiberLocus located near a given point in space
/**
A divide-and-conquer algorithm is used to find all segments of fibers close to a given point:
 
 -# It uses a grid 'mGrid' covering the space, initialized by setGrid().
    After initialization, each cell of the grid has an empty SegmentList (a list of FiberLocus*).
 -# clear() resets all lists on the grid
 -# paintGrid() distributes the segments specified in the arguments to the cell-associated SegmentList.
    One of the argument specifies a maximum distance to be queried (\a max_range).
    After the distribution, tryToAttach() is able to find any segment
    located at a distance \a max_range or less from any given point, in linear time.
 -# The function tryToAttach(X, ...) finds the cell on mGrid that contain \a X. 
    The associated SegmentList will then contains all the segments located at distance \a max_range or less from \a X. 
    tryToAttach() calls a function distanceSqr() sequentially for all the segments in this list,
    to calculate the exact Euclidian distance. 
    Finally, using a random number it tests the probability of attachment for the Hand given as argument.
 .
 
 @todo we could call paintGrid() only if the objects have moved by a certain threshold.
 This would work if we also extend the painted area around the rod, by the same threshold.
 we must also redo the paintGrid() when MT points are added or removed.
 
 Such algortihm should lead to large CPU gain, if calling clear() or paintGrid() is limiting,
 which is the case in particular in 3D, because the number of grid-cells is large.
*/

class FiberGrid 
{
public:
    
    /// type for a list of FiberLocus
    typedef Array<FiberLocus const*> SegmentList;
    //typedef std::vector<FiberLocus const*> SegmentList;

    typedef Grid<DIM, SegmentList, unsigned int> grid_type;
    
private:
    
    ///the maximum distance that can be found by the grid
    real  gridRange;
    
    ///grid for divide-and-conquer strategies:
    grid_type mGrid;
    
    ///the modulo object
    const Modulo * modulo;
        
public:
    
    ///creator
    FiberGrid()             { modulo = 0; gridRange = -1; }
        
    ///destructor
    virtual ~FiberGrid()    { }
    
    
    ///create a grid to cover the specified Space with cells of width \a max_step at most
    int setGrid(const Space *, const Modulo *, real max_step, unsigned long max_nb_cells);
    
    ///true if the grid was initialized by calling setGrid()
    bool hasGrid() const;
    
    ///clear the grid
    void clear();
    
    ///paint the Fibers, to be able to find up to a distance max_range
    void paintGrid(const Fiber * first, const Fiber * last, real max_range);
        
    ///given a position, find nearby Fiber segments and test attachement of the provided Hand
    bool tryToAttach(Vector const&, Hand&) const;
    
    /// return all fiber segments located at a distance D or less from P, except those belonging to \a exclude
    SegmentList nearbySegments(Vector const& P, real D, Fiber * exclude = 0);

    ///return the closest Segment to the given position, if it is closer than gridRange
    FiberLocus  closestSegment(Vector const&);
    
    ///test the results of tryToAttach(), at a particular position
    void testAttach(FILE *, Vector place, Fiber * start, HandProp const*);
    
    
#ifdef DISPLAY
    void display() const
    {
        glPushAttrib(GL_LIGHTING_BIT);
        glDisable(GL_LIGHTING);
        glColor4f(0, 1, 1, 1);
        glLineWidth(0.5);
        drawEdges(mGrid);
        glPopAttrib();
    }
#endif
    
 };


#endif
