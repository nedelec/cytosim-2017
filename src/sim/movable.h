// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MOVABLE_H
#define MOVABLE_H

#include "vector.h"
#include "rotation.h"

class Glossary;
class Space;
class Modulo;


/// Can be moved and rotated in space
/**
This provides a common interface, in which translation and rotation are disabled.
These features can be enabled by redefining the related functions in a derived class.
 
 To enable translation:
 - position() should be implemented
 - translate() should be implemented
 - translatable() should return true
 .
 To enable rotation:
 - rotate() should be implemented
 - rotatable() should return true
 .
 To support periodic boundary conditions, foldPosition() should be defined.
 */
class Movable
{
    
public:
    
    /// read a position specified with primitives, such as 'circle 5', etc.
    static Vector readPrimitive(std::istream&, const Space*);
    
    /// read a position in space
    static Vector readPosition(std::istream&, const Space*);

    /// read an orientation, and return a normalized vector
    static Vector readDirection(std::istream&, const Vector&, const Space*);

    /// read a rotation specified in \a is, at position \a pos
    static Rotation readRotation(std::istream&, const Vector&, const Space*);
    
public:
    
    /// constructor
    Movable() {}
    
    /// destructor
    virtual ~Movable() {}
    
    
    /// return the position in space of the object
    virtual Vector    position()  const { return Vector(0,0,0); } 
    
    /// move object to specified position
    virtual void      setPosition(Vector const&);
    
    /// true if object accepts translations (default=false)
    virtual bool      translatable()  const { return false; }
    
    /// move the object ( position += given vector )
    virtual void      translate(Vector const&);

    
    /// true if object accepts rotations (default=false)
    virtual bool      rotatable()  const { return false; }
    
    /// rotate the object around the origin of coordinates
    virtual void      rotate(Rotation const&);
    
    /// rotate the object around its current position
    virtual void      rotateP(Rotation const&);
    
    
    /// perform modulo for periodic boundary conditions
    /** This brings the object to its centered fiMirror image in Modulo */
    virtual void      foldPosition(Modulo const*) {}
    
};


#endif

