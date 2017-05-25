// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef NUCLEUS_H
#define NUCLEUS_H

#include "nucleus_prop.h"
#include "organizer.h"
#include "sphere.h"
#include "fiber.h"

//------------------------------------------------------------------------------
/// Organizer built around a Sphere
/**
A Nucleus attaches Fibers to a Sphere:\n
 - organized(0) is the Sphere
 - organized(n) for n>0 is a Fiber attached to the sphere
 - prop->focus designate the end of each Fiber that is attached on the sphere.
 - prop->stiffness is the stiffness of the link.
 .
 */
class Nucleus : public Organizer
{
    
public:
    
    /// Properties for the Nucleus
    NucleusProp const* prop;
        
    //------------------- construction and destruction -------------------------
    /// constructor
    Nucleus(NucleusProp const* p) : prop(p) { }

    /// construct all the dependent Objects of the Organizer
    ObjectList    build(Glossary&, Simul&);
    
    /// destructor
    virtual      ~Nucleus() { prop = 0; }
    
    //------------------- simulation -------------------------------------------    

    /// monte-carlo simulation step
    void          step();
    
    ///add interactions for this object to the Meca
    void          setInteractions(Meca &) const;

    //------------------- querying the nucleus ---------------------------------    
    
    ///position of center of gravity (returns the center of the sphere)
    Vector        position() const { return sphere()->position(); }
    
    ///the Sphere on which the nucleus is built
    Sphere *      sphere()   const { return static_cast<Sphere*>(organized(0)); }
    
    ///the Sphere on which the nucleus is built
    Fiber *       fiber(int ii) const { return static_cast<Fiber*>(organized(ii+1)); }
    
    
    ///number of connections between the sphere() and fiber()
    unsigned int  nbLinks() const;
    
    ///the position on the Sphere to which Fiber ii is attached
    Vector        posLink1(unsigned int ii) const;
    
    ///the position of the Fiber ii which is attached
    Vector        posLink2(unsigned int ii) const;
    
    /// display parameters 
    PointDisp *   pointDisp() const { return sphere()->prop->disp; }
    
    //------------------------------ read/write --------------------------------
    
    /// a unique character identifying the class
    static const Tag TAG = 'n';
    
    /// return unique character identifying the class
    Tag           tag() const { return TAG; }
    
    /// return Object Property
    const Property* property() const { return prop; }

};



#endif

