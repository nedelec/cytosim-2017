// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Visual test for the rasterizer used in attachment algorithm of Cytosim
// Francois Nedelec, nedelec@embl.de, October 2002

#include "dim.h"
#include <ctime>
#include "vector.h"
#include "glapp.h"
#include "real.h"
#include "smath.h"
#include "random.h"
#include "rasterizer.h"

extern Random RNG;

//===================================================================

real radius  = 5;
real shift[] = {0, 0, 0};
real delta[] = {1, 1, 1};


const int size = 20;

const unsigned int MAX = 16;
unsigned int nbpts = 2;
real pts[3*MAX] = {1,1,1,0,0,0};

#if ( DIM == 3 )
int hit[2*size+1][2*size+1][2*size+1];
#else
int hit[2*size+1][2*size+1];
#endif

void newPoints()
{
    for ( unsigned int i = 0; i < 3*MAX ; ++i )
        pts[i] = (size-1) * RNG.sreal();
}

void clearHits()
{
    for ( int i = 0; i <= 2*size; ++i )
        for ( int j = 0; j <= 2*size; ++j )
#if ( DIM == 3 )
            for ( int k = 0; k <= 2*size; ++k )
                hit[i][j][k] = 0;
#else
            hit[i][j] = 0;
#endif
}

//-------------------------------------------------------------------

void paintHit(int x_inf, int x_sup, int y, int z, void*, void*)
{
    if ( x_sup < x_inf )
        printf("mixedup order x = %i %i at y = %i\n", x_inf, x_sup, y);
    
    if ( y < -size ||  y > size ) return;
    if ( z < -size ||  z > size ) return;
    
    for ( int x = x_inf; x <= x_sup; ++x )
    {
        if ( -size <= x && x <= size )
        {
#if ( DIM == 3 )
            ++hit[x+size][y+size][z+size];
#else
            ++hit[x+size][y+size];
#endif
        }
    }
}

//-------------------------------------------------------------------

void paintDraw(int x_inf, int x_sup, int y, int z, void*, void*)
{
    glPointSize(4.0);
    glBegin(GL_POINTS);
    glColor4f(1.0, 1.0, 1.0, 0.5);
    glVertex3i( x_inf, y, z );
    glVertex3i( x_sup, y, z );
    glEnd();
    
    glLineWidth(1);
    glBegin(GL_LINES);
    glColor4f(1.0, 1.0, 1.0, 0.5);
    glVertex3i( x_inf, y, z );
    glVertex3i( x_sup, y, z );
    glEnd();
}

//-------------------------------------------------------------------



bool inCylinder(Vector const& p, Vector const& q, Vector const& x)
{
    const Vector pq = q - p;
    const real pqn = pq.norm();
    real abs = pq * (x-p) / pqn;
    if ( abs <   0-radius ) return false;
    if ( abs > pqn+radius ) return false;
    abs /= pqn;
    if ( abs < 0 ) abs = 0;
    if ( abs > 1 ) abs = 1;
    return ( x-p-pq*abs ).normSqr() <= radius * radius;
}


bool check_point(Vector const& p, Vector const& q, int i, int j, int k)
{
    int in = inCylinder(p,q,Vector(i,j,k));
    
#if ( DIM == 3 )
    int ht = hit[i+size][j+size][k+size];
#else
    int ht = hit[i+size][j+size];
#endif
    
    if ( ht != in )
    {
        glPointSize(10.0);
        glBegin(GL_POINTS);
        if ( ht == 1 )
            glColor3f(0.0, 0.0, 1.0);
        else
            glColor3f(1.0, 0.0, 0.0);
#if ( DIM == 3 )
        glVertex3i(i, j, k);
#else
        glVertex2i(i, j);
#endif
        glEnd();
    }
    
    return ( ht != in );
}


#if ( DIM == 3 )

bool realTest()
{
    bool res = false;
    clearHits();    
    
    // use rasterizer to set hit[][]
    //Rasterizer::paintHexLine3D(paintHit, 0, 0, pts, pts+DIM, radius, shift, delta);
    Rasterizer::paintFatLine3D(paintHit, 0, 0, pts, pts+DIM, radius, shift, delta);

    // compare with expected
    Vector p(pts[0], pts[1], pts[2]), q(pts[DIM], pts[DIM+1], pts[DIM+2]);
    
    for ( int i = -size; i <= size; ++i )
        for ( int j = -size; j <= size; ++j )
            for ( int k = -size; k <= size; ++k )
                res |= check_point(p, q, i, j, k);
    return res;
}

#else

bool realTest()
{
    bool res = false;
    clearHits();
    
    // use rasterizer to set hit[][]
    Rasterizer::paintFatLine2D(paintHit, 0, 0, pts, pts+DIM, radius, shift, delta);
    
    // compare with expected
    Vector p(pts[0], pts[1], pts[2]), q(pts[DIM], pts[DIM+1], pts[DIM+2]);
    
    for ( int i = -size; i <= size; ++i )
        for ( int j = -size; j <= size; ++j )
            res |= check_point(p, q, i, j, 0);

    return res;
}

#endif

void display();



