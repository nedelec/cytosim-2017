// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/*
Simple GLUT application
 
To compile on mac OSX:
g++ test_glut.cc -framework GLUT -framework OpenGL

To compile on Windows using Cygwin
g++ test_glut.cc -lopengl32 -lglut32

To compile on Linux
g++ test_glut.cc -lglut -lGL

*/

#include <cstdlib>
#include <cstdio>
#include <cmath>

#ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#else
  #include <GL/glew.h>
  #include <GL/glut.h>    //use this on Linux & PC
#endif

///------------ state of the simulated point
//size of point drawn:
int pointSize = 1;


//------------- precision: real is an alias for double or float:

typedef double real;

//------------- size of the display window:

GLint windowSize[2] = { 800, 800 };

real pixelSize = 1;


//------------- delay for the Timer function in milli-seconds:

int timerDelay = 50;

//------------- user adjustable zoom:

real zoom = 0.8;

//------------- point of focus:

real focus[] = { 0.0, 0.0 };

//------------- variables for the mouse driven zoom:

int mouseAction;
real zoomSave, zoomFactor;
real focusSave[]       = { 0.0, 0.0 };
GLdouble mouseClick[]  = { 0.0, 0.0, 0.0 };
GLdouble unprojected[] = { 0.0, 0.0, 0.0 };

//------------- function to set the OpenGL transformation:

void setModelView()
{
    glMatrixMode(GL_MODELVIEW);
    
    //set the matrix with a simple zoom:
    glLoadIdentity();
    glScaled(zoom, zoom, zoom);
    glTranslated(-focus[0], -focus[1], 0);
}


void unproject(const int wx, const int wy, real res[2])
{
    res[0] = ( wx - 0.5 * windowSize[0] ) * pixelSize + focus[0];
    res[1] = ( 0.5 * windowSize[1] - wy ) * pixelSize + focus[1];
}

void windowReshaped(int w, int h)
{
    glViewport(0, 0, w, h);
    windowSize[0] = w;
    windowSize[1] = h;
        
    // --- set-up the projection matrix:
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    if ( w > h )
    {
        real ratio = h / (real) w;
        glOrtho(-1.0, 1.0, -ratio, ratio, 0, 1 );
        pixelSize = 2.0 / ( zoom * windowSize[0] );
    }
    else
    {
        real ratio = w / (real) h;
        glOrtho(-ratio, ratio, -1.0, 1.0, 0, 1 );
        pixelSize = 2.0 / ( zoom * windowSize[1] );
    }
}

//----------------------------- KEYS --------------------------------

void processNormalKey(unsigned char c, int mouseX, int mouseY)
{
    switch (c)
    {
        case 27:
        case 'q':
            exit(EXIT_SUCCESS);
            
        case ' ':
            zoom = 1;
            break;   //we use break to call the setModelView() below
            
        default:
            printf("hit key `%c' (ascii %i) with modifier %i\n", c, c, glutGetModifiers());
    }
    setModelView();
    glutPostRedisplay();
}


// handle special keys: arrows, ctrl, etc.
void processInputKey(int c, int mouseX, int mouseY)
{
    printf("unknown special key %c\n", c);
}


//----------------------------- MENUS --------------------------------

enum MENUS_ID { MENU_QUIT, MENU_RESETVIEW };

void processMenu(int item)
{
    switch ( item )
    {
        case MENU_QUIT:
            exit(EXIT_SUCCESS);
        case MENU_RESETVIEW:
            zoom = 1;
            focus[0] = 0;
            focus[1] = 1;
            setModelView();
            glutPostRedisplay();
            break;
    }
}

