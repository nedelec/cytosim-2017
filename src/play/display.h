// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef DISPLAY_H
#define DISPLAY_H

#include "real.h"
#include "display_prop.h"
#include "property_list.h"

class Simul;
class SingleSet;
class CoupleSet;
class Fiber;
class FiberSet;
class Solid;
class SolidSet;
class Organizer;
class OrganizerSet;
class Space;
class SpaceSet;
class Sphere;
class SphereSet;
class Bead;
class BeadSet;
class FieldSet;
class FiberProp;
class FiberDisp;
class PointDisp;
class LineDisp;

/// defining the DISPLAY keyword enables display code in included files
#define DISPLAY


///Base class to display Cytosim state using OpenGL
class Display
{
protected:
    
    /// associated parameters
    DisplayProp const* prop;
    
    /// the pixel size for this particular display
    real           mPixelSize;
    
    /// scaling factors for glPointSize() and glLineWidth()
    real           uFactor;
    
    /// scaling factors for real units
    real           sFactor;
    
private:
  
    /// set default value of FiberProp
    void prepareFiberDisp(FiberProp *, PropertyList&, gle_color, char);

    /// set values of fiber's LineDisp
    void prepareLineDisp(const Fiber *);

    template < typename T >
    void preparePointDisp(T * prop, PropertyList&, const std::string& kind, gle_color, char);

public:
    
    ///constructor
    Display(DisplayProp const*);
    
    ///destructor
    virtual ~Display() {}
    
    /// set current pixel-size and point-size factor
    void         setPixelFactors(real pixel_size, real u_factor);
    
    ///prepare to display
    virtual void prepareForDisplay(Simul const&, PropertyList&);
    
    ///display the given simulation state using OpenGL commands
    virtual void display(Simul const&);
    
    ///display for periodic systems
    void         displayTiled(Simul const&, int nine);
    
        
    /// find a color for a large spherical objects
    gle_color bodyColor(PointDisp const*, Number) const;

    ///display a scalar field
    virtual void displayFields(FieldSet const&);
    
    ///display the Space in 2D, and the back of the surface in 3D
    virtual void displayBSpace(Space const&);

    ///display the front of the Space in 3D
    virtual void displayFSpace(Space const&);


    
    ///display MINUS_END of a Fiber
    virtual void displayMinusEnd(int style, Fiber const&, real width) const;
    
    ///display PLUS_END of a Fiber
    virtual void displayPlusEnd(int style, Fiber const&, real width) const;

    ///display a Fiber
    virtual void displayFiberMinusEnd(Fiber const&) = 0;
    
    ///display a Fiber
    virtual void displayFiberPlusEnd(Fiber const&) = 0;

    ///display a Fiber
    virtual void displayFiber(Fiber const&) = 0;
    
    ///display a Bead
    virtual void displayBead(Bead const&) = 0;

    ///display translucent elements of a Bead
    virtual void displayTBead(Bead const&) = 0;

    ///display opaque elements of a Solid
    virtual void displaySolid(Solid const&) = 0;
    
    ///display translucent elements of a Solid
    virtual void displayTSolid(Solid const&, unsigned int) = 0;
    
    ///display the Sphere
    virtual void displaySphere(Sphere const&) = 0;

    ///display translucent elements of a Sphere
    virtual void displayTSphere(Sphere const&) = 0;
   
    ///display Fibers
    virtual void displayFibers(FiberSet const&);
    
    ///display the Beads
    virtual void displayBeads(BeadSet const&);
    
    ///display the Solids
    virtual void displaySolids(SolidSet const&);
    
    ///display the Spheres
    virtual void displaySpheres(SphereSet const&);
    
    ///display the Spaces
    virtual void displayBSpaces(SpaceSet const&);
    
    ///display the front of Spaces
    virtual void displayFSpaces(SpaceSet const&);

    ///display translucent object after depth-sorting
    virtual void displayTransparentObjects(Simul const&);

    ///display the Nucleus
    virtual void displayOrganizer(Organizer const&) = 0;
    
    ///display the Nucleus
    virtual void displayOrganizers(OrganizerSet const&);
    
    ///display the free Couples
    virtual void displayFCouples(CoupleSet const&) = 0;

    ///display the attached Couples
    virtual void displayACouples(CoupleSet const&) = 0;

    ///display the bridging Couples
    virtual void displayBCouples(CoupleSet const&) = 0;

    ///display the free Singles
    virtual void displayFSingles(SingleSet const&) = 0;

    ///display the attached Singles
    virtual void displayASingles(SingleSet const&) = 0;

    
    ///display the average fiber for the pool defined by func(obj, val) == true
    virtual void displayAverageFiber(FiberSet const&, bool (*func)(Fiber const*, void*), void*);
    
    ///display the averaged fiber
    virtual void displayAverageFiber1(FiberSet const&, void*);
    
    ///display the average for left-pointing and right-pointing fibers
    virtual void displayAverageFiber2(FiberSet const&, void*);
    
    ///display additional items
    virtual void displayMisc(Simul const&);
    
};


#endif

