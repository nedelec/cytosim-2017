// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// F. Nedelec started glApp in Dec 2005

#include "glapp.h"
#include "tictoc.h"
#include <cstdarg>
#include "exceptions.h"
#include "glossary.h"
#include "gle.h"


using namespace gle;


namespace glApp
{    
    /*
     Most of the variables below are internal to glApp, and do not need to
     be accessed by the user. This is why they are only declared here and not
     in the header file.
    */
    
    glAppProp  GP("*");
    
    std::vector<View> views;
    
    int        mDIM = 3;            ///< current dimensionality
    int        mZoomWindow = -1;    ///< Window-ID for detailed view
    int        specialKeys = 0;     ///< state of special keys given by GLUT
    
    bool       fKeyArray[17];       ///< default array of function-key states
    bool     * fKey = fKeyArray;    ///< pointer to function-key states changed by bindFunctionKeys()
    char       fKeyString[4*17];    ///< String to display function-key states
    void       buildfKeyString();   ///< build fKeyString[]
    
    // --------------- MOUSE
    
    /// actions that can be performed with the mouse
    enum UserMode
    {
        MOUSE_ROTATE,
        MOUSE_MOVE,
        MOUSE_ACTIVE,
        MOUSE_MOVE_XZ,
        MOUSE_SPIN,
        MOUSE_ZOOM,
        MOUSE_SET_ROI,
        MOUSE_MOVE_ROI,
        MOUSE_SELECT,
        MOUSE_PASSIVE
    };
    
    /// Specifies in which dimensionality each action is valid
    int actionValidity[] = { 3, 2, 2, 3, 2, 2, 2, 4, 4, 0 };
    
    void         nextUserMode(int dir);
    
    /// the current mode (this decide which action is done with the mouse)
    UserMode     userMode = MOUSE_ROTATE;

    
    View         mouseView("backup");
    UserMode     mouseAction = MOUSE_MOVE;  ///< the action being performed by the mouse
    Vector3      mouseDown;                 ///< position where mouse button was pressed down
    Vector3      depthAxis;                 ///< vector normal to desired rotation
    Vector3      mouseAxis;                 ///< axis of rotation for MOUSE_SPIN
    real         mouseZoomScalar;           ///< accessory scalar for zooming
    Vector3      ROI[2], ROIdown[2];        ///< Regions of interest selected with the mouse
    int          savedWindowPos[4] = { 512, 512, 10, 10 };
    
    real         nearZ  = 0;       ///< normalized device Z-coordinate-Z of front-plane
    real         midZ   = 0.5;     ///< normalized device Z-coordinate-Z of middle
    real         farZ   = 1.0;     ///< normalized device Z-coordinate-Z of back-plane

    bool         mouseActive = 0;
    /// function pointer for shift-click actions
    void (*actionFuncClick)(const Vector3 &, int) = 0;
    
    /// function pointer for shift-motion actions
    void (*actionFuncMotion)(Vector3 &, const Vector3 &, int) = 0;
    
    // --------------- GRAPHICS

    bool         showResizeBox = false;     ///< display window resize handle
    unsigned int flash_count = 0;
    std::string  flash;
    
    void         dummyDisplay();
    void (*displayCallback)() = dummyDisplay;
    
