// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SPACE_H
#define SPACE_H

#include "sim.h"
#include "real.h"
#include "node.h"
#include "vector.h"
#include "object.h"
#include "common.h"
#include <string>
#include "space_prop.h"


class PointExact;
class PointInterpolated;
class Simul;
class Meca;

//------------------------------------------------------------------------------

/// Defines the spatial constrains in cytosim
/**
Confined Space needs to define two important functions:\n
 - inside(x), which tells if a position is inside the space or outside,
 - project(x,p), which projects x perpendicularly on the edge of the space.
 .
The edges are considered to be inside.
*/
class Space : public Object
{
    /// private length setting
    void length(unsigned int d, real v);

protected:
    
    /// max number of dimensions
    static const unsigned int DMAX = 8;
    
    /// number of dimensions defined in mLength[]
    unsigned int nLength;
        
    /// dimensions that define the geometry
    real         mLength[DMAX];
    
    /// double of each dimension
    real         mLength2[DMAX];
    
    /// square of each dimension
    real         mLengthSqr[DMAX];
    
public:
    
    /// parameters
    const SpaceProp* prop;
        
public:
    
    /// constructor
    Space(const SpaceProp* p);
    
    /// destructor
    virtual ~Space();
    
    //------------------------------ BASIC -------------------------------------
    
    /// number of dimensions defined by the user
    unsigned int nbLengths() const { return nLength; }
    
    /// return dimension \a d
    real length(unsigned int d) const { assert_true(d<DMAX); return mLength[d]; }
 
    /// return double dimension \a d
    real length2(unsigned int d) const { assert_true(d<DMAX); return mLength2[d]; }

    /// return squared dimension \a d
    real lengthSqr(unsigned int d) const { assert_true(d<DMAX); return mLengthSqr[d]; }
    
    /// read dimensions from a stream
    void readLengths(const std::string&);
    
    /// check that \a required lengths have been specified
    void checkLengths(unsigned int required, bool positive) const;
    
    /// change dimension \a d to \a v, and update derived variables (this calls resize())
    void resize(unsigned int d, real v);
    
    /// this is called if any length has been changed
    virtual void resize() {}

    //------------------------------ OBJECT ------------------------------------
    
    /// the volume inside in 3D, or the surface area in 2D
    virtual real   volume() const = 0;
    
    /// Maximum absolute value of X, Y and Z taken over all points inside.
    /**
     @return [ max(|X|), max(|Y|), max(|Z|) ]
     
     This is a weaker form of 'minimum bounding rectangle'.
     It defines a rectangle that is centered around the origin, containing the entire volume.
     The bounds are not required to be minimal, but most algorithms are more efficient when that is the case.
     */
    virtual Vector extension() const = 0;
        
    /// true if \a point is inside or on the edge of this Space 
    virtual bool   inside(const real point[]) const = 0;
    
    /// set \a proj to be the projection (ie. nearest point) of \a point on the edge of this Space.
    virtual void   project(const real point[], real proj[]) const = 0;
    
    /// apply a force directed towards the edge of this Space, for a point located at `pos`
    virtual void   setInteraction(Vector const& pos, PointExact const&, Meca &, real stiff) const;
    
    /// apply a force directed towards the edge of this Space deflated by \a radius
    virtual void   setInteraction(Vector const& pos, PointExact const&, real rad, Meca &, real stiff) const;
    
    /// true if a sphere (\a center, \a radius) is entirely inside this Space
    virtual bool   allInside(const real center[], real rad) const;
    
    /// true if a sphere (\a center, \a radius) is entirely outside this Space
    virtual bool   allOutside(const real center[], real rad) const;
    
    //---------------------------- DERIVED -------------------------------------
    
    /// true if \a point  is outside this Space ( defined as !inside(point) )
    bool           outside(const real point[])  const { return ! inside(point); }
    
    /// project \a point on this Space deflated by \a radius
    void           project(const real point[], real proj[], real rad) const;
    
    /// project \a point on edge of this Space, and return the result in \a point
    void           project(real point[]) const;
    
    
    /// the square of the distance to the edge of this Space
    real           distanceToEdgeSqr(const real point[]) const;
    
    /// the distance to the edge, always positive
    real           distanceToEdge(const real point[]) const { return sqrt(distanceToEdgeSqr(point)); }
                                                                      
    /// the distance to the edge, positive if \a point is outside, and negative if inside
    real           signedDistanceToEdge(const real point[]) const;
    
    /// bounce the object off the wall to put it back inside
    void           bounce(real w[]) const;
    
    
    /// a Vector perpendicular to the space edge at \a point, directed towards the outside
    virtual Vector normalToEdge(const real point[]) const;
    
    /// a random position in the volume
    Vector         randomPlace()  const;

    /// a random position located at most at distance \a radius from the edge
    virtual Vector randomPlaceNearEdge(real rad) const;
    
    /// a random position located on the edge
    /** obtained by projection of randomPlaceNearEdge() */
    Vector         randomPlaceOnEdge(real rad) const;
    
    /// estimate Volume using a poor Monte-Carlo method with \a cnt trials
    virtual real   estimateVolume(unsigned long cnt) const;

    //------------------------------ SIMULATION ---------------------------------
    
    /// one Monte-Carlo simulation step
    virtual void   step() {}
    
    /// add interactions to a Meca
    virtual void   setInteractions(Meca &) const {}

    //------------------------------ READ/WRITE --------------------------------
    
    /// a unique character identifying the class
    static const Tag TAG = 'e';
    
    /// return unique character identifying the class
    Tag           tag() const { return TAG; }
    
    /// return Object Property
    const Property* property() const { return prop; }
    
    /// read from file
    virtual void  read(InputWrapper&, Simul&);
    
    /// write to file
    virtual void  write(OutputWrapper&) const;
    
    /// a static_cast<> of Node::next()
    Space*        next()  const  { return static_cast<Space*>(nNext); }
    
    /// a static_cast<> of Node::prev()
    Space*        prev()  const  { return static_cast<Space*>(nPrev); }
    
    //------------------------------ DISPLAY ----------------------------------
    
    /// a shape-specific openGL display function, return true is display was done
    /**
     In 2D, this should cover the inside area using polygons primities.
     in 3D, this should draw the surface of the space, using polygon primities.
     */
    virtual bool  display()  const { return false; }

    /// display the outline of a section of the box
    void          displaySection(int dim, real pos, real step) const;

};

#endif