void initMenus()
{
    // --- one menu with two entries:
    glutCreateMenu(processMenu);
    glutAddMenuEntry("Reset", MENU_RESETVIEW);
    glutAddMenuEntry("Quit", MENU_QUIT);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//----------------------------- MOUSE -------------------------------

//----------different actions controled by the mouse
enum { MOUSE_PASSIVE, MOUSE_ZOOM, MOUSE_MOVE, MOUSE_CLICK };

void processMouse(int button, int state, int x, int y)
//this is called when the mouse button is pressed or released:
{
    // for a button release event, do nothing:
    if ( state != GLUT_DOWN ) return;
    
    mouseAction = MOUSE_PASSIVE;
    
    // action depends on mouse button:
    switch( button )
    {
        case GLUT_LEFT_BUTTON:
        {
            if ( glutGetModifiers() & GLUT_ACTIVE_SHIFT )
                mouseAction = MOUSE_CLICK;
            else
                mouseAction = MOUSE_MOVE;
        } break;
        case GLUT_MIDDLE_BUTTON:
            mouseAction = MOUSE_ZOOM;
            break;
    }

    // perform action...
    switch( mouseAction )
    {
        case MOUSE_MOVE:
        {
            unproject(x, y, unprojected);
            focusSave[0] = focus[0];
            focusSave[1] = focus[1];
        } break;
            
        case MOUSE_ZOOM:
        {
            real xx = x - 0.5*windowSize[0];
            real yy = y - 0.5*windowSize[1];
            zoomFactor = sqrt( xx*xx + yy*yy );
            if ( zoomFactor > 0 )
                zoomFactor = 1.0 / zoomFactor;
            zoomSave = zoom;
        } break;
            
        case MOUSE_CLICK:
        {
            unproject(x, y, mouseClick);
        } break;
            
        case MOUSE_PASSIVE:
            return;
    }
}


void processMotion(int x, int y)
{
    switch( mouseAction )
    {
        case MOUSE_MOVE:
        {
            GLdouble up[3];
            unproject(x,y, up);
            focus[0] = focusSave[0] + unprojected[0] - up[0];
            focus[1] = focusSave[1] + unprojected[1] - up[1];
        } break;
            
        case MOUSE_ZOOM:
        {
            // --- we set the zoom from how far the mouse is from the window center
            real xx = x - 0.5*windowSize[0];
            real yy = y - 0.5*windowSize[1];
            real Z = zoomFactor * sqrt( xx*xx + yy*yy );
            if ( Z <= 0 ) return;
            zoom = zoomSave * Z;
        } break;
        
        case MOUSE_CLICK:
        {
            unproject(x,y, mouseClick);
        } break;
                        
        case MOUSE_PASSIVE:
            return;
    }
    setModelView();
    glutPostRedisplay();
}

//----------------------------- DISPLAY --------------------------------

void display()
{
    // --- clear window to the current clearing color:
    glClear( GL_COLOR_BUFFER_BIT );
    
    // --- set line width to 1 and color to white:
    glColor3f(1.0, 1.0, 1.0);
    glLineWidth(1.0);
    
    // Display of the simulation state:
    // --- draw a wireframe triangle:
    glBegin(GL_LINE_LOOP);
    glVertex2f( 1.0, -1.0);
    glVertex2f(-1.0, -1.0);
    glVertex2f( 0.0,  1.0);
    glEnd();
    
    // --- a point of variable size at the last mouse click:
    glColor3f(1.0, 1.0, 0.0);
    glPointSize(pointSize);
    glBegin(GL_POINTS);
    glVertex2d(mouseClick[0], mouseClick[1]);
    glEnd();
    
    // OpenGL cleanup:
    glutSwapBuffers();
    // --- check for OpenGL errors:
    glutReportErrors();
}

//------------------------- TIMER FUNCTION -----------------------------
void timerFunction(int value)
{
    //This is a very basic simulation!
    pointSize = 1 + ( pointSize + 1 ) % 16;
    
    
    glutPostRedisplay();
    //register another automatic timer call back (timerDelay is in milli-sec):
    glutTimerFunc(timerDelay, timerFunction, 1);
}


//----------------------------- INIT GL --------------------------------
void initGL()
{
    // --- choose the clearing color: black
    glClearColor(0.0, 0.0, 0.0, 0.0);
    
    //--- hints for OpenGL rendering:
    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    initMenus();
    setModelView();
}


//-----------------------------   MAIN  --------------------------------
int main(int argc, char* argv[])
{
    
    // --- initialization of GLUT:
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );
    glutInitWindowSize(windowSize[0], windowSize[1]);
    glutInitWindowPosition(50, 50);
    glutCreateWindow(argv[0]);
    
    // --- further initialization of OpenGL:
    initGL();
    
    // --- register all the necessary functions:
    glutDisplayFunc(display);
    glutReshapeFunc(windowReshaped);
    glutMouseFunc(processMouse);
    glutMotionFunc(processMotion);
    glutSpecialFunc(processInputKey);
    glutKeyboardFunc(processNormalKey);
    glutTimerFunc(50, timerFunction, 0);
    
    // --- starts the event loop, which will never return:
    glutMainLoop();
    
    return EXIT_SUCCESS;
}