    void         setROI(Vector3);
    void         setROI(Vector3, Vector3);
    bool         insideROI(Vector3);    
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 This will disable OpenGL depth-test for DIM<3
 */
void glApp::setDIM(const int d)
{
    if ( mDIM != d )
    {
        mDIM = d;
        //flashText("dimensionality changed to %i", mDIM);
        userMode = ( mDIM == 3 ) ? MOUSE_ROTATE : MOUSE_MOVE;
        GP.depth_test = ( mDIM == 3 );
    }
}

void glApp::displayFunc(void (*func)())
{
    displayCallback = func;
}

//------------------------------------------------------------------------------
void glApp::build()
{
    for ( int ii = 0; ii < 16; ++ii )
        fKey[ii] = false;
    
    /// start with one single view
    views.clear();
    View tmp("view0");
    views.push_back(tmp);
}



/**
 The first argument is a display callback.
 The second argument is the dimensionality (2 or 3) that appears to the user.
 
 glutInit() should be called before glApp::init().
 */
void glApp::init(void (*func)(), int d, Glossary* glos)
{
    if ( 0 == views.size() )
        build();
    
    setDIM(d);
    displayCallback = func;

    if ( glos )
    {
        GP.read(*glos);
        views[0].read(*glos);
    }
    
    createWindow("cytosim");
    
    //enter full-screen mode directly if asked:
    if ( GP.full_screen )
        enterFullScreen();
}

//------------------------------------------------------------------------------
void glApp::enterFullScreen()
{
    GP.full_screen = true;
    //save the current window size and position:
    savedWindowPos[0] = glutGet(GLUT_WINDOW_WIDTH);
    savedWindowPos[1] = glutGet(GLUT_WINDOW_HEIGHT);
    savedWindowPos[2] = glutGet(GLUT_WINDOW_X);
    savedWindowPos[3] = glutGet(GLUT_WINDOW_Y);
    //invoke full screen from GLUT
    glutFullScreen();
    //std::cerr << "Fullscreen window " << glutGetWindow() << std::endl;
}


void glApp::exitFullScreen()
{
    GP.full_screen = false;
    //restore saved window dimensions:
    glutReshapeWindow( savedWindowPos[0], savedWindowPos[1] );
    glutPositionWindow( savedWindowPos[2], savedWindowPos[3] );
}


void glApp::switchFullScreen()
{
    if ( GP.full_screen )
        exitFullScreen();
    else
        enterFullScreen();
}

//------------------------------------------------------------------------------
#pragma mark -

int glApp::createWindow(const char * window_name)
{
    View & view = glApp::currentView();
    
    std::ostringstream oss;
    oss << "rgba";
    if ( GP.buffered )       oss << " double";
    if ( GP.depth_test )     oss << " depth";
    if ( GP.stencil_buffer ) oss << " stencil";
    if ( GP.multisample )
        oss << " samples~" << GP.multisample;
    else
        oss << " samples~8";

    //std::cerr << "GLUT string mode " << oss.str() << std::endl;
    
    // set GLUT display mode:
    glutInitDisplayString(oss.str().c_str());

    // set window size:
    glutInitWindowSize(view.window_size[0], view.window_size[1]);
    
    // set window position:
    view.window_position[0] += 20;
    view.window_position[1] += 20;
    glutInitWindowPosition(view.window_position[0], view.window_position[1]);
    
    int win = 0;
    if ( window_name )
        win = glutCreateWindow(window_name);
    else
    {
        win = glutCreateWindow("GLUT");
        std::ostringstream name;
        name << "Cytosim " << win;
        glutSetWindowTitle(name.str().c_str());
    }
    
    assert_true( win > 0 );
    //std::cerr << "new window " << win << std::endl;

    //duplicate current View:
    if ( win >= (int)views.size() )
        views.resize(win+1, view);
    
    views[win].window(win);
    views[win].initGL(GP.depth_test, GP.multisample, GP.depth_clamp);
    
    glutKeyboardFunc(processNormalKey);
    glutSpecialFunc(processSpecialKey);
    glutReshapeFunc(windowReshaped);
    glutMouseFunc(mouseClick);
    glutMotionFunc(mouseMotion);
    glutPassiveMotionFunc(passiveMouseMotion);
    attachMenu(GLUT_RIGHT_BUTTON);

    if ( win <= 1 )
        glutDisplayFunc(mainDisplay);
    else
        glutDisplayFunc(plainDisplay);
    
    //register call back for traveling:
    travelingTimer(win);
    return win;
}


/**
 This will not destroy the main window
 */
void glApp::destroyWindow(int view)
{
    if ( view == mZoomWindow )
        mZoomWindow = -1;
    
    if ( 1 < view  &&  view < (int)views.size()  &&  views[view].window() > 0 )
    {
        //std::cerr << "Destroy window " << win << std::endl;
        assert_true( views[view].window() == view );
        glutDestroyWindow(views[view].window());
        views[view].window(0);
    }
}

//------------------------------------------------------------------------------

void glApp::windowReshaped(int w, int h)
{
    unsigned int n = glutGetWindow();
    views[n].reshaped(w, h);
    flashText("Window size %i %i", w, h);
    glClear(GL_COLOR_BUFFER_BIT);
    glutPostRedisplay();
}


void glApp::setScale(real s)
{
    int win = glutGetWindow();
    assert_true( win > 0 );
    
    views[0].setScale(s);
    // update all window-associated views:
    for ( unsigned int n = 1; n < views.size(); ++n )
    {
        View & view = views[n];
        if ( view.window() > 0 )
        {
            glutSetWindow(view.window());
            view.setScale(s);
        }
    }
    glutSetWindow(win);
}

//------------------------------------------------------------------------------
#pragma mark -


/// this works even if no window is open
View& glApp::currentView()
{
    assert_true( views.size() > 0 );
    
    if ( views.size() <= 1 )
        return views[0];
    else
        return views[glutGetWindow()];
}


void glApp::resetView()
{
    assert_true( glutGetWindow() < (int)views.size() );
    glApp::currentView().reset();
}


void glApp::resetAllViews()
{
    for ( unsigned int n = 0; n < views.size(); ++n )
        views[n].reset();
}

//------------------------------------------------------------------------------
void glApp::travelingTimer(const int win)
{
    View & view = views[win];

    if ( view.traveling )
    {
        assert_true( view.window() > 0 );
        glutTimerFunc(view.traveling, travelingTimer, win);
        glutSetWindow(view.window());
        view.travelingMotion(0.001*view.traveling);
        glutPostRedisplay();
    }
    else
        glutTimerFunc(500, travelingTimer, win);
}

//------------------------------------------------------------------------------
#pragma mark -

/** Only 2D */
bool glApp::insideROI(Vector3 pos)
{
    bool inX = ( ROI[0].XX < pos.XX  &&  pos.XX < ROI[1].XX );
    bool inY = ( ROI[0].YY < pos.YY  &&  pos.YY < ROI[1].YY );
    return inX && inY;
}

/** Only 2D */
void glApp::setROI(Vector3 a)
{
    ROI[0] = a;
    ROI[1] = a;
}

/** Only 2D */
void glApp::setROI(Vector3 a, Vector3 b)
{
    ROI[0].XX = ( a.XX < b.XX ? a.XX : b.XX );
    ROI[1].XX = ( a.XX < b.XX ? b.XX : a.XX );
    ROI[0].YY = ( a.YY < b.YY ? a.YY : b.YY );
    ROI[1].YY = ( a.YY < b.YY ? b.YY : a.YY );
    ROI[0].ZZ = ( a.ZZ < b.ZZ ? a.ZZ : b.ZZ );
    ROI[1].ZZ = ( a.ZZ < b.ZZ ? b.ZZ : a.ZZ );
}


void glApp::matchROI(int view)
{
    //std::cerr << " zoom on ROI " << win << " wind = " << glutGetWindow() << std::endl;
    if ( view < (int)views.size()  &&  views[view].window() > 0 )
    {
        int w = glutGetWindow();
        glutSetWindow(views[view].window());
        views[view].matchROI(ROI[0], ROI[1]);
        glutPostRedisplay();
        glutSetWindow(w);
    }
}

//------------------------------------------------------------------------------
//------------------------------ keyboard commands -----------------------------
//------------------------------------------------------------------------------
#pragma mark -

void glApp::help(std::ostream & os)
{
    os << "--------------------Mouse Controls----------------------\n";
    os << "  press TAB to toggle between different modes:\n";
    os << "             Rotate           (3D only)\n";
    os << "             Translate in XY\n";
    os << "             Active\n";
    os << "             Translate in XZ  (3D only)\n";
    os << "             Spin\n";
    os << "             Zoom\n";
    os << "             Select ROI\n";
    //os << "             Select Objects\n";
    os << "\n";
    os << "  hold SHIFT for live action!\n";
    os << "  Right click to access the menu\n";
    os << "  Mouse wheel to zoom in/out (this may not work)\n";
    os << "\n";
    os << "--------------------Keyboard Controls--------------------\n";
    os << " + -         Zoom in and out (+SHIFT: small increments\n";
    os << " arrow keys  Translate in XY, hold SHIFT for fine motion\n";
    os << " space       Reset view and refresh display\n";
    os << " h           Hide/show help\n";
    os << " b           Show/hide a 10 um scale bar\n";
    os << " f           toggle fullscreen mode\n";
}


//------------------------------------------------------------------------------
void glApp::nextUserMode(int dir)
{
    do {
        userMode = (UserMode)(( userMode + dir + MOUSE_PASSIVE ) % MOUSE_PASSIVE);
    } while ( actionValidity[userMode] > mDIM );

    switch ( userMode )
    {
        case MOUSE_MOVE:     flashText("Mouse: Translate");    break;
        case MOUSE_ROTATE:   flashText("Mouse: Rotate");       break;
        case MOUSE_ACTIVE:   flashText("Mouse: Active");       break;
        case MOUSE_MOVE_XZ:  flashText("Mouse: Translate XZ"); break;
        case MOUSE_SPIN:     flashText("Mouse: Spin");         break;
        case MOUSE_ZOOM:     flashText("Mouse: Zoom");         break;
        case MOUSE_SET_ROI:  flashText("Mouse: Select ROI");   break;
        case MOUSE_MOVE_ROI: flashText("Mouse: Move ROI");     break;
        case MOUSE_SELECT:   flashText("Mouse: Select");       break;
        case MOUSE_PASSIVE:  flashText("Mouse: Passive");      break;
    }
}


///\todo flexible key assignment map to accomodate different keyboard layouts
void glApp::processNormalKey(unsigned char c, int, int)
{
    View & view = glApp::currentView();
    
    /* In the switch below:
     - use 'break' if the display needs a refresh
     - use 'return' if redrawing is not necessary.
    */
    switch (c)
    {
        case 17:
            if ( glutGetModifiers() &  GLUT_ACTIVE_CTRL )
                exit(EXIT_SUCCESS);
            break;
        
            
        case 9:          // ascii 9 is the horizontal tab
        case 25:         // ascii 25 is shift-tab
            nextUserMode(c==9 ? 1 : -1);
            break;
        
        
        case 27:             // ascii 27 is the escape key
            if ( GP.full_screen )
                exitFullScreen();
            else
                destroyWindow(glutGetWindow());
            break;
        
            
        case 'f':
            switchFullScreen();
            break;
        
        
        case ' ':
            view.reset();
            postRedisplay();
            break;
        
        
        case 'n':
            if ( view.hasClipPlane(0) )
            {
                view.disableClipPlane(0);
                view.disableClipPlane(1);
                flashText("Normal view");
            }
            else
            {
                real off = view.eyeTranslation[2];
                real thk = 0.1 * view.view_size;
                //view.enableClipPlane(0, Vector3(0,0,+1), off, false);
                view.enableClipPlane(0, Vector3(0,0,-1), thk+off, false);
                view.enableClipPlane(1, Vector3(0,0,+1), thk-off, false);
                flashText("Narrow view");
            }
            break;
        
        
        case 'N':
            if ( mouseAction == MOUSE_SET_ROI )
            {
                mZoomWindow = createWindow("view");
                matchROI(mZoomWindow);
            }
            else
            {
                createWindow();
            }
            break;
        
        
        case 'b':
            view.show_scale = ( view.show_scale + 1 ) % 3;
            break;
        
            
        case 'h':
            GP.show_message = ( GP.show_message + 1 ) % 3;
            if ( GP.show_message == 2 ) {
                std::ostringstream oss;
                help(oss);
                GP.message = oss.str();
            }
            else
                GP.message = "Please, visit www.cytosim.org";
            break;
        
        
        case 'x':
            view.show_axes = ! view.show_axes;
            break;
            
        //------------------------- zooming & window resizing:
            
        case '_':
            view.zoom_out(1.0352649238);
            break;
        case '-':
            view.zoom_out(1.4142135623);
            break;
        
        
        case '+':
            view.zoom_in(1.0352649238);
            break;
        case '=':
            view.zoom_in(1.4142135623);
            break;
        
        
        default:
            flashText("glapp ignored key %i [%c]", c, c);
            return;
    }
    
    //if break was used, redisplay is needed:
    postRedisplay();
    buildMenu();
}


//------------------------------------------------------------------------------
void glApp::buildfKeyString()
{
    //rebuild the function key string:
    fKey[0] = false;
    assert_true( sizeof(fKeyString) >= 4*12 );
    for ( int ii = 0; ii < 12; ++ii ) {
        if ( fKey[ii+1] ) {
            fKeyString[3*ii  ] = 'F';
            fKeyString[3*ii+1] = (char)('1'+ii);
            fKey[0] = true;
        }
        else {
            fKeyString[3*ii  ] = ' ';
            fKeyString[3*ii+1] = ' ';
        }
        fKeyString[3*ii+2] = ' ';
    }
    
    if ( fKey[0] )
        fKeyString[3*9+2] = '\0';
    else
        fKeyString[0] = '\0';
}

void glApp::bindFunctionKeys(bool * f)
{
    fKey = f;
    buildfKeyString();
}

bool glApp::functionKey(int k)
{
    if ( 0 < k && k < 17 )
        return fKey[k];
    else
        return false;
}

void glApp::toggleFunctionKey(int key)
{
    switch ( key )
    {
        case GLUT_KEY_F1:    fKey[1]  = !fKey[1];   break;
        case GLUT_KEY_F2:    fKey[2]  = !fKey[2];   break;
        case GLUT_KEY_F3:    fKey[3]  = !fKey[3];   break;
        case GLUT_KEY_F4:    fKey[4]  = !fKey[4];   break;
        case GLUT_KEY_F5:    fKey[5]  = !fKey[5];   break;
        case GLUT_KEY_F6:    fKey[6]  = !fKey[6];   break;
        case GLUT_KEY_F7:    fKey[7]  = !fKey[7];   break;
        case GLUT_KEY_F8:    fKey[8]  = !fKey[8];   break;
        case GLUT_KEY_F9:    fKey[9]  = !fKey[9];   break;
        case GLUT_KEY_F10:   fKey[10] = !fKey[10];  break;
        case GLUT_KEY_F11:   fKey[11] = !fKey[11];  break;
        case GLUT_KEY_F12:   fKey[12] = !fKey[12];  break;
        
        default:
            return;
    }
    
    buildfKeyString();
    postRedisplay();
}

/**
 arrow-keys controls translation, and
 arrow-keys with 'ALT' pressed controls rotation.
 
 motion is reduced by holding down SHIFT.
 */
void glApp::processSpecialKey(int key, int, int)
{
    View & view = glApp::currentView();

    real F = ( glutGetModifiers() & GLUT_ACTIVE_SHIFT ) ? 0.01 : 0.1;
    Vector3 dxy(0, 0, 0);
    
    switch ( key )
    {
        case GLUT_KEY_HOME:      view.reset();            glutPostRedisplay(); return;
        case GLUT_KEY_PAGE_UP:   view.zoom_in(1.4142);    glutPostRedisplay(); return;
        case GLUT_KEY_PAGE_DOWN: view.zoom_out(1.4142);   glutPostRedisplay(); return;
        case GLUT_KEY_LEFT:      dxy.set(-F,0,0);         break;
        case GLUT_KEY_RIGHT:     dxy.set(+F,0,0);         break;
        case GLUT_KEY_DOWN:      dxy.set(0,-F,0);         break;
        case GLUT_KEY_UP:        dxy.set(0,+F,0);         break;
        default:                 toggleFunctionKey(key);  return;
    }
    
    // the center of the window:
    real cx = 0.5 * view.width();
    real cy = 0.5 * view.height();    

    if ( glutGetModifiers() & GLUT_ACTIVE_ALT )
    {
        Vector3 cen = view.unproject(cx, cy, nearZ, true);
        Vector3 mov = view.unproject(cx*(1+dxy.XX), cy*(1+dxy.YY), nearZ, true);
        Quaternion<real> rot;
        rot.setFromAxis(vecProd(cen-mouseView.focus, mov-cen), F * M_PI);        
        view.rotate_by(rot);
    }
    else
    {
        Vector3 cen = view.unproject(cx, cy, midZ, true);
        Vector3 mov = view.unproject(cx*(1+dxy.XX), cy*(1+dxy.YY), midZ, true);
        view.move_by(cen-mov);
    }
    glutPostRedisplay();
}

//------------------------------------------------------------------------------
#pragma mark -

int buildFogMenu()
{
    static int menu = 0;
    if ( menu == 0 )
    {
        menu = glutCreateMenu(glApp::processMenuEvent);
        glutAddMenuEntry("Disable",        100);
        glutAddMenuEntry("Linear ",        101);
        glutAddMenuEntry("Exponential .5", 102);
        glutAddMenuEntry("Exponential 1",  103);
        glutAddMenuEntry("Exponential 2",  104);
        glutAddMenuEntry("Exponential 4",  105);
    }
    return menu;
}

int buildWindowSizeMenu()
{
    static int menu = 0;
    if ( menu == 0 )
    {
        menu = glutCreateMenu(glApp::processMenuEvent);
        glutAddMenuEntry("256x256",   200);
        glutAddMenuEntry("384x384",   201);
        glutAddMenuEntry("512x256",   202);
        glutAddMenuEntry("512x384",   203);
        glutAddMenuEntry("512x512",   204);
        glutAddMenuEntry("768x768",   205);
        glutAddMenuEntry("1024x512",  210);
        glutAddMenuEntry("1024x768",  211);
        glutAddMenuEntry("1024x1024", 212);
        glutAddMenuEntry("1080x720",  213);  //HDTV
        glutAddMenuEntry("2048x1024", 214);
        glutAddMenuEntry("2048x1536", 215);
        glutAddMenuEntry("2048x2048", 220);
    }
    return menu;
}


int buildClipMenu()
{
    static int menu = 0;
    if ( menu == 0 )
    {
        menu = glutCreateMenu(glApp::processMenuEvent);
        glutAddMenuEntry("Disable",    300);
        
        glutAddMenuEntry(" 0 > X",     301);
        glutAddMenuEntry(" X < 0",     302);
        glutAddMenuEntry("-1 < X < 1", 303);
        
        glutAddMenuEntry(" 0 < Y",     311);
        glutAddMenuEntry(" Y < 0",     312);
        glutAddMenuEntry("-1 < Y < 1", 313);
        
        glutAddMenuEntry(" 0 < Z",     321);
        glutAddMenuEntry(" Z < 0",     322);
        glutAddMenuEntry("-1 < Z < 1", 323);
        glutAddMenuEntry("-0.5 < Z < 0.5", 324);
    }
    return menu;
}


int glApp::buildMenu()
{
    static int menu = 0;
    static int menu1, menu2, menu3;
    
    //std::cerr << "buildMenu" << std::endl;
    if ( menu )
        clearMenu(menu);
    else {
        menu1 = buildFogMenu();
        menu2 = buildWindowSizeMenu();
        menu3 = buildClipMenu();
        menu  = glutCreateMenu(processMenuEvent);
    }
    
    glutAddMenuEntry("Reset View",         1);
    glutAddMenuEntry("Show/hide Scalebar", 2);
    glutAddMenuEntry("Show/hide XYZ-axes", 3);
    glutAddMenuEntry(GP.full_screen?"Exit Fullscreen":"Enter Fullscreen", 4);
    glutAddMenuEntry(mDIM==2?"Use 3D Controls":"Use 2D Controls", 7);
    glutAddSubMenu("Fog",            menu1);
    glutAddSubMenu("Window Size",    menu2);
    glutAddSubMenu("Slice",          menu3);
    glutAddMenuEntry("New Window",   10);
    glutAddMenuEntry("Close Window", 11);
    glutAddMenuEntry("Quit",         20);
    
    return menu;
}

//------------------------------------------------------------------------------

void glApp::clearMenu(int menu)
{
    glutSetMenu(menu);
    const int mx = glutGet(GLUT_MENU_NUM_ITEMS);
    for ( int m = mx; m > 0; --m )
        glutRemoveMenuItem(m);
    assert_true( glutGet(GLUT_MENU_NUM_ITEMS) == 0 );
}

void glApp::attachMenu(int b)
{
    buildMenu();
    assert_true( b==GLUT_LEFT_BUTTON || b==GLUT_MIDDLE_BUTTON || b==GLUT_RIGHT_BUTTON );
    glutAttachMenu(b);
}

void glApp::processMenuEvent(int item)
{
    View & view = glApp::currentView();
    switch( item )
    {
        case 0:   return;
        case 1:   view.reset();                      break;
        case 2:   view.show_scale=!view.show_scale;  break;
        case 3:   view.show_axes=!view.show_axes;    break;
        case 4:   switchFullScreen();                break;
        case 7:   setDIM(mDIM==2?3:2);               break;

        case 10:  createWindow();                    break;
        case 11:  destroyWindow(glutGetWindow());    break;
        
        case 20:  exit(EXIT_SUCCESS);                break;
            
        case 100: view.setFog(0, 0);                 break;
        case 101: view.setFog(GL_LINEAR, 1);         break;
        case 102: view.setFog(GL_EXP, 0.5);          break;
        case 103: view.setFog(GL_EXP, 1);            break;
        case 104: view.setFog(GL_EXP, 2);            break;
        case 105: view.setFog(GL_EXP, 4);            break;
            
        case 200: glutReshapeWindow(256, 256);       break;
        case 201: glutReshapeWindow(384, 384);       break;
        case 202: glutReshapeWindow(512, 256);       break;
        case 203: glutReshapeWindow(512, 384);       break;
        case 204: glutReshapeWindow(512, 512);       break;
        case 205: glutReshapeWindow(768, 768);       break;
        case 210: glutReshapeWindow(1024, 512);      break;
        case 211: glutReshapeWindow(1024, 768);      break;
        case 212: glutReshapeWindow(1024, 1024);     break;
        case 213: glutReshapeWindow(1080, 720);      break;
        case 214: glutReshapeWindow(2048, 1024);     break;
        case 215: glutReshapeWindow(2048, 1536);     break;
        case 220: glutReshapeWindow(2048, 2048);     break;
        
        
        case 300:
            view.disableClipPlane(0);
            view.disableClipPlane(1);
            break;
            
        case 301:
            view.enableClipPlane(0, Vector3(+1,0,0), 0);
            view.disableClipPlane(1);
            break;
            
        case 302:
            view.enableClipPlane(0, Vector3(-1,0,0), 0);
            view.disableClipPlane(1);
            break;
            
        case 303:
            view.enableClipPlane(0, Vector3(+1,0,0), 1);
            view.enableClipPlane(1, Vector3(-1,0,0), 1);
            break;
 
        case 311:
            view.enableClipPlane(0, Vector3(0,+1,0), 0);
            view.disableClipPlane(1);
            break;
            
        case 312:
            view.enableClipPlane(0, Vector3(0,-1,0), 0);
            view.disableClipPlane(1);
            break;
            
        case 313:
            view.enableClipPlane(0, Vector3(0,+1,0), 1);
            view.enableClipPlane(1, Vector3(0,-1,0), 1);
            break;
 
        case 321:
            view.enableClipPlane(0, Vector3(0,0,+1), 0);
            view.disableClipPlane(1);
            break;
            
        case 322:
            view.enableClipPlane(0, Vector3(0,0,-1), 0);
            view.disableClipPlane(1);
            break;
            
        case 323:
            view.enableClipPlane(0, Vector3(0,0,+1), 1);
            view.enableClipPlane(1, Vector3(0,0,-1), 1);
            break;

        case 324:
            view.enableClipPlane(0, Vector3(0,0,+1), 0.5);
            view.enableClipPlane(1, Vector3(0,0,-1), 0.5);
            break;

        default: ABORT_NOW("unknown menu item");
    }
    glutPostRedisplay();
    buildMenu();
}

//------------------------------------------------------------------------------
//--------------------------------  MOUSE  -------------------------------------
//------------------------------------------------------------------------------
#pragma mark -

void  glApp::actionFunc(void (*func)(const Vector3 &, int))
{
    actionFuncClick = func;
}

void  glApp::actionFunc(void (*func)(Vector3 &, const Vector3 &, int))
{
    actionFuncMotion = func;
}


//------------------------------------------------------------------------------
void glApp::mouseClick(int button, int state, int mx, int my)
{
    View & view = glApp::currentView();
    int winW = view.width(), winH = view.height();    

    //printf("mouse button %i (%4i %4i) state %i key %i\n", button, mx, my, state, specialKeys);

    mouseView = view;
    mouseView.getGLMatrices();
    mouseDown = mouseView.unproject(mx, winH-my, nearZ);

    if ( state == GLUT_UP )
    {
        if ( mouseAction == MOUSE_SET_ROI )
            matchROI(mZoomWindow);
        
        if ( mouseAction == MOUSE_MOVE_ROI )
            mouseAction = MOUSE_SET_ROI;
        /*
         Zooming with the mouse-wheel requires an extended GLUT implementation.
         for example http://iihm.imag.fr/blanch/howtos/MacOSXGLUTMouseWheel.html
         
         The zoom preserves the position pointed at by the mouse.
         */
        real wz = 1.0;
        
        if ( button == 3 )
            wz = 0.992248;
        if ( button == 4 )
            wz = 1.007812;
        
        if ( wz != 1 )
        {
            /*
             in 2D, we do not allow any shift in Z,
             and in 3d, we zoom in on the middle Z-plane
             */
            if ( mDIM == 3 )
                mouseDown = mouseView.unproject(mx, winH-my, midZ);
            else
                mouseDown[2] = 0;
            view.zoom_out(wz);
            view.move_to((1.0-wz)*mouseDown+wz*view.focus);
            glutPostRedisplay();
        }

        glutSetCursor(GLUT_CURSOR_INHERIT);
        return;
    }

    glutSetCursor(GLUT_CURSOR_INFO);
    
    // action is primarily decided by current mode
    mouseAction = userMode;
    specialKeys = glutGetModifiers();
    
    // change the mouse action if the CONTROL is pressed:
    if ( specialKeys & GLUT_ACTIVE_CTRL )
    {
        switch ( mouseAction )
        {
            case MOUSE_MOVE:    mouseAction = (mDIM==2)?MOUSE_SPIN:MOUSE_ROTATE; break;
            case MOUSE_SPIN:    mouseAction = (mDIM==2)?MOUSE_MOVE:MOUSE_MOVE_XZ; break;
            case MOUSE_ZOOM:    mouseAction = MOUSE_MOVE; break;
            case MOUSE_SET_ROI: mouseAction = MOUSE_MOVE; break;
            case MOUSE_ROTATE:  mouseAction = MOUSE_MOVE; break;
            case MOUSE_MOVE_XZ: mouseAction = (mDIM==2)?MOUSE_MOVE:MOUSE_ROTATE;  break;
            default: break;
        }
    }
    
    // change the mouse action because the shift key is down:
    if ( specialKeys & GLUT_ACTIVE_SHIFT )
    {
        mouseAction = MOUSE_ACTIVE;
        specialKeys -= GLUT_ACTIVE_SHIFT;
    }
    
    switch( mouseAction )
    {
        case MOUSE_MOVE:
        {
        } return;
            
            
        case MOUSE_MOVE_XZ:
        {
            Vector3 uc = mouseView.unproject(winW/2.0, winH/2.0, nearZ);
            depthAxis  = ( uc - mouseView.focus ).normalized();
            Vector3 ud = mouseView.unproject(winW/2.0, winH, nearZ);
            mouseAxis  = ( ud - uc ).normalized();
        } break;
            
            
        case MOUSE_ROTATE:
        {
            /* 
            Choose the amplification factor for mouse controlled rotation:
            for a value of one, the rotation exactly follows the mouse pointer 
            */
            const real amplification = 3.0;
            depthAxis  = mouseDown - mouseView.focus;
            depthAxis *= amplification / depthAxis.normSqr();
        } break;
            
            
        case MOUSE_SPIN:
        {
            Vector3 up = mouseDown;
            mouseDown  = mouseView.unproject(winW/2.0, winH/2.0, nearZ);
            mouseAxis  = ( mouseDown - mouseView.focus ).normalized();
            depthAxis  = up - mouseDown;
        } break;
            
            
        case MOUSE_ZOOM:
        {
            real xx = mx - 0.5 * mouseView.width();
            real yy = my - 0.5 * mouseView.height();
            mouseZoomScalar = sqrt( xx*xx + yy*yy );
            if ( mouseZoomScalar > 5 )
                mouseZoomScalar = 1.0 / mouseZoomScalar;
            else
                mouseZoomScalar = 0.2;
        } break;
        
        
        case MOUSE_SET_ROI:
        case MOUSE_MOVE_ROI:
        {
            mouseDown = mouseView.unproject(mx, winH-my, midZ);
            if ( insideROI(mouseDown) )
            {
                ROIdown[0] = ROI[0];
                ROIdown[1] = ROI[1];
                mouseAction = MOUSE_MOVE_ROI;
            }
            if ( mouseAction == MOUSE_SET_ROI )
            {
                setROI(mouseDown);
                flashText("Position = %.3f %.3f", ROI[0].XX, ROI[0].YY);
            }
        } break;
        
            
        case MOUSE_ACTIVE:
        {
            if ( actionFuncClick )
            {
                mouseDown = mouseView.unproject(mx, winH-my, midZ);
                //std::cerr << "Action down at "<<mouseDown<<std::endl;
                actionFuncClick(mouseDown, specialKeys);
            }
        }
        
        case MOUSE_SELECT:
        case MOUSE_PASSIVE:
            return;
    }
    glutPostRedisplay();
}



//------------------------------------------------------------------------------
void glApp::mouseMotion(int mx, int my)
{
    //printf("mouse motion (%i %i) %i\n", mx, my, mouseAction);
    View & view = glApp::currentView();
    int winH = view.height();    

    Vector3 mouse = mouseView.unproject(mx, winH-my, nearZ);

    switch( mouseAction )
    {
        case MOUSE_ROTATE:
        {
            /* we should rotate after: Q <- dQ * sQ, however dQ is defined in the 
            reference frame rotated by sQ already, so dQ = sQ * dM * inv(sQ).
            This leads to the multiplication on the right: Q <- sQ * dM. */
            Quaternion<real> q;
            q.setFromAxis( vecProd(depthAxis, mouse-mouseDown) );
            view.rotate_to( mouseView.rotation * q );
        } break;
        
        
        case MOUSE_SPIN:
        {
            real cos = depthAxis * ( mouse - mouseDown );
            real sin = vecProd( depthAxis, ( mouse - mouseDown ) ) * mouseAxis;
            Quaternion<real> q;
            q.setFromAxis( mouseAxis, atan2( sin, cos ) );
            view.rotate_to( mouseView.rotation * q );
        } break;
        
        
        case MOUSE_MOVE:
        {
            view.move_to( mouseView.focus - ( mouse - mouseDown ) );
        } break;
        
        
        case MOUSE_MOVE_XZ:
        {
            real S = ( mouse - mouseDown ) * mouseAxis;
            Vector3 move = mouse - mouseDown - S * ( depthAxis + mouseAxis );
            view.move_to( mouseView.focus - move );
        } break;
        
        
        case MOUSE_ZOOM:
        {
            real xx = mx - 0.5 * mouseView.width();
            real yy = my - 0.5 * mouseView.height();
            real Z = mouseZoomScalar * sqrt( xx*xx + yy*yy );
            if ( Z > 0.001 ) view.zoom_to( mouseView.zoom * Z );
         } break;
        
        
        case MOUSE_SET_ROI:
        {
            setROI(mouseDown, mouseView.unproject(mx, winH-my, midZ));
            Vector3 d = ROI[1] - ROI[0];
            flashText("ROI %.3fx%.3f diag. %.3f", d.XX, d.YY, d.norm());
        } break;
        
        
        case MOUSE_MOVE_ROI:
        {
            Vector3 d = mouseView.unproject(mx, winH-my, midZ) - mouseDown;
            if ( glutGetWindow() == mZoomWindow )
                d = -d;
            ROI[0] = ROIdown[0] + d;
            ROI[1] = ROIdown[1] + d;
            flashText("ROI moved %.3f %.3f", d.XX, d.YY);
            matchROI(mZoomWindow);
        } break;
            
        
        case MOUSE_ACTIVE:
        {
            if ( actionFuncMotion )
            {
                mouse = mouseView.unproject(mx, winH-my, midZ);
                //std::cerr << "Action move at " << mouse << std::endl;
                actionFuncMotion(mouseDown, mouse, specialKeys);
            }
        } break;
        
        
        case MOUSE_SELECT:
        case MOUSE_PASSIVE: 
            break;
    }
    glutPostRedisplay();
}

//------------------------------------------------------------------------------
void glApp::passiveMouseMotion(int mx, int my)
{
    //printf("passive mouse (%i %i)\n", mx, my);
    int x = glutGet(GLUT_WINDOW_WIDTH)-8;
    int y = glutGet(GLUT_WINDOW_HEIGHT)-8;
    bool s = (abs(mx-x) < 50) && (abs(my-y) < 50);
    if ( s != showResizeBox )
    {
        showResizeBox = s;
        glutPostRedisplay();
    }
}

//------------------------------------------------------------------------------
#pragma mark -

void glApp::displayLabel(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char tmp[1024];
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);
    
