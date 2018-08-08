// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef POINT_GRID_H
#define POINT_GRID_H

#include "dim.h"
#include "vector.h"
#include "grid.h"
#include "point_exact.h"
#include "fiber_locus.h"
#include "array.h"

class Space;
class Modulo;
class Simul;
class Meca;
class Fiber;


/// represents a PointExact for steric interactions
class FatPoint
{
    friend class PointGrid;
    
public:
    
    /// buffer for position
    Vector         pos;
    
    /// equilibrium radius of the interaction (distance where force is zero)
    real           radius;
    
    /// indicates the central Model-point
    PointExact     pe;
        
public:
    
    FatPoint() {}
    
    
    FatPoint(PointExact const& p, real rd, Vector const& w)
    {
        pe     = p;
        radius = rd;
        pos    = w;
    }    
    
    /// set from PointExact p, with radius=rd
    void set(PointExact const& p, real rd, Vector const& w)
    {
        pe     = p;
        radius = rd;
        pos    = w;
    }
};



/// represents the Segment of a Fiber for steric interactions
class FatLocus
{
    friend class PointGrid;
    
public:
    
    /// equilibrium radius of the interaction (distance where force is zero)
    real           radius;
    
    /// interaction range (maximum distance at which the force can operate)
    real           range;
    
    /// this represents the entire segment supporting 'pi'
    FiberLocus     fl;
    
public:
    
    FatLocus() {}
    
    FatLocus(FiberLocus const& p, real rd, real rg)
    {
        fl     = p;
        radius = rd;
        range  = rg;
    }
        
    /// set from FiberLocus p, with radius=rd and range=rd+erg
    void set(FiberLocus const& p, real rd, real rg)
    {
        fl     = p;
        radius = rd;
        range  = rg;
    }

    /// true if the segment is the first of the Fiber
    bool isFirst() const
    {
        return fl.isFirst();
    }
    
    /// true if the segment is the last of the Fiber
    bool isLast() const
    {
        return fl.isLast();
    }

    FatPoint point1() const
    {
        return FatPoint(fl.exact1(), radius, fl.pos1());
    }
    
    FatPoint point2() const
    {
        return FatPoint(fl.exact2(), radius, fl.pos2());
    }
};




/// type for a list of FatPoint
typedef Array<FatPoint> FatPointList;
/// type for a list of FatLocus
typedef Array<FatLocus> FatLocusList;



/// a few lists associated with the same location
class PointGridCell
{
    friend class PointGrid;
    
    /// different steric panes
    FatPointList point_pane;

    /// different steric panes
    FatLocusList locus_pane;

public:
    
    PointGridCell()
    {
    }
    
    void clear()
    {
        point_pane.clear();
        locus_pane.clear();
    }
};



/// Contains the stiffness parameters for the steric engine
class PointGridParam
{
public:
    real stiff_push;
    real stiff_pull;
    
    PointGridParam(real push, real pull)
    {
        stiff_push = push;
        stiff_pull = pull;
    }
};


/// Divide-and-Conquer to implement steric interactions
/**
 A divide-and-conquer algorithm is used to find FatPoints that overlap:
 - It uses a grid 'mGrid' covering the space, initialized by setGrid()
 To each point on mGrid is associated a list of FatPoint* of class PointGridCell.
 - The functions 'add()' position the given FatPoints on the grid
 - Function setStericInteraction() uses mGrid to find pairs of FatPoints that may overlap.
 It then calculates their actual distance, and set a interaction from Meca if necessary
 .
*/
class PointGrid
{
private:
    
    /// grid for divide-and-conquer strategies:
    Grid<DIM, PointGridCell, unsigned> mGrid;
    
    /// max radius that can be included
    real max_diameter;
    
private:
    
    /// check two Spheres
    void checkPP(Meca&, PointGridParam const& pam, FatPoint const&, FatPoint const&) const;
    
    /// check Sphere against Line segment
    void checkPL(Meca&, PointGridParam const& pam, FatPoint const&, FatLocus const&) const;
    
    /// check Line segment against Sphere
    void checkLL1(Meca&, PointGridParam const& pam, FatLocus const&, FatLocus const&) const;
    
    /// check Line segment against Sphere
    void checkLL2(Meca&, PointGridParam const& pam, FatLocus const&, FatLocus const&) const;
    
    /// check two Line segments
    void checkLL(Meca&, PointGridParam const& pam, FatLocus const&, FatLocus const&) const;

    
    /// cell corresponding to position `w`
    FatPointList& point_list(Vector const& w) const
    {
        return mGrid.cell(w).point_pane;
    }

    /// cell corresponding to position `w`
    FatLocusList& locus_list(Vector const& w) const
    {
        return mGrid.cell(w).locus_pane;
    }

    /// cell corresponding to index `w`
    FatPointList& point_list(const unsigned w) const
    {
        return mGrid.cell(w).point_pane;
    }
    
    /// cell corresponding to index `w`
    FatLocusList& locus_list(const unsigned w) const
    {
        return mGrid.cell(w).locus_pane;
    }
    
public:
    
    /// creator
    PointGrid();
    
    /// destructor
    virtual ~PointGrid()    { }
    
    /// create a grid to cover the specified Space, with cell of size min_step at least
    void setGrid(Space const*, Modulo const*, real min_step);
    
    /// true if the grid was initialized by calling setGrid()
    bool hasGrid() const    { return mGrid.hasCells(); }
    
    /// clear the grid
    void clear()            { mGrid.clear(); }
    
    /// place PointExact on the grid
    void add(PointExact const& p, real radius) const;
    
    /// place FiberLocus on the grid
    void add(FiberLocus const& p, real radius, real extra_range) const;
    
    /// enter interactions into Meca between two panes with given stiffness
    void setInteractions(Meca&, PointGridParam const& pam) const;

#ifdef DISPLAY
    void display() const
    {
        glPushAttrib(GL_LIGHTING_BIT);
        glDisable(GL_LIGHTING);
        glColor4f(1, 0, 1, 1);
        glLineWidth(0.5);
        drawEdges(mGrid);
        glPopAttrib();
    }
#endif
};


#endif
