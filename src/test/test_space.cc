// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Francois Nedelec, nedelec@embl.de, started in October 2002
// test_space provides a visual test of Cytosim's Space

#include <ctime>
#include "dim.h"
#include "exceptions.h"
#include "glossary.h"
#include "real.h"
#include "smath.h"
#include "vector.h"
#include "random.h"
extern Random RNG;

#include "space_prop.h"
#include "space.h"
#include "space_set.h"
#include "glapp.h"
#include "gle.h"
using namespace gle;

// property
SpaceProp prop("test_space");

// Space to be tested:
Space * spc = 0;

// number of points
const int maxpts = 65536;
int nbpts  = 1024;
int scan   = 100;

// INFLATION of the rectangle containing point to be projected
const real INFLATION = 2;

// regular or random distribution of the test-points
bool regular_distribution = false;


//coordinates of the points:
Vector point[maxpts];

//true if inside
int    inside[maxpts];

//coordinates of the projections
Vector project[maxpts];

//coordinates of the projections of the projections
Vector project2[maxpts];

//normals to the projections
Vector normal[maxpts];

//located on the edges
Vector edge[maxpts];

//max distance from projection to second projection
real  max_error_projection;

//slicing parameters
bool slicing = false;
const real sliceStep = 0.2;
real thickness = sliceStep;

Vector slicePos(0, 0, 0);
Vector sliceDir(1, 0, 0);

//show or hide points in or outside
int showInside    = true;
int showOutside   = true;
int showProject   = true;
int showReproject = true;
int showNormals   = false;
int showEdges     = false;

//use timer function on or off
int timerOn = false;
int timerDelay = 50;

//display parameter for OpenGL
GLfloat line_width = 0.5;

//amount of white added to colors
const GLfloat COL = 0.8;

//------------------------------------------------------------------------------

void generatePoints()
{
    Vector range = spc->extension() + INFLATION * Vector(1, 1, 1);
    
    if ( regular_distribution )
    {
        int kk = 0;
        nbpts = 0;
        //follow a regular lattice:
        for ( int ii = -scan; ii <= scan; ++ii )
            for ( int jj = -scan; jj <= scan; ++jj )
#if ( DIM == 3 )
                for ( kk = -scan; kk <= scan; ++kk )
#endif
                {
                    point[nbpts++] = range.e_mul(Vector(ii, jj, kk) / scan);
                    if ( nbpts >= maxpts )
                        return;
                }
    }
    else
    {
        for ( int ii = 0; ii <= nbpts; ++ii )
            point[ii] = range.e_mul(Vector::randBox());
        //point[ii] = Vector::randUnit(1);
        //point[ii] = spc->randomPlaceNearEdge(0.1);
    }
}


void distributePoints()
{
    if ( spc == 0 ) return;
    
    generatePoints();
    max_error_projection = 0;
    
    for ( int ii = 0; ii < nbpts; ++ii )
    {
        //see if space finds it inside:
        inside[ii] = spc->inside( point[ii] );
        //calculate the projection:
        spc->project( point[ii], project[ii] );
        
        //calculate the projection of the projection:
        //spc->project( project[ii], project2[ii] );
        project2[ii] = project[ii];
        
        if ( showNormals )
            normal[ii] = spc->normalToEdge(project[ii]);
        else
            normal[ii].set(0,0,0);
        
        edge[ii] = spc->randomPlaceOnEdge(1);
        
        real d = (project[ii] - project2[ii]).normSqr();
        if ( d > max_error_projection ) max_error_projection = d;
    }
    max_error_projection = sqrt( max_error_projection );
    
    glApp::displayLabel(" error=%5.2e (press s)", max_error_projection);
}

//------------------------------------------------------------------------------
void timerFunction(int)
{
    if ( timerOn )
    {
        distributePoints();
        glutPostRedisplay();
        glutTimerFunc(timerDelay, timerFunction, 0);
    }
}

//------------------------------------------------------------------------------
void setSpace()
{
    std::cerr << "Space:geometry=" << prop.geometry << "\n";
    
    try {
        prop.complete(0, 0);
        if ( spc )
        {
            delete(spc);
            spc = 0;
        }
        spc = prop.newSpace();
    }
    catch( Exception & e )
    {
        printf("Error: `%s'\n", e.what());
    }
    
    try {
        if ( spc )
            distributePoints();
    }
    catch( Exception & e )
    {
        printf("Error: `%s'\n", e.what());
    }

    glutPostRedisplay();
}