void manyTest()
{
    do {
        newPoints();
        display();
        glutSwapBuffers();
    } while ( ! realTest() );
}

//===================================================================


void processNormalKey(unsigned char c, int x=0, int y=0)
{
    switch (c) {
        case 27:
        case 'q':
            exit(EXIT_SUCCESS);
        case ' ':
            newPoints();
            break;
        case '0':
            glApp::resetView();
            break;
        case 'p': if ( nbpts+1 < MAX ) ++nbpts; break;
        case 'o': if ( nbpts > 2 ) --nbpts; break;
        case 'r':
            manyTest();
            break;
        default:
            glApp::processNormalKey(c);
            printf("keyboard commands:\n"
                   " space : draw a new random distribution\n"
                   " p     : increase number of points\n"
                   " o     : decrease number of points\n"
                   " r     : perform many tests as fast as possible\n");
    }
    glApp::postRedisplay();
}

//===================================================================



void display2D()
{
    glDisable(GL_DEPTH_TEST);
#if ( 1 )
    //--------------draw points on the grid:
    glPointSize(10);
    glBegin(GL_POINTS);
    glColor3f(0.15, 0.15, 0.15);
    for ( int i = -size; i <= size; i += 1)
        for ( int j = -size; j <= size; j += 1)
                glVertex2i(i, j);
    glEnd();
    
    //--------------draw a grid in gray:
    glLineWidth(0.5);
    glBegin(GL_LINES);
    glColor3f(1, 1, 1);
    for ( int i = -size; i <= size; i += 5)
    {
        glVertex2i(i, -size);
        glVertex2i(i, +size);
        glVertex2i(-size, i);
        glVertex2i(+size, i);
    }
    glEnd();
#endif   
    
    glPointSize(10);
    glBegin(GL_POINTS);
    glColor3f(0, 0, 1.0);
    for ( unsigned int i = 0; i < nbpts ; ++i )
        glVertex2d(pts[2*i], pts[2*i+1]);
    glEnd();

    if ( nbpts == 2 )
    {
        realTest();
        Rasterizer::paintFatLine2D(paintDraw, 0, 0, pts, pts+DIM, radius, shift, delta);
    }
    else
    {
        glLineWidth(1);
        unsigned int nb = Rasterizer::convexHull2D(nbpts, pts);
        Rasterizer::paintPolygon2D(paintDraw, 0, 0, nb, pts);
        
        glLineWidth(1);
        glBegin(GL_LINES);
        glColor3f(0, 1.0, 0);
        glVertex2d( pts[0], pts[1] );
        for ( unsigned int i = 1; i < nb ; ++i )
        {
            glColor3f(0, 1.0, 1.0);
            glVertex2d(pts[2*i], pts[2*i+1]);
            glColor3f(0, 1.0, 0);
            glVertex2d(pts[2*i], pts[2*i+1]);
        }
        glColor3f(0, 1.0, 1.0);
        glVertex2d(pts[0], pts[1]);
        glEnd();
    }
}


void display3D()
{
#if ( 0 )
     //--------------draw a grid:
     glPointSize(0.5);
     glBegin(GL_POINTS);
     glColor3f(0.3, 0.3, 0.3);
     for ( int i = -size; i <= size; i += 1)
     for ( int j = -size; j <= size; j += 1)
     for ( int k = -size; k <= size; k += 1)
     glVertex3i(i, j, k);
     glEnd();
#endif
    
    
    glPointSize(10.0);
    glBegin(GL_POINTS);
    glColor3f(0, 0, 1.0);
    for ( unsigned int i = 0; i < nbpts ; ++i )
        glVertex3d(pts[3*i], pts[3*i+1], pts[3*i+2]);
    glEnd();
    
    if ( nbpts == 2 )
    {
        realTest();
        //Rasterizer::paintHexLine3D(paintDraw, 0, 0, pts, pts+DIM, radius, shift, delta);
        Rasterizer::paintFatLine3D(paintDraw, 0, 0, pts, pts+DIM, radius, shift, delta);
    } else
    {
        Rasterizer::paintPolygon3D(paintDraw, 0, 0, nbpts, pts);
    }
}



void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
#if (DIM == 2)
    display2D();
#else
    display3D();
#endif
}

extern bool rasterizer_draw_things;

/* 
 This only work if rasterizer does not issue openGL commands */
void speedTest(unsigned int cnt)
{
    clearHits();
    newPoints();
    rasterizer_draw_things = 0;
    
    for ( unsigned int ii = 0; ii < cnt; ++ii )
        Rasterizer::paintFatLine2D(paintHit, 0, 0, pts, pts+DIM, radius, shift, delta);
}

//===================================================================

int main(int argc, char* argv[])
{
    if ( argc > 1 )
        speedTest(atoi(argv[1]));
    else
    {
        glutInit(&argc, argv);
        glApp::init(display, 3);
        glApp::attachMenu(GLUT_RIGHT_BUTTON);
        glApp::setScale(size+radius+1);
        glutKeyboardFunc(processNormalKey);
        RNG.seedTimer();
        glutMainLoop();
    }
    return EXIT_SUCCESS;
}