    GP.message_left = GP.label + tmp;
}

void glApp::flashText(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char tmp[1024];
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);

    flash = tmp;
    if ( flash_count == 0 )
        glutTimerFunc(100, flashTimer, 1);
    flash_count = 50;
    
    if ( views.size() > 1  &&  views[1].window()==1 )
        glutPostWindowRedisplay(1);
    else
        ;//std::cerr << tmp << std::endl;
}

void glApp::flashTimer(int win)
{
    //printf("flash %i, cnt %i at %lu\n", flash, cnt, clock());
    
    if ( --flash_count > 0 )
    {
        //continue count-down:
        glutTimerFunc(100, flashTimer, win);
    }
    else
    {
        //count-down completed:
        flash = "";
        //restore the front buffer image:
        glutPostRedisplay();
    }
}


//------------------------------------------------------------------------------
//draw the Region of interest
void glApp::drawROI(Vector3 roi[2])
{
    glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT);
    glEnable(GL_LINE_STIPPLE);
    glEnable(GL_COLOR_LOGIC_OP);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_LIGHTING);
    glLogicOp(GL_XOR);
    glLineStipple(1, 0x0f0f);
    glBegin(GL_LINE_LOOP);
    glVertex3d(roi[0].XX, roi[0].YY, roi[0].ZZ);
    glVertex3d(roi[1].XX, roi[0].YY, roi[0].ZZ);
    glVertex3d(roi[1].XX, roi[1].YY, roi[0].ZZ);
    glVertex3d(roi[0].XX, roi[1].YY, roi[0].ZZ);
    glEnd();
    if ( mDIM == 3 ) {
        glBegin(GL_LINE_LOOP);
        glVertex3d(roi[0].XX, roi[0].YY, roi[1].ZZ);
        glVertex3d(roi[1].XX, roi[0].YY, roi[1].ZZ);
        glVertex3d(roi[1].XX, roi[1].YY, roi[1].ZZ);
        glVertex3d(roi[0].XX, roi[1].YY, roi[1].ZZ);
        glEnd();
    }
    glPopAttrib();
}

