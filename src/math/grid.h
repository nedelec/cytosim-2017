// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Francois Nedelec; Last updated Jan 2008. nedelec@embl.de

#ifndef GRID_H
#define GRID_H

#include "assert_macro.h"
#include "exceptions.h"
#include <cstdio>
#include <cmath>
#include "real.h"

#ifdef DISPLAY
#  include "opengl.h"
#endif

///\def flag to disable support for periodic boundaries conditions
/**
Periodic boundary conditions are supported using a function-pointer.
 If keyword NO_PERIODIC_SUPPORT is defined however, inlined functions are 
 used instead, which might be faster, but Periodic boundary are then not supported.
 */
//#define NO_PERIODIC_SUPPORT


///Divide a rectangle of dimensionality ORD into regular voxels
/** 
Grid<int ORD, typename CELL, typename INDEX> creates a regular lattice over a rectangular
region of space of dimensionality ORD.
The grid is initialized by setDimensions() and createCells() allocates a one-dimensional
array of CELL, with one value for each lattice point of the grid.

Functions are provided to convert from the real space coordinates (of type real)
into an index (of type INDEX) usable to access the one-dimensional array of CELL.
The cells are ordered successively, the first dimension (X) varying the fastest
i.e. cell[ii+1] will in most cases be located on the right of cell[ii], although
if cell[ii] is on the right edge, then cell[ii+1] is on the symmetric edge. 

\par Access:

Cells can be accessed in three ways:
 - Position:      a set of real       operator()( real[] ), or operator(real, real, real)
 - Index:         one integer         operator[](int index)
 - Coordinates:   a set of integer    function cell(int[]), or cell(int,int,int)
.
Valid indices are [0...nbCells()-1], where nbCells() is calculated by setDimensions().
If a position lies outside the rectangular region where the grid is defined,
index(real[]) returns the index of the closest voxel.

Functions to convert between the three types are provided:
 - index()
 - indexFromCoordinates()
 - setCoordinatesFromIndex(),
 - setCoordinatesFromPosition()
 - setPositionFromCoordinates()
 - setPositionFromIndex()
.

\par Indices:

The grid is initialized by setDimensions(inf, sup, nbCells), which calculates:
  - cWidth[d] = ( sup[d] - inf[d] ) / nbCells[d], for d in [0, ORD[

The coordinates of a cell at position pos[] are:
  - c[d] = int(  ( pos[d] - inf[d] ) / cWidth[d] )

and its index is
  - with ORD==1: index = c[0]
  - with ORD==2: index = c[0] + nbcells[0] * c[1]
  - with ORD==3: index = c[0] + nbcells[0] * ( c[1] + nbcells[1] * c[2] )
  - etc.
.
    
For a 4x4 2D grid, the index are like this:
@code
12  13  14  15
8    9  10  11
4    5   6   7
0    1   2   3
@endcode

\par Neighborhood:

The class also provides information on which cells surround each cell:
 - createSquareRegions(range) calculates square regions of size range
   ( range==1 gives nearest neighbors ).
 - createRoundRegions(range) calculates round regions of size range
 - createSideRegions(range)
.
After calling one of the above function, getRegion(offsets, index) will set 'offsets'
to point to an array of 'index offsets' for the cell referred by 'index'.
A zero offset value (0) is always first in the list and refers to self.
In the example above:
    - for index = 0 it would return { 0 1 4 5 }
    - for index = 5 it would return { 0 -1 1 -5 -4 -3 3 4 5 }
.
You obtain the cell-indices of the neighboring cells by adding offsets[n] to 'index':
Example:
@code
    CELL * cell = & mGrid.cell(indx);
    nb_neighbors = mGrid.getRegion(region, indx);
    for ( int n = 1; n < nb_neighbors; ++n ) 
    {
        Cell & neighbor = cell[region[n]];
        ...
    }
@endcode
*/

///\todo Derive GridNumeric specialized for numerical values
///\todo add Grid<> copy constructor and copy assignment

template <int ORD, typename CELL, typename INDEX> 
class Grid
{
    
#ifdef NO_PERIODIC_SUPPORT
    
    /// closest integer to c in the segment [ 0, s-1 ]
    inline INDEX imageI(const int& s, const int& c) const
    {
        return c <= 0 ? 0 : ( c >= s ? s-1 : c );
    }

    /// closest integer to c in the segment [ 0, s-1 ]
    inline INDEX imageF(const int& s, const real& c) const
    {
        return c <= 0 ? 0 : ( c >= s ? s-1 : (int)c );
    }

#else
    
    /// closest integer to c in the segment [ 0, s-1 ]
    static INDEX imageIB(const int& s, int c)
    {
        return c <= 0 ? 0 : ( c >= s ? s-1 : c );
    }

    /// c modulo s in [ 0, s [
    static INDEX imageIP(const int& s, int c)
    {
        while ( c >= s )  c -= s;
        while ( c <  0 )  c += s;
        return c;
    }

