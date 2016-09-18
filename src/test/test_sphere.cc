// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Test class PointsOnSphere

#include <pthread.h>

#include "pointsonsphere.h"
#include "glapp.h"
#include "gle.h"
extern Random RNG;

int nPoints = 12;
PointsOnSphere S, T;

PointsOnSphere * front = &S;
PointsOnSphere * back  = &T;

pthread_t       thread;
pthread_mutex_t lock;

//------------------------------------------------------------------------------

void batch(int mn, int mx)
{
    int total = 0;
    
    for ( int nbp = mn; nbp < mx; nbp += 7 )
    {
        printf("%4i pts :", nbp);
        printf(" %6.4f :", S.expectedDistance(nbp));
        for ( int m=0; m < mx; ++m )
        {
            int step = S.distributePoints(nbp);
            printf(" %6.4f", S.minimumDistance());
            total += step;
        }
        printf(" energy %7.2f", S.finalEnergy());
        printf(" total iter. %7i\n", total);
    }
}
//------------------------------------------------------------------------------

void* calculateSphere(void * arg)
{    
    glApp::GP.message_left="Calculating...";
    glApp::postRedisplay();

    back->distributePoints(nPoints);

    glApp::GP.message_left.clear();
    glApp::postRedisplay();
    
    PointsOnSphere * tmp = back;
    back  = front;
    front = tmp;
    
    pthread_mutex_unlock(&lock);
    pthread_exit(0);
}

//------------------------------------------------------------------------------
void processNormalKey(unsigned char c, int x, int y)
{
    switch (c)
    {
        case 't': nPoints-=10;    break;
        case 'y': nPoints+=1;     break;
        case 'u': nPoints+=10;    break;
        case 'i': nPoints+=100;   break;
        case 'o': nPoints+=1000;  break;
        case 'q' : exit(1);
            
        default:
            glApp::processNormalKey(c,x,y);
            return;
    }
    if ( nPoints < 1 )
        nPoints = 1;
    
    if ( 0 == pthread_mutex_trylock(&lock) )
    {
        pthread_create(&thread, 0, &calculateSphere, (void *)1);
    }
    else
    {
        glApp::flashText("already calculating...");
    }
}

//------------------------------------------------------------------------------
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glPointSize(8);
    glBegin(GL_POINTS);
    
    if ( front == &T )
        glColor3f(0.0, 0.0, 1.0);
    else
        glColor3f(0.0, 1.0, 0.0);
    
    for ( unsigned int ii=0; ii < front->nbPoints(); ++ii )
    {
#ifdef REAL_IS_FLOAT
        glVertex3fv( front->addr(ii) );
#else
        glVertex3dv( front->addr(ii) );
#endif
    }

    glEnd();
    
    if ( 0 )
    {
        glColor4f(0.3, 0.3, 0.3, 0.5);
        glDepthMask(GL_FALSE);
        glutSolidSphere(0.98,30,30);
        glDepthMask(GL_TRUE);
    }
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{    
    RNG.seedTimer();
    if ( argc == 3 ) 
    {
        batch(atoi(argv[1]), atoi(argv[2]));
        return EXIT_SUCCESS;
    }
    if ( argc == 2 ) 
    {
        nPoints = atoi(argv[1]);
        argv[1][0] = '\0';
    }
    
    pthread_mutex_init(&lock, 0);
    front->distributePoints(nPoints);
    
    glutInit(&argc, argv);
    glApp::init(display, 3);
    glApp::setScale(2);
    glApp::attachMenu(GLUT_RIGHT_BUTTON);
    glutKeyboardFunc(processNormalKey);
    
    glutMainLoop();
    return EXIT_SUCCESS;
}
