// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Francois Nedelec, nedelec@embl.de, January 2008

#define DIM 2

#include "vector.h"
#include "random.h"
#include "glapp.h"
#include "real.h"
#include "tictoc.h"
#include "gle.h"
using namespace gle;

#define DISPLAY
#include "grid.h"
extern Random RNG;

//area of the grid
const int    range = 5;
real  left[] = {  -range,  -range,  0 };
real right[] = {   range,   range,  range };
int   size[] = { 2*range,   range,  range };


typedef Grid<DIM, real, unsigned int> grid_type;
grid_type mGrid;

grid_type::index_type indx;
int coord[DIM];
Vector3 pos(0,0,0);
Vector3 nod(0,0,0);


#define TEST_REGIONS
real  regionRadius = 1.5;

//------------------------------------------------------------------------------
void throwMarbles(int cnt)
{
    mGrid.setValues(0);
    real w[3] = { 0, 0, 0 };
    for( int n = 0; n < cnt; ++n )
    {
        w[0] = range * RNG.sreal();
        w[1] = range * RNG.sreal();
        w[2] = range * RNG.sreal();
        ++mGrid(w);
    }
}


void processNormalKey(unsigned char c, int x=0, int y=0)
{
    switch (c)
    {
        case 'p':
            mGrid.periodic(!mGrid.periodic());
            break;
            
        case 'i':
            //decrease region-radius
            if ( regionRadius > 1 )
                regionRadius -= 0.25;
            mGrid.createRoundRegions(regionRadius);
            //mGrid.createSquareRegions(regionRadius);
            glApp::flashText("radius = %f", regionRadius);
            break;

        case 'o':
            // increase region-radius
            regionRadius += 0.25;
            mGrid.createRoundRegions(regionRadius);
            //mGrid.createSquareRegions(regionRadius);
            glApp::flashText("radius = %f", regionRadius);
            break;

        case 'r':
            mGrid.createRoundRegions(regionRadius);
            glApp::flashText("radius = %f", regionRadius);
            break;

        case 's':
            mGrid.createSideRegions(regionRadius);
            break;
            
        case 'h':
            printf("Shift-click to position test-point\n");
            return;

        case ' ': 
            throwMarbles(20);
            break;

        default:
            glApp::processNormalKey(c,x,y);
            return;
    }
    glutPostRedisplay();
}


//------------------------------------------------------------------------------

///set callback for shift-click, with unprojected click position
void  processMouseAction(const Vector3 & a, int)
{
    pos = a;
    indx = mGrid.index(pos);
    mGrid.setPositionFromIndex(nod, indx, 0);
    mGrid.setCoordinatesFromIndex(coord, indx);
    
    char str[32];

    if ( mGrid.hasRegions() )
    {
        real num = mGrid.sumValuesInRegion(indx);
        snprintf(str, sizeof(str), "cell %u : coord %i %i : %.0f marbles",
                 indx, coord[0], coord[1], num);
    } 
    else
    {
        snprintf(str, sizeof(str), "cell %u : coord %i %i", indx, coord[0], coord[1]);
    }
    
    glApp::GP.message_left = str;
    glutPostRedisplay();
}

///set callback for shift-drag, with unprojected mouse and click positions
void  processMouseActionMotion(Vector3 & a, const Vector3 & b, int m)
{
    processMouseAction(b, m);
}

//------------------------------------------------------------------------------