    /// closest integer to c in the segment [ 0, s-1 ]
    static INDEX imageFB(const int& s, real c)
    {
        return c <= 0 ? 0 : ( c >= s ? s-1 : (int)c );
    }
    
    /// c modulo s in [ 0, s [
    static INDEX imageFP(const int& s, real c)
    {
        while ( c >= s )  c -= s;
        while ( c <  0 )  c += s;
        return (int)c;
    }

    /// pointer to image_bounded() or image_periodic()
    INDEX (*imageI)(const int&, int);
    INDEX (*imageF)(const int&, real);
    
#endif

public:
    
    /// The type of cells (=CELL)
    typedef CELL cell_type;
    
    /// the type for indices (=INDEX)
    typedef INDEX index_type;

private:

    /// Disabled copy constructor
    Grid<ORD, CELL, INDEX>(Grid<ORD, CELL, INDEX> const&);
    
    /// Disabled copy assignment
    Grid<ORD, CELL, INDEX>& operator=(Grid<ORD, CELL, INDEX> const&);
    
    /// allocated size of array cells[]
    INDEX   allocated;
    
protected:
    
    /// The array of pointers to cells
    CELL *  gCell;
    
    /// The number of cells in the map; size of cells[]
    INDEX   nCells;
    
    /// The number of cells in each dimension of the real space
    INDEX   gDim[ORD];
    
    /// The position of the inferior edge (min) in each dimension
    real    gInf[ORD];
    
    /// The position of the superior edge (max) in each dimension
    real    gSup[ORD];
    
    /// cWidth[d] = ( gSup[d] - inf[d] ) / gDim[d]
    real    cWidth[ORD];
    
    /// cDelta[d] = 1.0 / cWidth[d]
    real    cDelta[ORD];
    
    /// The volume occupied by one cell
    real    cVolume;

    
//--------------------------------------------------------------------------
#pragma mark -
public:
    
    /// constructor
    Grid() : gDim(), gInf(), gSup(), cWidth(), cDelta()
    {
        allocated   = 0;
        nCells      = 0;
        gCell       = 0;
        regionsEdge = 0;
        regions     = 0;
        cVolume     = 0;
#ifndef NO_PERIODIC_SUPPORT
        imageI      = imageIB;
        imageF      = imageFB;
#endif
    }
    
    /// Free memory
    void destroy()
    {
        deleteCells();
        deleteRegions();
    }
    
    /// Destructor
    virtual ~Grid() { destroy(); }
    
    
#ifdef NO_PERIODIC_SUPPORT
    
    /// true if boundary conditions are periodic
    bool periodic() { return false; }
    
    /// change boundary conditions
    void periodic(const bool p)
    {
        if ( p )
        {
            throw InvalidParameter("grid.h was compiled with NO_PERIODIC_SUPPORT");
        }
    }

#else
    
    /// true if boundary conditions are periodic
    bool periodic() { return imageI == imageIP; }
    
    /// change boundary conditions
    void periodic(const bool p)
    {
        if ( p )
        {
            imageI = imageIP;
            imageF = imageFP;
        }
        else
        {
            imageI = imageIB;
            imageF = imageFB;
        }
    }
    
#endif
    
    //--------------------------------------------------------------------------
    /// set the sizes of the real space dimensions
    void setDimensions(const real infs[ORD],
                       real sups[ORD],
                       const int nbcells[ORD])
    {
        nCells = 1;
        cVolume = 1;
        
        for ( int d = 0; d < ORD; ++d )
        {
            if ( nbcells[d] <= 0 )
                throw InvalidParameter("Grid::nbcells[] is <= 0");
            
            if ( infs[d] > sups[d] )
            {
                if ( infs[d] > sups[d] + REAL_EPSILON )
                    throw InvalidParameter("Grid::sup[] < inf[]");
                sups[d] = infs[d];
            }
            
            nCells   *= nbcells[d];
            gDim[d]   = nbcells[d];
            gInf[d]    = infs[d];
            gSup[d]    = sups[d];
            cWidth[d]  = ( gSup[d] - gInf[d] ) / real( gDim[d] );
            cDelta[d]  = real( gDim[d] ) / ( gSup[d] - gInf[d] );
            cVolume   *= cWidth[d];
        }
    }
    
    ///true if setDimensions() was called
    bool hasDimensions() { return nCells > 0; }
    
    //--------------------------------------------------------------------------
#pragma mark -

    /// total number of cells in the map
    INDEX           nbCells()           const { return nCells; }
    INDEX           dim( )              const { return nCells; }

    /// number of cells in dimensionality d
    INDEX           nbCells(int d)      const { return gDim[d]; }
    INDEX           dim(int d)          const { return gDim[d]; }
    
    /// position of the inferior (left/bottom/etc) edge
    const real*     inf()               const { return gInf;    }
    real            inf(int d)          const { return gInf[d]; }
    