//------------------------------------------------------------------------------
enum MENUS_ID {
    MENU_QUIT = 102, MENU_RESETVIEW = 103,
    MENU_INSIDE = 104, MENU_OUTSIDE = 105, MENU_PROJECT = 106,
    MENU_XSLICING = 107, MENU_YSLICING = 108, MENU_ZSLICING = 109,
    MENU_EDGES = 111
};

void processMenu(int item)
{
    switch( item )
    {
        case MENU_QUIT:
            exit(EXIT_SUCCESS);
        case MENU_RESETVIEW:
            glApp::resetView();
            break;
        case MENU_INSIDE:
            showInside = ! showInside;
            break;
        case MENU_OUTSIDE:
            showOutside = ! showOutside;
            break;
        case MENU_EDGES:
            showEdges = ! showEdges;
            break;
        case MENU_PROJECT:
            showProject = ! showProject;
            break;
        case MENU_XSLICING:
            slicing = !slicing;
            sliceDir.set(1,0,0);
            break;
        case MENU_YSLICING:
            slicing = !slicing;
            sliceDir.set(0,1,0);
            break;
        case MENU_ZSLICING:
            slicing = !slicing;
            sliceDir.set(0,0,1);
            break;
    }
    glutPostRedisplay();
}


void initMenus()
{
    int gm = glApp::buildMenu();
    glutCreateMenu(processMenu);
    glutAddSubMenu("glApp", gm);
    
    glutAddMenuEntry("Reset",                MENU_RESETVIEW);
    glutAddMenuEntry("Quit",                 MENU_QUIT);
    glutAddMenuEntry("-", 0);
    glutAddMenuEntry("Toggle inside  (i)",   MENU_INSIDE);
    glutAddMenuEntry("Toggle outside (o)",   MENU_OUTSIDE);
    glutAddMenuEntry("Toggle edges   (e)",   MENU_EDGES);
    glutAddMenuEntry("Toggle project (p)",   MENU_PROJECT);
    
    glutAddMenuEntry("Toggle x-slicing (x)", MENU_XSLICING);
    glutAddMenuEntry("Toggle y-slicing (y)", MENU_YSLICING);
    glutAddMenuEntry("Toggle z-slicing (z)", MENU_ZSLICING);
    
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//------------------------------------------------------------------------------
void processSpecialKey(int key, int x=0, int y=0)
{
    switch (key)
    {
        case GLUT_KEY_LEFT:
            slicePos.XX -= 0.2;
            break;
        case GLUT_KEY_RIGHT:
            slicePos.XX += 0.2;
            break;
        case GLUT_KEY_UP:
            thickness += sliceStep;
            break;
        case GLUT_KEY_DOWN:
            thickness -= sliceStep;
            if ( thickness < sliceStep ) thickness = sliceStep;
            break;
        default:
            break;
    }
    
    glutPostRedisplay();
}

void processNormalKey(unsigned char c, int x=0, int y=0)
{
    switch (c)
    {
        case 27:
        case 'q':
            exit(EXIT_SUCCESS);
            
        case ' ':
            distributePoints();
            break;
            
        case '0':
            glApp::resetView();
            break;
            
        case ']':
            scan *= 2;
            nbpts *= 2;
            if ( nbpts > maxpts )
                nbpts = maxpts;
            distributePoints();
            break;
            
        case '[':
            if ( scan > 2 ) scan /= 2;
            if ( nbpts > 2 ) nbpts /= 2;
            distributePoints();
            break;
            
        case 'x':
            slicing = !slicing;
            sliceDir.set(1,0,0);
            break;
            
        case 'y':
            slicing = !slicing;
            sliceDir.set(0,1,0);
            break;
            
        case 'z':
            slicing = !slicing;
            sliceDir.set(0,0,1);
            break;
            
        case 'i':
            showInside = ! showInside;
            break;
            
        case 'o':
            showOutside = ! showOutside;
            break;
            
        case 'r':
            showReproject = ! showReproject;
            break;
            
        case 'p':
            showProject = ! showProject;
            break;
            
        case 'e':
            showEdges = ! showEdges;
            break;
            
        case 'n':
            showNormals = ! showNormals;
            if ( showNormals)
                distributePoints();
            break;
            
        case 'R':
            regular_distribution = !regular_distribution;
            distributePoints();
            break;
            
        case 't':
            timerOn = ! timerOn;
            if ( timerOn )
                glutTimerFunc(timerDelay, timerFunction, 0);
            break;
            
        default:
            glApp::processNormalKey(c,x,y);
    }
    
    glutPostRedisplay();
}

//------------------------------------------------------------------------------

bool showPoint(int i)
{
    if ( inside[i] )
    {
        if ( ! showInside )
            return false;
    }
    else
    {
        if ( ! showOutside )
            return false;
    }
    
    if ( ! slicing )
        return true;
    
    Vector & pos = project[i];
    
    return (( (slicing & 0x01) &&  fabs(pos.XX-slicePos.XX) < thickness )
#if ( DIM > 1 )
            || ( (slicing & 0x02) &&  fabs(pos.YY-slicePos.YY) < thickness )
#endif
#if ( DIM > 2 )
            || ( (slicing & 0x04) &&  fabs(pos.ZZ-slicePos.ZZ) < thickness )
#endif
            );
}


void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    //plot a gren dot for points inside, a red dot for point outside:
    glPointSize(2.0);
    glBegin(GL_POINTS);
    for ( int ii = 0; ii < nbpts; ++ii )
    {
        if ( showPoint(ii) )
        {
            if ( inside[ii] )
                glColor3f( 0.0, COL, 0.0 );
            else
                glColor3f( 0.0, 0.0, COL );
            gleVertex( point[ii] );
        }
    }
    glEnd();
    
    if ( showProject )
    {
        //plot a blue line from the point to its projection:
        glLineWidth(line_width);
        glBegin(GL_LINES);
        for ( int ii = 0; ii < nbpts; ++ii )
        {
            if ( showPoint(ii) )
            {
                if ( inside[ii] )
                    glColor3f( 0.0, COL, 0.0 );
                else
                    glColor3f( 0.0, 0.0, COL );

                gleVertex( point[ii] );
                gleVertex( project[ii] );
            }
        }
        glEnd();
    }
    
    if ( showNormals )
    {
        glLineWidth(line_width);
        glBegin(GL_LINES);
        for ( int ii = 0; ii < nbpts; ++ii )
        {
            glColor4f( 1.0, 1.0, 1.0, 1.0 );
            gleVertex( project[ii] );
            glColor4f( 1.0, 1.0, 1.0, 0.0 );
            gleVertex( project[ii] + normal[ii] );
        }
        glEnd();
    }
    
    if ( showReproject )
    {
        glLineWidth(2*line_width);
        glBegin(GL_LINES);
        for ( int ii = 0; ii < nbpts; ++ii )
        {
            if ( showPoint(ii) )
            {
                glColor3f( COL, 0.0, 0.0 );
                gleVertex( project[ii] );
                gleVertex( project2[ii] );
            }
        }
        glEnd();
    }
    
    if ( showEdges )
    {
        glPointSize(2.0);
        glBegin(GL_POINTS);
        glColor3f( 1.0, COL, COL );
        for ( int ii = 0; ii < nbpts; ++ii )
            gleVertex( edge[ii] );
        glEnd();
        glBegin(GL_POINTS);
        glColor3f( 0.0, COL, 0.0 );
        for ( int ii = 0; ii < nbpts; ++ii )
            gleVertex( project[ii] );
        glEnd();
    }
}

void checkVolume()
{
    real ev = spc->estimateVolume(1000000);
    real v = spc->volume();
    
    real err = fabs( ev - v ) / v;
    
    if ( err > 1e-3 )
    {
        printf("Analytical volume = %f\n", v);
        printf("   difference 2 methods = %.6f %%\n", 100*err);
    }
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glApp::init(display, DIM);
    glApp::setScale(10);
    
    initMenus();
    if ( argc > 1 )
    {
        prop.geometry = argv[1];
        setSpace();
    }
    
    if ( ! spc )
    {
        printf("A geometry should be given in the command line, for example:\n");
        printf("    test_space 'capsule 1 2'\n");
        exit(EXIT_SUCCESS);
    }

    for ( int n = 0; n < 3; ++n )
        checkVolume();
    
    glutKeyboardFunc(processNormalKey);
    glutSpecialFunc(processSpecialKey);
    
    glutMainLoop();
    
    return EXIT_SUCCESS;
}


