// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef PLAY_H
#define PLAY_H

///this turns on display code in simul.h
#define  DISPLAY

#include "glapp.h"
#include "simul.h"
#include "parser.h"
#include "display.h"
#include "sim_thread.h"
#include "display_prop.h"
#include "play_prop.h"

class FiberDisp;


///GUI for Cytosim. Display is done by Display, most mouse handling by glApp
namespace Player
{
    
    /// the display properties 
    PropertyList dproperties;
    
    /// the display parameters
    DisplayProp DP("*");
    
    /// the parameters for play
    PlayProp  PP("*");
    
    /// the 'current' FiberDisp on which any change is applied
    FiberDisp * FDisp = 0;
    
    /// SimThread for live simulation
    SimThread simThread(glApp::postRedisplay);
    
    /// make alias
    Simul& simul = simThread.sim();
    
    /// the Display class
    Display * mDisplay = 0;
    
    /// set FDisp;
    void setPointers(bool);

    //---------------------------------COMMANDS---------------------------------
   
    ///adjust window to screen width and tightly around the visible space in 2D.
    void widenDisplay();
   
    /// start animation
    void startForward();
    
    /// start animation
    void startBackward();
    
    /// allow one step of the simulation engine
    void step();
    
    /// stop replay
    void stop();
    
    /// reset the sim-state and timer
    void restart();
    
    /// start or stop the replay
    void startstop();
    
    /// rewind file
    void rewind();

    /// load specified frame
    void readFrame(int);
    
    /// load previous frame
    void previousFrame();
    
    /// go to the next frame, returns 1 if EOF is encountered
    void nextFrame();
    
    /// write global display parameters
    void writePlayParameters(std::ostream & out, bool prune);

    /// write Object display parameters
    void writeDisplayParameters(std::ostream & out, bool prune);

    //-----------------------------------GUI----------------------------------
    
    /// returns a string with some help on what pressing keys do
    void help(std::ostream &);
    
    /// GLUT callback function for timed events
    void timer(int);

    /// build all the menus from scratch
    void buildMenus();
    
    /// GLUT callback function for most keys
    void processNormalKey(unsigned char, int = 0, int = 0);

    /// GLUT callback function for when mouse enters current window
    void processMouseEntry(int);

    /// GLUT callback function for mouse motion, when one button is pressed
    void processMouseAction(const Vector3&, int);
    
    /// GLUT callback function for mouse motion, when no button is pressed
    void processMouseActionMotion(Vector3&, const Vector3&, int);
    
    //-----------------------------DISPLAY------------------------------------
    
    /// initialize display with given style
    void initStyle(int);
    
    /// build left message
    std::string buildMessageL(Simul const&);
    
    /// build right message
    std::string buildMessageR(Simul const&);
    
    /// build central message
    std::string buildMessage(int);

    
    /// set View::focus and quat to match the center of gravity of the Fibers
    void autoTrack(FiberSet const&, View&, int mode);
    
    /// adjust the viewing area
    int  autoScale(SpaceSet const&, View&);

    /// adjust the model view and load frame if asked
    void prepareDisplay(View&, Simul const&, real mag=1);
    
    /// display cytosim state and message
    void displayCytosim(Simul const&);
    
    /// display function for on-screen rendering
    void displayLive();

    /// display function for off-screen rendering
    void displayScene();
    
    
    /// save the displayed image in a graphic file
    int  saveImage(const char * filename, const char* format);
    
    /// save a high-resolution image
    int  saveMagnifiedImage(int mag, const char* filename, const char* format);
    
    /// save the displayed image in a graphic file
    int  saveImage(unsigned indx);
    
    /// save a high-resolution image
    int  saveMagnifiedImage(int mag, unsigned indx);

    //--------------------------------------------------------------------------
    
    /// the different values for PP.play
    enum PlayMode
    {
        PLAY_REVERSE       = -1,
        PLAY_STOP          = 0,
        PLAY_FORWARD       = 1,
        PLAY_FORWARD_WRITE = 2
    };

}


#endif
