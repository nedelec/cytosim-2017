// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef ASTER_H
#define ASTER_H

#include "object.h"
#include "organizer.h"
#include "aster_prop.h"
#include "solid.h"
#include "fiber.h"

class Glossary;

#pragma mark -

/// defines a connection between Solid and Fiber
class AsterClamp
{
public:
    
    /// index of the Solid-point corresponding to MT end
    unsigned int  clamp1;
    
    /// index of the Solid-point corresponding to MT secondary point
    unsigned int  clamp2;
    
    /// abscissa of the MT secondary point
    real clampA;
    
    /// clear member values
    void clear() { clamp1 = 0; clamp2 = 0; clampA = 0; }
    
    /// constructor calls clear()
    AsterClamp() { clear(); }
    
    /// set member values
    void set(unsigned int c1, unsigned int c2, real ca) { clamp1 = c1; clamp2 = c2; clampA = ca; }
    
};


#pragma mark -

/// A radial configuration of Fiber(s) built around a Solid
/**
 The parameters are defined in AsterProp.
 
 Each Fiber is attached to two points on the Solid:
 - a primary point that is tied to the end of the Fiber
 - a secondary point that is tied to the Fiber at some distance from the its end.
 .
 Together they link the Fiber to the Solid, both in position and direction.
 Their stiffness is defined in AsterProp::stiffness.
 To hold \a N fibers in different directions, the Aster uses multiple anchoring points.
 How they are distributed on the Solid depends on the dimensionality, and on: 
 - aster_radius = AsterProp::radius[0] + AsterProp::radius[1]
 - inner_radius = AsterProp::radius[1]
 .
 
 In short:
 -# If inner_radius == 0:
   - The Solid has one primary point, located in the center.
   - in 1D, only two secondary points are used: left and right,
   - in 2D, \a N secondary points are distributed on a circle of radius \a aster_radius.
   - in 3D, \a N secondary points are distributed on the surface of a sphere of radius aster_radius, using PointsOnSphere.
   .
 -# If inner_radius > 0:
   - the secondary points are distributed as if inner_radius==0, 
   - an equal number of primary points are obtained by scaling the secondary points,
     to bring them at distance inner_radius from the center.
   .
 .
 
 The distribution of the points could easily be changed to create non-isotropic fiber arrangements.
 */
class Aster : public Organizer
{
private:
    
    /// store the information needed to make the links between Solid and Fiber
    Array<AsterClamp> asClamp;
    
    /// create and configure the Solid
    Solid *       buildSolid(Glossary& opt, Simul&);
    
public:
    
    /// Property
    AsterProp const* prop;
    
    /// constructor
    Aster(AsterProp const* p) : prop(p) { }
    
    /// destructor  
    virtual      ~Aster();
    
    /// construct all the dependent Objects of the Organizer
    ObjectList    build(Glossary&, Simul&);
    
    /// return the scaffolding Solid
    Solid *       solid() const { return static_cast<Solid*>(organized(0)); }
    
    /// return the center of the Solid
    Vector        position() const { return solid()->posPoint(0); }
    
    /// return Fiber \a n
    Fiber *       fiber(unsigned int n) const { return static_cast<Fiber*>(organized(n+1)); }
    
    /// perform one Monte-Carlo step
    void          step();
    
    /// add interactions to the Meca
    void          setInteractions(Meca &) const;
    

    ///number of connections between the sphere() and fiber()
    unsigned int  nbLinks() const;
    
    ///the position on the Sphere to which Fiber ii is attached
    Vector        posLink1(unsigned int ii) const;
    
    ///the position of the Fiber ii which is attached
    Vector        posLink2(unsigned int ii) const;
    
    /// display parameters 
    PointDisp *   pointDisp() const { return solid()->prop->disp; }
        
    /// a unique character identifying the class
    static const Tag TAG = 'a';
    
    /// return unique character identifying the class
    Tag           tag() const { return TAG; }
    
    /// return Object Property
    const Property* property() const { return prop; }
    
    /// read from IO
    void          read(InputWrapper&, Simul&);
    
    /// write to IO
    void          write(OutputWrapper&) const;
    
};


#endif