    /// position of the superior (right/top/etc) edge
    const real*     sup()               const { return gSup;    }
    real            sup(int d)          const { return gSup[d]; }
    
    /// the widths of a cell
    const real*     delta()             const { return cDelta;    }
    real            delta(int d)        const { return cDelta[d]; }
    
    const real*     cellWidth()         const { return cWidth;    }
    real            cellWidth(int d)    const { return cWidth[d]; }

    real            position(int d, real c) const { return gInf[d] + c * cWidth[d]; }
    
    /// the volume of a cell
    real            cellVolume()        const { return cVolume; }

    /// the length of the diagonal of a cell = sqrt( sum(cWidth[d]^2) )
    real diagonalLength() const
    {
        real res = cWidth[0] * cWidth[0];
        for ( int d = 1; d < ORD; ++d )
            res += cWidth[d] * cWidth[d];
        return sqrt( res );
    }
    
    /// the smallest cell width, along dimensions that have more than \a min_size cells
    real minimumWidth(unsigned int min_size) const
    {
        real res = 0;
        for ( int d = 0; d < ORD; ++d )
            if ( gDim[d] > min_size )
                res = cWidth[d];
        for ( int d = 0; d < ORD; ++d )
            if ( gDim[d] > min_size  &&  cWidth[d] < res )
                res = cWidth[d];
        return res;
    }
    
    //--------------------------------------------------------------------------
#pragma mark - Conversion

    /// checks if coordinates are inside the box
    bool inside(const int coord[ORD]) const
    {
        for ( int d = 0; d < ORD; ++d )
        {
            if ( coord[d] < 0 || (index_type)coord[d] >= gDim[d] )
                return false;
        }
        return true;
    }
    
    /// checks if point is inside the box
    bool inside(const real w[ORD]) const
    {
        for ( int d = 0; d < ORD; ++d )
        {
            if ( w[d] < gInf[d] || w[d] >= gSup[d] )
                return false;
        }
        return true;
    }
    
    /// periodic image
    void bringInside(int coord[ORD]) const
    {
        for ( int d = 0; d < ORD; ++d )
            coord[d] = imageI(gDim[d], coord[d]);
    }
    
    /// conversion from index to coordinates
    void setCoordinatesFromIndex(int coord[ORD], INDEX indx) const
    {
        for ( int d = 0; d < ORD; ++d )
        {
            coord[d] = indx % gDim[d];
            indx    /= gDim[d];
        }
    }
    
    /// conversion from Position to coordinates (offset should be in [0,1])
    void setCoordinatesFromPosition(int coord[ORD], const real w[ORD], const real offset=0) const
    {
        for ( int d = 0; d < ORD; ++d )
            coord[d] = imageF(gDim[d], offset+(w[d]-gInf[d])*cDelta[d]);
    }
    
    /// conversion from Index to Position (offset should be in [0,1])
    void setPositionFromIndex(real w[ORD], INDEX indx, real offset=0) const
    {
        for ( int d = 0; d < ORD; ++d )
        {
            w[d] = gInf[d] + cWidth[d] * ( offset + indx % gDim[d] );
            indx /= gDim[d];
        }
    }
    
    /// conversion from Coordinates to Position (offset should be in [0,1])
    void setPositionFromCoordinates(real w[ORD], const int coord[ORD], real offset=0) const
    {
        for ( int d = 0; d < ORD; ++d )
            w[d] = gInf[d] + cWidth[d] * ( offset + coord[d] );
    }
    
    /// conversion from coordinates to index
    INDEX indexFromCoordinates(const int coord[ORD]) const
    {
        INDEX inx = imageI(gDim[ORD-1], coord[ORD-1]);
        
        for ( int d = ORD-2; d >= 0; --d )
            inx = gDim[d] * inx + imageI(gDim[d], coord[d]);
        
        return inx;
    }
        
    
    /// returns the index of the cell whose center is closest to the point w[]
    INDEX index(const real w[ORD], const real offset=0) const
    {
        int d = ORD-1;
        INDEX inx = imageF(gDim[d], offset+(w[d]-gInf[d])*cDelta[d]);
        
        for ( d = ORD-2; d >= 0; --d )
            inx = gDim[d] * inx + imageF(gDim[d], offset+(w[d]-gInf[d])*cDelta[d]);
        
        return inx;
    }

    //--------------------------------------------------------------------------
#pragma mark -
#pragma mark Cells

    /// allocate the array of cells
    void createCells()
    {
        if ( nCells == 0 )
            printf("nCells==0 in createCells() : call setDimensions() first\n");
        
        if ( gCell )
            delete[] gCell;
                
        gCell = new CELL[nCells];
        allocated = nCells;
    }
    
    /// returns true if cells have been allocated
    bool hasCells() const
    {
        return ( gCell != 0 );
    }
    
    /// deallocate array of cells
    void deleteCells()
    {
        if ( gCell )
            delete[] gCell;
        gCell = 0;
        allocated = 0;
    }
    
