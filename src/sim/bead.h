// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef BEAD_H
#define BEAD_H

#include "dim.h"
#include "array.h"
#include "object.h"
#include "mecable.h"
#include "bead_prop.h"

class Meca;
class Single;
class SingleProp;
class Glossary;

/// A single point with a radius
/**
 The Bead is the simplest Mecable.
 It represents a spherical object using: 
 - a position vector,
 - a radius.
 .
 The orientational degrees of freedom are neglected.
 Translation follows Stokes's law.
 A Single can be attached in the center of the bead.
 
 For more elaborate models, see Sphere and Solid.
*/
class Bead : public Mecable
{
public:
    
    /// Property
    BeadProp const* prop;

private:
    
    /// position
    Vector      paPos;
    
    /// radius
    real        paRadius;
    
    /// force on center, during past simulation step
    Vector      paForce;

    /// the total drag coefficient for translation
    real        paDrag;
    
public:
    
    /// create following specifications
    Bead(BeadProp const*, Vector pos, real rad);

    /// destructor
    virtual ~Bead();
    
    //--------------------------------------------------------------------------
    
    /// return the position in space of the object
    Vector       position()              const { return paPos; } 
    
    /// true if object accepts translations (default=false)
    bool         translatable()          const { return true; }
    
    /// move the object position ( position += given vector )
    void         translate(Vector const& w)    { paPos += w; }
    
    /// set the object position ( position = given vector )
    void         setPosition(Vector const& w)  { paPos = w; }

    /// modulo the position (periodic boundary conditions)
    void         foldPosition(const Modulo *);

    //--------------------------------------------------------------------------
        
    /// the radius of the Bead
    real         radius()                const { return paRadius; }
    
    /// the volume of the bead
    real         radiusSqr()             const { return paRadius * paRadius; }
    
    /// set the radius of the Bead
    void         resize(real R)                { assert_true(R>0); paRadius = R; setDragCoefficient(); }
    
    /// the volume of the bead
    real         volume() const;
    
    //--------------------------------------------------------------------------

    /// can only have one point
    unsigned int nbPoints()              const { return 1; }
    
    /// return position of point
    Vector       posPoint(unsigned n)        const { assert_true(n==0); return paPos; }
    
    /// copy coordinates to given array
    void         putPoints(real * x)     const { paPos.put(x); }
    
    /// set position
    void         getPoints(const real * x)     { paPos.get(x); }
    
    /// return Force on point x calculated at previous step by Brownian dynamics
    Vector       netForce(unsigned n)   const { assert_true(n==0); return paForce; }
    
    /// replace current forces by the ones provided
    void         getForces(const real * x)     { paForce.get(x); }
    
    //--------------------------------------------------------------------------
    
    /// sets the mobility
    void        setDragCoefficient();
    
    /// the total drag-coefficient of object (force = drag * speed)
    real        dragCoefficient()        const { return paDrag; }
    
    /// sets the mobility (called at every step)
    /**
     setDragCoefficient() is called when the Bead is created,
     or when it is resized, and nothing needs to be done here.
     */
    void        prepareMecable() {}
    
    /// calculates the speed of points in Y, for the forces given in X
    void        setSpeedsFromForces(const real* X, real* Y, real, bool) const;
    
    /// add contribution of Brownian forces
    real        addBrownianForces(real* rhs, real sc) const;

    /// add the interactions due to confinement
    void        setInteractions(Meca &) const;    
    
    /// monte-carlo step
    void        step();
    
    //---------------------------- next / prev ---------------------------------
    
    /// a static_cast<> of Node::next()
    Bead *      next()  const { return static_cast<Bead*>(nNext); }
    
    /// a static_cast<> of Node::prev()
    Bead *      prev()  const { return static_cast<Bead*>(nPrev); }
    
    //------------------------------ read/write --------------------------------

    /// a unique character identifying the class
    static const Tag TAG = 'b';
    
    /// return unique character identifying the class
    Tag         tag() const { return TAG; }
    
    /// return Object Property
    const Property* property() const { return prop; }
    
    ///read from file
    void        read(InputWrapper&, Simul&);
    
    ///write to file
    void        write(OutputWrapper&) const;

};

#endif
