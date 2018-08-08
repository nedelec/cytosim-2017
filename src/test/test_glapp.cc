// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
//------------------------------------------------------------------------------
//                        a test for glApp
//      mouse driven zoom and rotation with Quaternions and GLU unproject
//      Francois Nedelec nedelec@embl.de,  Oct. 2002, modified Jan. 2006 

#include "glossary.h"
#include "glapp.h"
#include "gle.h"

Vector3 origin(0,0,0), position(0,0,0);

//------------------------------------------------------------------------------
void processNormalKey(unsigned char c, int x, int y)
{
    switch (c)
    {
        case 'q':
            exit(1);

        default:
            glApp::processNormalKey(c,x,y);
            return;
    }
    
    glApp::postRedisplay();
}

//------------------------------------------------------------------------------
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(1.0, 1.0, 1.0);
    glLineWidth(3.0);
    glutWireCube(2.0);
    
    glColor4f(0.0, 0.25, 0.0, 0.5);
    glDepthMask(GL_FALSE);
    glLineWidth(1);
    glutSolidCube(2);
    glDepthMask(GL_TRUE);
    
    glPointSize(16.0);
    glBegin(GL_POINTS);
    glColor3f(1.0, 1.0, 1.0);
    glVertex3d( origin.XX, origin.YY, origin.ZZ );
    glColor3f(0.0, 1.0, 0.0);
    glVertex3d( position.XX, position.YY, position.ZZ );
    glEnd();
    
    glPointSize(7.0);
    glBegin(GL_POINTS);
    glColor3f(1.0, 0.0, 1.0);
    glVertex3f(0, 0, 0);
    glEnd();
    
    // double-pass rendering of a transparent icosahedron:
    glEnable(GL_LIGHTING);
    glColor4f(1.0, 0.0, 1.0, 0.75);
    glDepthMask(GL_FALSE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    gle::gleIcosahedron1();
    glCullFace(GL_BACK);
    gle::gleIcosahedron1();
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_LIGHTING);
}

//------------------------------------------------------------------------------
///set callback for shift-click, with unprojected click position
void  processMouseAction(const Vector3 & a, int)
{
    origin = a;
    glApp::postRedisplay();
}

///set callback for shift-drag, with unprojected mouse and click positions 
void  processMouseActionMotion(Vector3 & a, const Vector3 & b, int)
{
    origin   = a;
    position = b;
    glApp::postRedisplay();
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glApp::init(display, 3);

    glApp::actionFunc(processMouseAction);
    glApp::actionFunc(processMouseActionMotion);
    glApp::attachMenu(GLUT_RIGHT_BUTTON);
    glApp::setScale(2);
    glutKeyboardFunc(processNormalKey);

    glutMainLoop();
    return EXIT_SUCCESS;
}