    /// call function clear() for all cells
    void clear()
    {
        if ( gCell == 0 )
            return;
        
        CELL * c = gCell;
        const CELL * last = gCell + nCells;
#if ( 0 )
        for ( ; c < last; ++c )
            c->clear();
#else
        //we unroll the loop to go faster
        const CELL * stop = gCell + (nCells % 8);
        for ( ; c < stop; ++c )
            c->clear();
        for ( ; c < last ; c += 8 )
        {
            c[0].clear();
            c[1].clear();
            c[2].clear();
            c[3].clear();
            c[4].clear();
            c[5].clear();
            c[6].clear();
            c[7].clear();
        }
#endif
    }

    //--------------------------------------------------------------------------

    /// address of the cell array ( equivalent to &cell(0) )
    CELL * cell_addr()
    {
        return gCell;
    }
    
    /// return cell at index 'indx'
    CELL & cell(const INDEX indx) const
    {
        assert_true( gCell );
        assert_true( indx < nCells && indx < allocated );
        return gCell[ indx ];
    }
    
    /// reference to CELL whose center is closest to w[]
    CELL & cell(const real w[ORD]) const
    {
        assert_true( gCell );
        assert_true( index(w) < nCells );
        return gCell[ index(w) ];
    }
    
    /// reference to CELL of coordinates c[]
    CELL & cell(const int c[ORD]) const
    {
        assert_true( gCell );
        assert_true( indexFromCoordinates(c) < nCells );
        return gCell[ indexFromCoordinates(c) ];
    }
   
    /// operator access to a cell by index
    CELL & operator[](const INDEX indx) const
    {
        assert_true( gCell );
        assert_true( indx < nCells );
        return gCell[ indx ];
    }
    
    /// operator access to a cell by position
    CELL & operator()(const real w[ORD]) const
    {
        assert_true( gCell );
        assert_true( index(w) < nCells );
        return gCell[ index(w) ];
    }

    /// short-hand access to a cell by co-oordinates
    CELL & operator()(const int c[ORD]) const
    {
        assert_true( gCell );
        assert_true( indexFromCoordinates(c) < nCells );
        return gCell[ indexFromCoordinates(c) ];
    }
        
    //--------------------------------------------------------------
#pragma mark -
    
    /// create a 1D-map
    void create1D(real min, real max, int nbcells)
    {
        assert_true( ORD == 1 );
        setDimensions( &min, &max, &nbcells );
        createCells();
    }

    /// access to cell for ORD==1
    CELL & cell1D(const int x) const
    {
        assert_true( ORD == 1  &&  gCell );
        INDEX inx = imageI(gDim[0], x);
        assert_true( inx < nCells );
        return gCell[ inx ];
    }
    
    /// access to cell for ORD==2
    CELL & cell2D(const int x, const int y) const
    {
        assert_true( ORD == 2  &&  gCell );
        INDEX inx = imageI(gDim[0], x) + gDim[0]*imageI(gDim[1], y);
        assert_true( inx < nCells );
        return gCell[ inx ];
    }
    
    /// access to cell for ORD==3
    CELL & cell3D(const int x, const int y, const int z) const
    {
        assert_true( ORD == 3  &&  gCell );
        INDEX inx = imageI(gDim[0], x) + gDim[0]*( imageI(gDim[1], y) + gDim[1]*imageI(gDim[2], z) );
        assert_true( inx < nCells );
        return gCell[ inx ];
    }

    //==========================================================================
#pragma mark -
#pragma mark Regions

    /** For any cell, we can find the adjacent cells by adding 'index offsets'
    However, the valid offsets depends on wether the cell is on a border or not.
    For each 'edge', a list of offsets and its gDim are stored.*/
    
private:
    
    /// array of index offset to neighbors, for each edge type
    int  * regionsEdge;
    
    /// pointers to regionsEdge[], as a function of cell index
    int ** regions;
    
private:
    
    /// calculate the edge-characteristic from the size \a s, coordinate \a c and range \a r
    static INDEX edge_value(const int s, const int r, const int c)
    {
        if ( c < r )
            return r - c;
        else if ( c + r + 1 > s )
            return 2 * r + c - s + 1;
        else
            return 0;
    }
    
