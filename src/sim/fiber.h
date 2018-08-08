// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FIBER_H
#define FIBER_H

#include <vector>
#include <stdint.h>
#include "rigid_fiber.h"
#include "fiber_prop.h"
#include "node_list.h"
#include "field.h"
#include "array.h"
#include "sim.h"


class Single;
class FiberSet;
class FiberLocus;
class FiberBinder;
class LineDisp;


/// FiberNaked to which FiberBinder may bind
/**
 A Fiber is a Mecable that can be simulated.
 
 - FiberProp * prop points to the physical properties (ie. parameters) of the Fiber.
 - frBinders keeps track of all attached FiberBinders.
 - frRods provides pointers to the Segments of the Fiber.
 - FiberDisp * disp points to display parameters.
 .
 
 Fibers are stored in a FiberSet.
 */
class Fiber: public RigidFiber
{   
private:
    
    /// list of attached FiberBinders
    NodeList            frBinders;
    
    /// array of rods, used in Attachments algorithm
    Array<FiberLocus>   frRods;
    
    /// a grafted used to immobilize the Fiber
    Single *            frGlue;
    
    /// stores abscissa for sever() and delayedSevering()
    std::vector<real>   delayedCuts;
    
    /// called if a Fiber tip has elongated or shortened
    void                updateRange() {}
    
public:
        
    /// cut fiber at distance \a abs from the MINUS_END; returns section [ abs - PLUS_END ] 
    virtual Fiber* severM(real abs);
    
    /// cut fiber at abscissa \a abs from the ORIGIN; returns section [ abs - PLUS_END ] 
    Fiber *        severNow(real abs) { return severM(abs-abscissaM()); }
    
    /// cut fiber at absicca given to sever()
    void           delayedSevering();
    
    
    /// cut Fiber at point \a pti, return section [ pti - PLUS_END ]
    virtual Fiber* severPoint(unsigned int pti);
    
    /// cut fiber at points where consecutive segments make a kink
    void           severKinks();

    
    /// viscous drag coefficient for a cylinder moving close to a surface
    real           dragCoefficientSurface();
    
    /// viscous drag coefficient for a cylinder moving close to a surface
    real           dragCoefficientVolume();
    
public:
    
    /// the Property of this object
    FiberProp const*    prop;
    
    /// the display parameters
    LineDisp mutable*   disp;

    //--------------------------------------------------------------------------

    /// constructor
    Fiber(FiberProp const*);
    
    /// destructor
    virtual ~Fiber();
    
    //--------------------------------------------------------------------------
    
    /// allocate memory for 'nbp' points
    virtual unsigned int allocatePoints(unsigned int nbp);
    
    /// calculate viscous drag coefficient
    void           setDragCoefficient();
    
    /// prepare for Meca
    void           prepareMecable();
    
    /// add interactions to a Meca
    void           setInteractions(Meca &) const;

    /// cut fiber at abscissa \a abs from the ORIGIN, in the next time step 
    void           sever(real abs) { delayedCuts.push_back(abs); }
    
    /// Cut all segments intersecting the plane defined by <em> n.x + a = 0 </em>
    void           cutAlongPlane(FiberSet *, Vector const& n, real a);
    
    /// call FiberNaked::join(), and transfer Hands (caller should delete \a fib).
    virtual void   join(Fiber * fib);
    
    /// simulation step
    virtual void   step();
    
    //--------------------------------------------------------------------------
    
    /// FiberLocus representing the segment [p, p+1]
    FiberLocus&    segment(unsigned int p) const;
    
    /// return the abscissa of the closest position to \c w on this Fiber, and set \a dist to the square of the distance
    real           projectPoint(Vector const& w, real & dist) const;
        
    //--------------------------------------------------------------------------
    
    /// return assembly/disassembly state of the FiberEnd
    virtual int    dynamicState(FiberEnd which) const { return 0; }
    
    /// change state of FiberEnd \a which to \a new_state
    virtual void   setDynamicState(FiberEnd which, int new_state) {}
    
    /// the amount of freshly assembled polymer during the last time step (this has units of length)
    virtual real   freshAssembly(FiberEnd which) const { return 0; }
    
    /// true if the tip \a which has grown in the last time step ( freshAssembly(which) > 0 )
    bool           isGrowing(FiberEnd which) const { return freshAssembly(which) > 0; }
    
    /// true if the tip \a which has shrunk in the last time step ( freshAssembly(which) < 0 )
    bool           isShrinking(FiberEnd which) const { return freshAssembly(which) < 0; }
    
    //--------------------------------------------------------------------------
    
    /// register a new Binder
    void           addBinder(FiberBinder*);
    
    /// unregister bound Binder
    void           removeBinder(FiberBinder*);
    
    /// a FiberBinder bound to this fiber (use ->next() to access all other binders)
    FiberBinder*   firstBinder() const;
    
    /// update all binders
    void           updateBinders();
    
    /// detach all binders
    void           detachBinders();
    
    /// number of attached FiberBinders
    int            nbBinders()    const { return frBinders.size(); }
    
    /// number of attached FiberBinders in a range of abscissa
    int            nbBindersInRange(real aMin, real aMax, FiberEnd from) const;
    
    /// number of attached FiberBinders at the specified FiberEnd
    int            nbBindersNearEnd(real len, FiberEnd which) const;
    
    /// a function to count binders using custom criteria
    int            nbBinders(unsigned int (*count)(FiberBinder const&)) const;
    
    //--------------------------------------------------------------------------
    
    /// set the box glue for pure pushing
    void           setGlue1(Single* glue, FiberEnd which, const Space * space);
    
    /// set the box glue for pure pulling
    void           setGlue2(Single* glue, FiberEnd which, const Space * space);
    
    /// set the box glue for pushing and pulling
    void           setGlue3(Single* glue, const Space * space);
    
    /// a setGlue to rule them all
    void           setGlue(Single*& glue, FiberEnd which, const Space * space, int glue_type);
    
    //--------------------------------------------------------------------------
    
    /// a static_cast<> of Node::next()
    Fiber *  next()  const  { return static_cast<Fiber*>(nNext); }
    
    /// a static_cast<> of Node::prev()
    Fiber *  prev()  const  { return static_cast<Fiber*>(nPrev); }

    //--------------------------------------------------------------------------
    
    /// a unique character identifying the class
    static const Tag TAG = 'f';
    
    /// return unique character identifying the class
    Tag         tag() const { return TAG; }
    
    /// return Object Property
    const Property* property() const { return prop; }
    
    ///write to file
    void        write(OutputWrapper&) const;
    
    ///read from file
    void        read(InputWrapper&, Simul&);
    
};

#endif

