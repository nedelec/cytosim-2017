// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef DISPLAY1_H
#define DISPLAY1_H

#include "display.h"
class PointDisp;

///Cytosim display class - style 1
/**
 This is the standard 2D display.
 It implements most of the characteristics in PointDisp and FiberDisp
 */
class Display1 : public Display
{
    ///display a ball
    void displayBall(Vector const&, real radius);
    
    ///display a point
    void displayPoint(Vector const&, PointDisp const*);

public:
    
    ///constructor
    Display1(DisplayProp const*);
    
    ///destructor
    ~Display1() {}
    
    
    ///display the given simulation state using OpenGL commands
    void display(Simul const&);
    
    ///display MINUS_END of a Fiber
    void displayMinusEnd(int style, Fiber const&, real width) const;
    
    ///display PLUS_END of a Fiber
    void displayPlusEnd(int style, Fiber const&, real width) const;
    
    ///display a Fiber
    void displayFiberMinusEnd(Fiber const&);
    
    ///display a Fiber
    void displayFiberPlusEnd(Fiber const&);

    ///display a Fiber
    void displayFiber(Fiber const&);
    
    ///display the Solids
    void displaySolid(Solid const&);
 
    ///display the transparent part for the Solids
    void displayTSolid(Solid const&, unsigned int);
    
    ///display a Bead
    void displayBead(Bead const&);
    
    ///display transparent membrane of Bead
    void displayTBead(Bead const&);
    
    ///display a Sphere
    void displaySphere(Sphere const&);
    
    ///display transparent membrane of Sphere
    void displayTSphere(Sphere const&);
    
    ///display an Organizer
    void displayOrganizer(Organizer const&);
    
    ///display the free Couples
    void displayFCouples(CoupleSet const&);
    
    ///display the attached Couples
    void displayACouples(CoupleSet const&);
    
    ///display the bridging Couples
    void displayBCouples(CoupleSet const&);
    
    ///display the free Singles
    void displayFSingles(SingleSet const&);

    ///display the attached Singles
    void displayASingles(SingleSet const&);
};

#endif