    /// caculate the edge characteristic from the coordinates of a cell \a coord and range
    INDEX edgeFromCoordinates(const int coord[ORD], const int range[ORD]) const
    {
        INDEX e = 0;
        for ( int d = ORD-1; d >= 0; --d )
        {
            e *= 2 * range[d] + 1;
            e += edge_value(gDim[d], range[d], coord[d]);
        }
        return e;
    }
    
    
    int * makeRectangularGrid(int& cmx, const int range[ORD])
    {
        cmx = 1;
        for ( int d = 0; d < ORD; ++d )
            cmx *= ( 2 * range[d] + 1 );
        int * ccc = new int[ORD*cmx];
        
        int nb = 1;
        for ( int d = 0; d < ORD; ++d )
        {
            int h = 0;
            for ( ; h < nb && h < cmx; ++h )
                ccc[ORD*h+d] = 0;
            for ( int s = -range[d]; s <= range[d]; ++s )
            {
                if ( s != 0 )
                {
                    for ( int n = 0; n < nb && h < cmx; ++n, ++h )
                    {
                        for ( int e = 0; e < d; ++e )
                            ccc[ORD*h+e] = ccc[ORD*n+e];
                        ccc[ORD*h+d] = s;
                    }
                }
            }
            nb = h;
        }
        assert_true(nb==cmx);
        return ccc;
    }
    
    
    /// calculate cell index offsets between 'ori' and 'ori+shift'
    int calculateOffsets(int offsets[], int shift[], int cnt, int ori[], bool positive)
    {
        int nb = 0;
        int cc[ORD];
        int ori_indx = (int)indexFromCoordinates(ori);
        for ( int ii = 0; ii < cnt; ++ii )
        {
            for ( int d = 0; d < ORD; ++d )
                cc[d] = ori[d] + shift[ORD*ii+d];
            int off = (int)indexFromCoordinates(cc) - ori_indx;
            
            bool add = ( positive ? off >= 0 : true );
            if ( periodic() )
            {
                //check that cell is not already included:
                for ( int n = 0; n < nb; ++n )
                    if ( offsets[n] == off )
                    {
                        add = false;
                        break;
                    }
            }
            else 
                add &= inside(cc);
            
            if ( add )
                offsets[nb++] = off;
        }
        return nb;
    }
    
   
    /// create regions in the offsets buffer
    /**
     Note: the range is taken specified in units of cells: 1 = 1 cell
     @todo: specify range in calculateRegion() as real distance!
     */
    void createRegions(int * ccc, const int regMax, const int range[ORD], bool positive)
    {
        INDEX edgeMax = 0;
        for ( int d = ORD-1; d >= 0; --d )
        {
            edgeMax *= 2 * range[d] + 1;
            edgeMax += 2 * range[d];
        }
        ++edgeMax;
        
        //allocate and reset arrays:
        deleteRegions();
        
        regions     = new int*[nCells];
        regionsEdge = new int[edgeMax*(regMax+1)];
        for ( INDEX e = 0; e < edgeMax*(regMax+1); ++e )
            regionsEdge[e] = 0;
        
        int ori[ORD];
        for ( INDEX indx = 0; indx < nCells; ++indx )
        {
            setCoordinatesFromIndex(ori, indx);
            INDEX e = edgeFromCoordinates(ori, range);
            assert_true( e < edgeMax );
            int * reg = regionsEdge + e * ( regMax + 1 );
            if ( reg[0] == 0 )
            {
                // calculate the region for this new edge-characteristic
                reg[0] = calculateOffsets(reg+1, ccc, regMax, ori, positive);
                //printf("edge %i has region of %i cells\n", e, reg[0]);
            }
            else if ( 0 )
            {
                // compare result for a different cell of the same edge-characteristic
                int * rig = new int[regMax+1];
                rig[0] = calculateOffsets(rig+1, ccc, regMax, ori, positive);
                if ( rig[0] != reg[0] )
                    ABORT_NOW("inconsistent region size");
                for ( int s = 1; s < rig[0]+1; ++s )
                    if ( rig[s] != reg[s] )
                        ABORT_NOW("inconsistent region offsets");
                delete [] rig;
            }
            regions[indx] = reg;
        }
    }
    
    /// accept within a certain diameter
    bool reject_disc(const int c[ORD], real radius)
    {
        real dsq = 0;
        for ( int d = 0; d < ORD; ++d ) 
            dsq += cWidth[d] * c[d] * cWidth[d] * c[d];
        return ( dsq > radius * radius );
    }
    
    /// accept within a certain diameter
    bool reject_square(const int c[ORD], real radius)
    {
        for ( int d = 0; d < ORD; ++d ) 
            if ( fabs( cWidth[d] * c[d] ) > radius )
                return true;
        return false;
    }
    
    /// used to remove ORD coordinates in array \a ccc
    void remove_entry(int * ccc, int& cmx, const int s)
    {
        --cmx;
        for ( int x = ORD*s; x < ORD*cmx; ++x )
            ccc[x] = ccc[x+ORD];
    }        
    
public:
    
    /// create regions which contains cells at a distance 'range' or less
    /**
     Note: the range is specified in real units.
     the region will cover an area of space that is approximately square.
     */
    void createSquareRegions(const real radius)
    {
        int range[ORD];
        for ( int d = 0; d < ORD; ++d )
            range[d] = ceil( radius / cWidth[d] );
        int cmx = 0;
        int * ccc = makeRectangularGrid(cmx, range);
        
        for ( int s = cmx-1; s >= 0 ; --s )
            if ( reject_square(ccc+ORD*s, radius) )
                remove_entry(ccc, cmx, s);
        
        createRegions(ccc, cmx, range, false);
        delete [] ccc;
    }
    
