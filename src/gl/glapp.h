// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
//F. Nedelec, Dec 2005

#ifndef GLAPP_H
#define GLAPP_H

#include "real.h"
#include "glut.h"
#include "gle_color.h"
#include "vector3.h"
#include "quaternion.h"
#include "glapp_prop.h"
#include "view.h"
#include <vector>

///glApp extends GLUT to manipulate a 2D or 3D display window
namespace glApp
{
    ///parameters for glApp
    extern glAppProp GP;
    
    /// different View associated with the display windows
    extern std::vector<View> views;

    ///
    void       build();
    
    ///set display function \a func and dimensionality \a dim
    void       init(void (*func)(), int dim, Glossary* glos = 0);

    ///set dimensionnality (this affects mostly mouse controls)
    void       setDIM(int sDIM);
    
    ///create a new display window
    int        createWindow(const char * = 0);

    ///draw rectangle with discontinuous lines
    void       drawROI(Vector3 roi[2]);
    
    ///set the zoom window to the ROI
    void       matchROI(int view);
    
    ///destroy window
    void       destroyWindow(int win);
    
    ///enter or exit glut full-screen mode
    void       enterFullScreen();

    ///enter or exit glut full-screen mode
    void       exitFullScreen();
    
    ///enter or exit glut full-screen mode
    void       switchFullScreen();
    
    ///GLUT callback function for window resize event
    void       windowReshaped(int, int);
    
    ///timer callback for automatic rotation
    void       travelingTimer(int);
    
    ///set the range normally visible for zoom = 1
    void       setScale(real);

    /// return view associated with current window
    View&      currentView();
    
    ///reset current view
    void       resetView();
    
    ///reset all views
    void       resetAllViews();
    
    //--------------------------------- MENUS -----------------------------------
    
    ///create a menu with cnt 'unset' entries, using func for callbacks
    void       clearMenu(int menuID);
    
    ///build menu, and attach it if argument is a valid button (-1: do not attach)
    int        buildMenu();
    
    ///callback function for the menu build by buildMenu()
    void       processMenuEvent(int item);
    
    /// attach default menu to button
    void       attachMenu(int button);

    //-----------------------------------KEYS------------------------------------
    
    ///set array of flags toggled by pressing the function-keys (F1->F12)
    void       bindFunctionKeys(bool *);

    ///return true if function 'k' was turned 'on'
    bool       functionKey(int k);
    
    ///toggle one of the function key
    void       toggleFunctionKey(int key);

    ///returns a string describing mouse and keyboard driven events
    void       help(std::ostream & = std::cout);
    
    ///GLUT callback function for normal keys
    void       processNormalKey(unsigned char, int mouseX = 0, int mouseY = 0);
    
    ///GLUT callback function for arrow/function keys
    void       processSpecialKey(int key, int mouseX, int mouseY);
    
    //-----------------------------------MOUSE-----------------------------------
        
    ///GLUT callback function for mouse button down/up
    void       mouseClick(int button, int state, int x, int y);
    
    ///GLUT callback function for mouse motion, when a button is pressed
    void       mouseMotion(int x, int y);
    
    ///GLUT callback function for mouse motion, when no button is pressed
    void       passiveMouseMotion(int x, int y);
    
    ///set callback for shift-click, with unprojected down-position
    void       actionFunc(void (*func)(const Vector3 &, int));
    
    ///set callback for shift-click, with unprojected down- and current- mouse positions 
    void       actionFunc(void (*func)(Vector3 &, const Vector3 &, int));
        
    //---------------------------------MESSAGES---------------------------------
    
    ///\todo: replace displayLabel(...) by a std::ostream &
    ///display text permanently in bottom-left corner of the window
    void       displayLabel(const char* fmt, ...);
    
    ///\todo: replace flashText(...) by a std::ostream &
    ///display text for 3 sec (to report that something has been done)
    void       flashText(const char* fmt, ...);
    
    /// timer for flashText() (this is automatically called)
    void       flashTimer(int);
    
    //-------------------------------DISPLAY------------------------------------

    /// called before display of scene
    void       setDisplay(View const&);
    
    /// called after display of scene
    void       endDisplay(View const&, bool);

    
    /// display function for main window
    void       mainDisplay();
    
    /// display function for secondary windows
    void       plainDisplay();

    /// set display function
    void       displayFunc(void (*func)());
    
    /// this will call glutPostRedisplay() for all windows open by glApp
    /** call glutPostRedisplay() if only the current window needs to be updated */
    void       postRedisplay();
}


#endif