void display()
{
    char str[16];
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    //--------------draw a grid in gray:
    glColor4f(1,1,1,0.6);
    glLineWidth(1.0);
    drawEdges(mGrid);
    
    //--------------draw filled cells
    glColor4f(0,0,1,1);
    for ( grid_type::index_type c = 0 ; c < mGrid.nbCells(); ++c )
    {
        int n = mGrid.cell(c);
        if ( n )
        {
            Vector x;
            mGrid.setPositionFromIndex(x, c, 0.5);
            snprintf(str, sizeof(str), "%i", n);
            gleDrawText(x, str, GLUT_BITMAP_HELVETICA_18);
        }
    }
    glEnd();
    
    //-------------draw selected-cell
    glPointSize(8.0);
    glBegin(GL_POINTS);
    glColor4f(1,1,1,1);
    gleVertex(pos);
    glColor4f(1,1,0,1);
    gleVertex(nod);
    glEnd();

    //-------------draw region
    if ( mGrid.hasRegions() )
    {
        int * offset = 0;
        int nb = mGrid.getRegion(offset, indx);

        glColor4f(1,1,1,0.7);
        for ( int ii = 0; ii < nb; ++ii )
        {
            Vector x;
            mGrid.setPositionFromIndex(x, indx+offset[ii], 0.4);
            snprintf(str, sizeof(str), "%i", ii);
            gleDrawText(x, str, GLUT_BITMAP_HELVETICA_10);
        }
    }
    else
    {
        real vi = mGrid.interpolate(pos);
        snprintf(str, sizeof(str), "cell %u %f", indx, vi);
        glApp::GP.message_right = str;
        
#if ( DIM == 2 )
        vi = mGrid.interpolate2D(pos);
#else
        vi = mGrid.interpolate3D(pos);
#endif
        snprintf(str, sizeof(str), "cell %u %f", indx, vi);
        glApp::GP.message_left = str;
    }
}


void speedTest()
{
    printf("Real test...");

    real  left[] = { 0, 0, 0};
    real right[] = { 1, 1, 1};
    int   size[] = { 10, 10, 10};

    Grid<3, float, unsigned int> map;
    map.setDimensions(left, right, size);
    map.createCells();
    map.setValues(0);

    real w[3];
    for ( int cc=0; cc<10000; ++cc )
    {
        w[0] = RNG.preal();
        w[1] = RNG.preal();
        w[2] = RNG.preal();
        for ( int x = 0; x < 1000; ++x )
        {
            ++map( w );
            ++map( w );
            ++map( w );
            ++map( w );
            ++map( w );
            ++map( w );
            ++map( w );
            ++map( w );
            ++map( w );
            ++map( w );
        }
    }

    FILE* test = fopen("testgrid.out","w");
    map.printValues(test, 0);
    fclose(test);
    printf("wrote file testgrid.out\n");
}


void testInterpolate()
{
    real  left[] = { 0, 0, 0 };
    real right[] = { 1, 1, 1 };
    int   size[] = { 100, 100, 100 };
    
    const int MAX = 1 << 14;
    real  rand[MAX+3] = { 0 };
    for ( int i = 0; i < MAX+3; ++i )
        rand[i] = RNG.preal();
    
    Grid<3, double, unsigned int> map;
    map.setDimensions(left, right, size);
    map.createCells();
    map.setValues(0);
    
    const int CNT = 1000000;
    for ( int cc = 0; cc < CNT; ++cc )
    {
        real w[] = { RNG.preal(), RNG.preal(), RNG.preal() };
        ++map( w );
    }

    
    real * vec[CNT];
    for ( int i = 0; i < CNT; ++i )
        vec[i] = rand + ( RNG.pint() & MAX );

    real sum = 0;
    TicToc::tic();
    for ( int r = 0; r < 100; ++r )
        for ( int cc = 0; cc < CNT; ++cc )
            sum += map.interpolate3D(vec[cc]) + map.interpolate3D(vec[cc]);
    printf("sum = %f\n", sum);
    TicToc::toc("interpolate3D");
    
    real som = 0;
    TicToc::tic();
    for ( int r = 0; r < 100; ++r )
        for ( int cc = 0; cc < CNT; ++cc )
            som += map.interpolate(vec[cc]) + map.interpolate(vec[cc]);
    printf("som = %f\n", sum);
    TicToc::toc("interpolate  ");
}


int main(int argc, char* argv[])
{
    RNG.seedTimer();

    if ( argc > 1 )
    {
        testInterpolate();
        return 0;
    }

    //initialize the grid:
    mGrid.setDimensions(left, right, size);
    mGrid.createCells();
    //mGrid.periodic(1);
    throwMarbles(8*(1<<DIM));

    glutInit(&argc, argv);
    glApp::init(display, DIM);
    glApp::attachMenu(GLUT_RIGHT_BUTTON);
    glApp::setScale(range+3);
    glApp::actionFunc(processMouseAction);
    glApp::actionFunc(processMouseActionMotion);
    
    glutKeyboardFunc(processNormalKey);
    glutMainLoop();

    return 0;
}
