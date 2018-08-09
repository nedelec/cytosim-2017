// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef COUPLE_H
#define COUPLE_H

#include "object.h"
#include "hand_monitor.h"
#include "couple_prop.h"
#include "hand.h"

class Meca;
class Glossary;
class FiberGrid;


/// A set of two Hand linked by an elastic element
/**
 A Couple contains two pointers to Hand:
 - cHand1
 - cHand2
 .
 There are 4 possible states for a Couple:
 - state FF [0]: cHand1 and cHand2 are free,
 - state AF [1]: cHand1 is bound, cHand2 is free,
 - state FA [2]: cHand1 is free, cHand2 is bound,
 - state AA [3]: both hands are attached
 .
 The method state() return the state of the Couple in 0-3, but it is 
 usually not necessary, since there are separate lists in CoupleSet.

 Generally the Couple behaves according to its state:
 - FF     : the Couple is diffusing and both Hands are trying to bind fibers,
 - AF, FA : the localization is given by the attachement point on the fiber,
 - AA     : the Couple is acting as a Hookean spring between the two fibers.
 .
 
 The default Couple has:
 - a zero resting length (it uses Meca:interLink())
 - no specificity
 .

 @ingroup CoupleGroup
 */
class Couple : public Object, public HandMonitor
{
protected:

    /// property
    CoupleProp const* prop;
    
    /// position and position in previous step of complex
    Vector            cPos;
    
public:
    
    /// first Hand
    Hand       * cHand1;
    /// second Hand
    Hand       * cHand2;
        
protected:
    
    /// specialization of HandMonitor
    bool      allowAttachment(const FiberBinder &);
    /// specialization of HandMonitor
    void      afterAttachment();
    /// specialization of HandMonitor
    void      beforeDetachment();
    /// specialization of HandMonitor
    void      afterDetachment();
    /// specialization of HandMonitor
    Number    objNumber() const { return Object::number(); }
    /// specialization of HandMonitor
    Hand *    otherHand(Hand *) const;
    /// specialization of HandMonitor
    Vector    otherDirection(Hand *) const;
    /// specialization of HandMonitor
    real      interactionLength() const;
    
public:
    
    /// create following the specifications in the CoupleProp
    Couple(CoupleProp const*, Vector const & w = Vector(0,0,0));

    /// destructor
    virtual ~Couple();

    /// copy operator
    Couple&  operator=(Couple const&);
    
    //--------------------------------------------------------------------------
    
    /// change the property and update the two Hands
    void           setProperty(CoupleProp *);
   
    /// add interactions to the Meca
    virtual void   setInteractions(Meca &) const;
    
    //--------------------------------------------------------------------------
    
    /// the position of the complex, calculated from cPos, cHand1 and cHand2
    virtual Vector position() const;
   
    /// Couple can be displaced only if it is not attached
    virtual bool   translatable()              const { return !cHand1->attached() && !cHand2->attached(); }
    
    /// translate object's position by the given vector
    virtual void   translate(Vector const& w)        { cPos += w; }
    
    /// move object to specified position
    virtual void   setPosition(Vector const& w)      { cPos = w; }

    /// modulo the current position vector in the space
    virtual void   foldPosition(const Modulo* s);
    
    /// set the position randomly inside the confining Space
    void           randomizePosition();
    
    //--------------------------------------------------------------------------
    
    ///stiffness of the link ( = prop->stiffness )
    real           stiffness()    const;
    
    /// return one Hand that is attached, or zero if both are detached
    Hand *         attachedHand() const;
    
    /// force between hands, essentially: stiffness * ( cHand2->posHand() - cHand1->posHand() )
    virtual Vector force1() const;
     
    /// cosine of the angle between the two Fibers attached by the hands
    real           cosAngle()                  const { return cHand1->dirFiber() * cHand2->dirFiber(); }
   
    /// position on the side of fiber1 used for sideInteractions
    virtual Vector posSide()                   const { return cHand1->pos(); }
    