    /// create regions which contains cells at a distance 'range' or less
    /**
     Note: the range is specified in real units.
     The region covers an area of space that is approximately circular.
     */
    void createRoundRegions(const real radius)
    {
        int range[ORD];
        for ( int d = 0; d < ORD; ++d )
        {
            assert_true( cWidth[d] > 0 );
            range[d] = ceil( radius / cWidth[d] );
        }
        int cmx = 0;
        int * ccc = makeRectangularGrid(cmx, range);
       
        for ( int s = cmx-1; s >= 0 ; --s )
            if ( reject_disc(ccc+ORD*s, radius) )
                remove_entry(ccc, cmx, s);
        
        createRegions(ccc, cmx, range, false);
        delete [] ccc;
    }

    /// regions that only contain cells of greater index.
    /**
     This is suitable for pair-wise interaction of particles, since it can
     be used to go through the cells one by one such that at the end, all
     pairs of cells have been considered only once.

     Note: the range is taken specified in units of cells: 1 = 1 cell
     */
    void createSideRegions(const int radius)
    {
        int range[ORD];
        for ( int d = 0; d < ORD; ++d )
            range[d] = radius;
        int cmx = 0;
        int * ccc = makeRectangularGrid(cmx, range);
        createRegions(ccc, cmx, range, true);
        delete [] ccc;
    }
    
    /// true is createRegions() or createRoundRegions() was called
    bool hasRegions() const
    {
        return ( regions != 0  &&  regionsEdge != 0 );
    }
    
    /// set region array 'offsets' for cell index
    /**
     A zero offset is always first in the list.
     //\returns the size of the list.
     
     \par Example:
     @code
     CELL * cell = & mGrid.cell(indx);
     nb_neighbors = mGrid.getRegion(region, indx);
     for ( int n = 1; n < nb_neighbors; ++n ) 
     {
         Cell & neighbor = cell[region[n]];
         ...
     }
     @endcode
     
     Note: you must call createRegions() first
    */
    int getRegion(int*& offsets, const INDEX indx) const
    {
        assert_true( hasRegions() );
        offsets = regions[indx]+1;
        assert_true( offsets[0] == 0 );
        return regions[indx][0];
    }
    
    /// free memory occupied by the regions
    void deleteRegions()
    {
        if ( regions )
            delete [] regions;
        regions = 0;
        
        if ( regionsEdge )
            delete [] regionsEdge;
        regionsEdge = 0;
    }

    
    //-----------------------------------------------------------------------
#pragma mark -
#pragma mark Interpolate
    
    /// a fast floor function
    inline static int ffloor(real x) { return ( x < 0 ) ? (int)x-1 : (int)x; }
    