//------------------------------------------------------------------------------
#pragma mark -

void glApp::setDisplay(View const& view)
{
    view.setModelView();
    view.setFog();
    view.setLights();
    view.setClipPlanes();
}


void glApp::endDisplay(View const& view, bool interactive)
{
    view.endClipPlanes();

    if ( view.show_axes )
        gleDrawAxes(view.scale_bar/10, mDIM);
    
    //display over-the-window features:
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    if ( view.show_scale )
    {
        glColor3f(1.0, 1.0, 0.5);
        view.displayScaleBar(view.show_scale == 2, view.scale_bar);
    }
    
    if ( GP.message_left.size() )
    {
        glColor3f(1.0, 1.0, 1.0);
        std::string msg = GP.message_left;
        // in non-interactive mode, only print first line:
        if ( !interactive )
            msg = msg.substr(0, msg.find('\n'));
        gleDisplayText(msg.c_str(), GLUT_BITMAP_8_BY_13, 0x0, 0, view.width(), view.height());
    }

    if ( interactive )
    {
        if ( GP.show_fps )
        {
            static char buf[16] = "\0";
            static int nb_frames = 0;
            static long msec = TicToc::milli_seconds_today();
            ++nb_frames;
            long now = TicToc::milli_seconds_today();
            if ( now > msec + 1000 )
            {
                double fps = nb_frames * 1000.0 / ( now - msec );
                snprintf(buf, sizeof(buf), "%3.2f fps", fps);
                msec = now;
                nb_frames = 0;
            }
            glColor3f(1,1,1);
            gleDisplayText(buf, GLUT_BITMAP_8_BY_13, 0x0, 3, view.width(), view.height());
        }
        
        if ( fKeyString[0] != '\0' )
        {
            glColor3f(1,0,1);
            gleDisplayText(fKeyString, GLUT_BITMAP_HELVETICA_18, 0x0, 3, view.width(), view.height());
        }
        
        if ( flash.size() )
        {
            glColor3f(0.6,0.6,1);
            gleDisplayText(flash.c_str(), GLUT_BITMAP_9_BY_15, 0x0, 2, view.width(), view.height());
        }
        
        if ( GP.show_message  &&  GP.message.size() )
        {
            glColor3f(1,1,1);
            gleDisplayText(GP.message.c_str(), GLUT_BITMAP_8_BY_13, 0x000000CC, 4, view.width(), view.height());
        }
        
        if ( GP.message_right.size() )
        {
            glColor3f(1,1,1);
            gleDisplayText(GP.message_right.c_str(), GLUT_BITMAP_8_BY_13, 0x0, 3, view.width(), view.height());
        }
        
        if ( mouseAction == MOUSE_SET_ROI  ||  mouseAction == MOUSE_MOVE_ROI )
        {
            //draw the Region of interest
            glLineWidth(1);
            glColor3f(1,1,0);
            drawROI(ROI);
        }
    }

#ifdef __APPLE__
    //display the window resize handle, if the mouse pointer is close enough
    if ( showResizeBox && ! GP.full_screen )
        gleDrawResizeBox(0xFFFFFFAA, view.width(), view.height());
#endif
    
    glPopAttrib();
}

//------------------------------------------------------------------------------
#pragma mark -

void glApp::dummyDisplay()
{
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(0.0, 0.0, 1.0);
    gleDrawText("Maximum number of windows exceeded", GLUT_BITMAP_8_BY_13);

    if ( GP.buffered )
        glutSwapBuffers();
    else
        glFlush();        
}


/**
 This is used for any secondary window.
 It does not show the interactive feedback to user.
 */
void glApp::plainDisplay()
{
    View & view = glApp::currentView();
    setDisplay(view);
    displayCallback();
    endDisplay(view, false);
    
    if ( GP.buffered )
        glutSwapBuffers();
    else
        glFlush();
}

/**
 This is used for the main window
 */
void glApp::mainDisplay()
{
    View & view = glApp::currentView();
    setDisplay(view);
    displayCallback();
    endDisplay(view, true);
    
    if ( GP.buffered )
        glutSwapBuffers();
    else
        glFlush();
}


void glApp::postRedisplay()
{
    for ( unsigned n = 1; n < views.size(); ++n )
        if ( views[n].window() > 0 )
            glutPostWindowRedisplay(n);
}