    /// the position of the complex if it is unattached
    Vector         posFree()                   const { return cPos; }
   
    //--------------------------------------------------------------------------

    /// simulation step for a free Couple: diffusion
    virtual void   stepFF(const FiberGrid&);
    
    /// simulation step for a Couple attached by Hand1
    void           stepAF(const FiberGrid&);
    
    /// simulation step for a Couple attached by Hand2
    void           stepFA(const FiberGrid&);
    
    /// simulation step for a Bridge Couple
    void           stepAA();

    //--------------------------------------------------------------------------

    /// pointer to Hand1
    Hand const*    hand1()                               const { return cHand1; }
    
    /// true if Hand1 is attached
    bool           attached1()                           const { return cHand1->attached(); }
    
    /// position of Hand1 when attached (invalid call otherwise)
    Vector         pos1()                                const { return cHand1->pos(); }
    
    /// Fiber to which Hand1 is attached, or zero if attached to another kind of Fiber
    Fiber*         fiber1()                              const { return cHand1->fiber(); }
    
    /// attach Hand1 at given abcissa
    void           attachTo1(Fiber* f, real ab)                { cHand1->attach(FiberBinder(f, ab)); }

    /// attach Hand1 at the given position
    void           attachTo1(Fiber *f, real ab, FiberEnd from) { cHand1->attachTo(f, ab, from); }
    
    /// attach Hand1 at the given end
    void           attachToEnd1(Fiber *f, FiberEnd end)        { cHand1->attachToEnd(f, end); }
    
    /// attach Hand1 at the given FiberBinder
    void           attach1(FiberBinder const& fb)              { cHand1->attach(fb); }

    //--------------------------------------------------------------------------

    /// pointer to Hand2
    Hand const*    hand2()                               const { return cHand2; }
    
    /// true if Hand2 is attached
    bool           attached2()                           const { return cHand2->attached(); }
    
    /// position of Hand2 when attached (invalid call otherwise)
    Vector         pos2()                                const { return cHand2->pos(); }
    
    /// Fiber to which Hand2 is attached, or zero if attached to another kind of Fiber
    Fiber*         fiber2()                              const { return cHand2->fiber(); }
    
    /// attach Hand1 at given abcissa
    void           attachTo2(Fiber* f, real ab)                { cHand2->attach(FiberBinder(f, ab)); }

    /// attach Hand2 at the given position
    void           attachTo2(Fiber *f, real ab, FiberEnd from) { cHand2->attachTo(f, ab, from); }
    
    /// attach Hand2 at the given end
    void           attachToEnd2(Fiber *f, FiberEnd end)        { cHand2->attachToEnd(f, end); }
    
    /// attach Hand2 at the given FiberBinder
    void           attach2(FiberBinder const& fb)              { cHand2->attach(fb); }

    //--------------------------------------------------------------------------

    /// a static_cast<> of Node::next()
    Couple *       next()        const { return static_cast<Couple*>(nNext); }
    
    /// a static_cast<> of Node::prev()
    Couple *       prev()        const { return static_cast<Couple*>(nPrev); }
    
    //------------------------------ read/write --------------------------------

    /// a unique character identifying the class
    static const Tag TAG = 'c';
    
    /// return unique character identifying the class
    Tag            tag() const { return TAG; }
    
    /// return Object Property
    const Property* property() const { return prop; }
    
    /// write to file
    void           write(OutputWrapper&) const;
    
    /// read from file
    void           read(InputWrapper&, Simul&);
    
    /// return PointDisp of Hand1
    PointDisp *    disp1() const { return cHand1->prop->disp; }
    
    /// return PointDisp of Hand2
    PointDisp *    disp2() const { return cHand2->prop->disp; }
    
    //--------------------------------------------------------------------------

    /// return link type for doubly-attached couples
    int       whichLinkAA() const;
    
    /// return link type for singly-attached couples
    int       whichLinkAF() const;


};


#endif

