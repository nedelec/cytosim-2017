// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef DISPLAY3_H
#define DISPLAY3_H

#include "display.h"
#include "real.h"
#include "opengl.h"
#include "vector.h"
class PointDisp;

///Cytosim display class - style 3
/**
 This style is for rendering in 3D.
 It uses Lighting for better volume rendering
 */
class Display3 : public Display
{
private:
     
    /// display list for one Space
    GLuint spaceDL;
    
    /// Space for which the spaceDL was built
    const Space * dlSpace;
    
    
    ///display a ball
    void displayBall(Vector const&, real radius);

    ///display a point
    void displayPoint(Vector const&, PointDisp const*);

public:
        
    ///constructor
    Display3(DisplayProp const*);
    
    ///destructor
    ~Display3();
    
    ///display the given simulation state using OpenGL commands
    void display(Simul const&);
    
    ///display back of Space
    void displayBSpace(Space const&);
    
    ///display front of Space
    void displayFSpace(Space const&);
    
    ///display a Fiber
    void displayFiberMinusEnd(Fiber const&);
    
    ///display a Fiber
    void displayFiberPlusEnd(Fiber const&);
    
    ///display a Fiber
    void displayFiber(Fiber const&);
    
    ///display the Fiber points
    void displayFiberPoints(Fiber const&);
    
    ///display Fibers
    virtual void displayFibersPoints(FiberSet const&);

    ///display the Solids
    void displaySolid(Solid const&);
 
    ///display the transparent parts of Solid
    void displayTSolid(Solid const&, unsigned int);

    ///display a Bead
    void displayBead(Bead const&);
    
    ///display transparent membrane of Bead
    void displayTBead(Bead const&);
    
    ///display a Sphere
    void displaySphere(Sphere const&);
    
    ///display transparent membrane of Sphere
    void displayTSphere(Sphere const&);
    
    //display an Organizer
    void displayOrganizer(Organizer const&);
    
    ///display points on the Nucleus
    void displayPointsOnNucleus(Simul const&);

    ///display free Couple
    void displayFCouples(CoupleSet const&);
    
    ///display attahed Couple
    void displayACouples(CoupleSet const&);

    ///display bridging Couple
    void displayBCouples(CoupleSet const&);
    
    ///display the free Single
    void displayFSingles(SingleSet const&);
    
    ///display the attached Single
    void displayASingles(SingleSet const&);
 
};

#endif