    /// return linear interpolation of values stored at the center of each cell
    /**
     
     */
    CELL interpolate( const real w[ORD] ) const
    {
        //we have 2^ORD corner cells
        const int sz = 1 << ORD;
        INDEX inx[sz];   //incides of the corner cells
        real  alp[sz];   //coefficients of interpolation
        
        int nb = 0;
        for ( int d = ORD-1; d >= 0; --d )
        {
            real a = ( w[d] - gInf[d] ) * cDelta[d] + 0.5;
            int ia = ffloor(a);
            a     -= ia;
            int  l = imageI(gDim[d], ia-1);
            int  u = imageI(gDim[d], ia  );
            
            if ( nb == 0 )
            {
                //initialize the edges ( l and u ) and appropriate coefficients
                inx[1] = u;  alp[1] = a;
                inx[0] = l;  alp[0] = 1-a;
                nb = 2;
            }
            else
            {
                //double the amount of edges at each round,
                //with the indices and coefficients for lower and upper bounds
                for ( int c = 0; c < nb; ++c )
                {
                    inx[c+nb] = gDim[d] * inx[c] + u;  alp[c+nb] = alp[c] * a;
                    inx[c   ] = gDim[d] * inx[c] + l;  alp[c   ] = alp[c] * (1-a);
                }
                nb *= 2;
            }
        }
        assert_true( nb == sz );
        
        //sum weighted cells to get interpolation
        real res = 0;
        for ( int c = 0; c < sz; ++c ) 
            res += alp[c] * gCell[inx[c]];
        return res;
    }
    
    
    /// return linear interpolation of values stored at the center of each cell, but only if ORD==2
    CELL interpolate2D( const real w[ORD] ) const
    {
        assert_true( ORD == 2 );
        
        real  ax = ( w[0] - gInf[0] ) * cDelta[0] + 0.5;
        real  ay = ( w[1] - gInf[1] ) * cDelta[1] + 0.5;
        
#ifdef NO_PERIODIC_SUPPORT
        int   ix = (int)ax;
        int   iy = (int)ay;
#else
        int   ix = ffloor(ax);
        int   iy = ffloor(ay);
#endif
        
        ax -= ix;
        ay -= iy;
        
        INDEX lx = imageI(gDim[0], ix-1);
        INDEX ux = imageI(gDim[0], ix  );
        
        INDEX ly = imageI(gDim[1], iy-1) * gDim[0];
        INDEX uy = imageI(gDim[1], iy  ) * gDim[0];
        
        //sum weighted cells to get interpolation
        real  rl = (1-ay) * gCell[lx+ly] + ay * gCell[lx+uy];
        real  ru = (1-ay) * gCell[ux+ly] + ay * gCell[ux+uy];

        return rl + ax * ( ru - rl );
    }

    
    /// return linear interpolation of values stored at the center of each cell, but only if ORD==3
    CELL interpolate3D( const real w[ORD] ) const
    {
        assert_true( ORD == 3 );
        
        real  ax = ( w[0] - gInf[0] ) * cDelta[0] + 0.5;
        real  ay = ( w[1] - gInf[1] ) * cDelta[1] + 0.5;
        real  az = ( w[2] - gInf[2] ) * cDelta[2] + 0.5;
        
#ifdef NO_PERIODIC_SUPPORT
        int   ix = (int)ax;
        int   iy = (int)ay;
        int   iz = (int)az;
#else
        int   ix = ffloor(ax);
        int   iy = ffloor(ay);
        int   iz = ffloor(az);
#endif
        
        ax -= ix;
        ay -= iy;
        az -= iz;

        INDEX lx = imageI(gDim[0], ix-1);
        INDEX ux = imageI(gDim[0], ix  );
        
        INDEX ly = imageI(gDim[1], iy-1) * gDim[0];
        INDEX uy = imageI(gDim[1], iy  ) * gDim[0];
        
        INDEX lz = imageI(gDim[2], iz-1) * gDim[0] * gDim[1];
        INDEX uz = imageI(gDim[2], iz  ) * gDim[0] * gDim[1];

        real * cuu = gCell+uy+uz, ruu = (1-ax) * cuu[lx] + ax * cuu[ux];
        real * cul = gCell+uy+lz, rul = (1-ax) * cul[lx] + ax * cul[ux];
        real * clu = gCell+ly+uz, rlu = (1-ax) * clu[lx] + ax * clu[ux];
        real * cll = gCell+ly+lz, rll = (1-ax) * cll[lx] + ax * cll[ux];

        return (1-ay) * ( rll + az * (rlu-rll) ) + ay * ( rul + az * (ruu-rul) );
    }


    //--------------------------------------------------------------------------
#pragma mark -
#pragma mark For Numerical Cells

    /// set all cells to zero, if CELL supports this operation
    void setValues(const CELL val)
    {
        assert_true( nCells <= allocated );
        for ( INDEX ii = 0; ii < nCells; ++ii )
            gCell[ii] = val;
    }
    
    /// set all cells to zero, if CELL supports this operation
    void scaleValues(const CELL val)
    {
        assert_true ( nCells <= allocated );
        for ( INDEX ii = 0; ii < nCells; ++ii )
            gCell[ii] *= val;
    }
    
    /// sum of all values, if CELL supports the addition
    CELL sumValues() const
    {
        assert_true( nCells <= allocated );
        CELL result = 0;
        for ( INDEX ii = 0; ii < nCells; ++ii )
            result += gCell[ii];
        return result;
    }
    
    /// maximum value over all cells
    CELL maxValue() const
    {
        assert_true( nCells <= allocated );
        CELL res = gCell[0];
        for ( INDEX ii = 1; ii < nCells; ++ii )
        {
            if ( res < gCell[ii] )
                res = gCell[ii];
        }
        return res;
    }
    
    /// minimum value over all cells
    CELL minValue() const
    {
        assert_true( nCells <= allocated );
        CELL res = gCell[0];
        for ( INDEX ii = 1; ii < nCells; ++ii )
        {
            if ( res > gCell[ii] )
                res = gCell[ii];
        }
        return res;
    }
    
    /// true if any( cells[] < 0 )
    bool hasNegativeValue() const
    {
        assert_true( nCells <= allocated );
        for ( INDEX ii = 0; ii < nCells; ++ii )
            if ( gCell[ii] < 0 )
                return true;
        return false;
    }
    
#pragma mark -
    
    /// the sum of the values in the region around cell referred by 'indx'
    CELL sumValuesInRegion(const INDEX indx) const
    {
        CELL result = 0;
        int * offsets = 0;
        const CELL * ce = gCell + indx;
        int nb = getRegion(offsets, indx);
        for ( int c = 0; c < nb; ++c )
            result += ce[ offsets[c] ];
        return result;
    }
    
    /// the sum of the values in the region around cell referred by 'indx'
    CELL avgValueInRegion(const INDEX indx) const
    {
        CELL result = 0;
        int * offsets = 0;
        const CELL * ce = gCell + indx;
        int nb = getRegion(offsets, indx);
        for ( int c = 0; c < nb; ++c )
            result += ce[ offsets[c] ];
        return result / (real)nb;
    }
    
    /// the maximum of the values in the region around cell referred by 'indx'
    CELL maxValueInRegion(const INDEX indx) const
    {
        assert_true( nCells <= allocated );
        CELL result = gCell[indx];
        int * offsets = 0;
        const CELL * ce = gCell + indx;
        int nb = getRegion(offsets, indx);
        for ( int c = 0; c < nb; ++c )
            if ( result < ce[ offsets[c] ] )
                result = ce[ offsets[c] ];
        return result;
    }
    
    /// write values to a file, with the position for each cell (file can be stdout)
    void printValues(FILE* file, const real offset) const
    {
        assert_true( nCells <= allocated );
        real w[ORD];
        for ( INDEX ii = 0; ii < nCells; ++ii )
        {
            setPositionFromIndex(w, ii, offset );
            for ( int d=0; d < ORD; ++d )
                fprintf(file, "%7.2f ", w[d]);
            fprintf(file,"  %f\n", gCell[ii]);
        }
    }
    
    /// write values to a file, with the range for each cell (file can be stdout)
    void printValuesWithRange(FILE* file) const
    {
        assert_true( nCells <= allocated );
        real l[ORD], r[ORD];
        for ( INDEX ii = 0; ii < nCells; ++ii )
        {
            setPositionFromIndex(l, ii, 0.0);
            setPositionFromIndex(r, ii, 1.0);
            for ( int d=0; d < ORD; ++d )
                fprintf(file, "%7.2f %7.2f  ", l[d], r[d]);
            fprintf(file,"  %f\n", gCell[ii]);
        }
    }
};


#pragma mark -

#ifdef DISPLAY

/// display the edges of the grid using OpenGL in 1D
/**
 OpenGL color and line width should specified before the call.
 This is implemented only for ORD==1, 2 and 3.
 */
template <typename CELL, typename INDEX>
bool drawEdges(const Grid<1, CELL, INDEX>& grid)
{
    glBegin(GL_LINES);
    for ( unsigned ix = 0; ix <= grid.dim(0); ++ix )
    {
        real x = grid.position(0, ix);
        glVertex2f(x, -0.5);
        glVertex2f(x,  0.5);
    }
    glEnd();
    return true;
}


/// display the edges of the grid using OpenGL in 2D
/**
 OpenGL color and line width should specified before the call.
 This is implemented only for ORD==1, 2 and 3.
 */
template <typename CELL, typename INDEX>
bool drawEdges(const Grid<2, CELL, INDEX>& grid)
{
    real i = grid.inf(0);
    real s = grid.sup(0);
    glBegin(GL_LINES);
    for ( unsigned iy = 0; iy <= grid.dim(1); ++iy )
    {
        real y = grid.position(1, iy);
        glVertex2f(i, y);
        glVertex2f(s, y);
    }
    glEnd();
    
    i = grid.inf(1);
    s = grid.sup(1);
    glBegin(GL_LINES);
    for ( unsigned ix = 0; ix <= grid.dim(0); ++ix )
    {
        real x = grid.position(0, ix);
        glVertex2f(x, i);
        glVertex2f(x, s);
    }
    glEnd();
    return true;
}


/// display the edges of the grid using OpenGL in 3D
/**
 OpenGL color and line width should specified before the call.
 This is implemented only for ORD==1, 2 and 3.
 */
template <typename CELL, typename INDEX>
bool drawEdges(const Grid<3, CELL, INDEX>& grid)
{
    real i = grid.inf(0);
    real s = grid.sup(0);
    glBegin(GL_LINES);
    for ( unsigned iy = 0; iy <= grid.dim(1); ++iy )
    for ( unsigned iz = 0; iz <= grid.dim(2); ++iz )
    {
        real y = grid.position(1, iy);
        real z = grid.position(2, iz);
        glVertex3f(i, y, z);
        glVertex3f(s, y, z);
    }
    glEnd();
    
    i = grid.inf(1);
    s = grid.sup(1);
    glBegin(GL_LINES);
    for ( unsigned ix = 0; ix <= grid.dim(0); ++ix )
    for ( unsigned iz = 0; iz <= grid.dim(2); ++iz )
    {
        real x = grid.position(0, ix);
        real z = grid.position(2, iz);
        glVertex3f(x, i, z);
        glVertex3f(x, s, z);
    }
    glEnd();
    
    i = grid.inf(2);
    s = grid.sup(2);
    glBegin(GL_LINES);
    for ( unsigned ix = 0; ix <= grid.dim(0); ++ix )
    for ( unsigned iy = 0; iy <= grid.dim(1); ++iy )
    {
        real x = grid.position(0, ix);
        real y = grid.position(1, iy);
        glVertex3f(x, y, i);
        glVertex3f(x, y, s);
    }
    glEnd();
    return true;
}

#endif



#endif
