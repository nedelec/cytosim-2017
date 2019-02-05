// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <cctype>
#include "assert_macro.h"
#include "gle.h"
#include "glut.h"
#include "smath.h"


void gle::initialize()
{
    //std::cerr << "gle::initialize()" << std::endl;
    /*
     GLfloat da = 0.5 * M_PI / (GLfloat)finesse;
     GLfloat ang = 0;
     for ( int ii = 0; ii <= 4*finesse; ++ii )
     {
       co[ii] = cosf(ang);
       si[ii] = sinf(ang);
       ang += da;
     }
     */
#ifdef GLE_USES_DISPLAY_LISTS
    initializeDL();
#endif
}

void gle::release()
{
#ifdef GLE_USES_DISPLAY_LISTS
    releaseDL();
#endif
}

//-----------------------------------------------------------------------
void gle::gleAlignX(const Vector2 & v)
{
    const GLfloat n = v.norm();
    //warning! this matrix is displayed transposed
    GLfloat mat[16] = {
        (GLfloat)v.XX, -(GLfloat)v.YY,  0,  0,
        (GLfloat)v.YY,  (GLfloat)v.XX,  0,  0,
        0,                          0,  n,  0,
        0,                          0,  0,  1 };
    glMultMatrixf(mat);
}

//-----------------------------------------------------------------------
/**
Graphical elements are aligned in 3D along Z and this function is used
to rotate them in the XY plane for the 2D display.
The rotation is chosen such that the Y face of the rotated object points
down the Z axis. In this way, the lower part of the object is drawn first,
such that the upper half overwrites it and become the only visible part.
The display is thus correct even if DEPTH_TEST is disabled.
*/
void gle::gleAlignZ(const Vector2 & A, const Vector2 & B)
{
    Vector2 D = B - A;
    GLfloat n = sqrt(D.XX*D.XX+D.YY*D.YY);
    if ( n < REAL_EPSILON ) return;
    //warning! this matrix is displayed transposed
    GLfloat mat[16] = {
        (GLfloat)D.YY/n, -(GLfloat)D.XX/n,   0,   0,
        0,                              0,  -1,   0,
        (GLfloat)D.XX,      (GLfloat)D.YY,   0,   0,
        (GLfloat)A.XX,      (GLfloat)A.YY,   0,   1 };
    glMultMatrixf(mat);
}

//-----------------------------------------------------------------------
/**
 ts is the transverse scaling done in the XY plane after rotation
 */
void gle::gleAlignZ(const Vector2 & A, const Vector2 & B, real ts)
{
    Vector2 D = B - A;
    GLfloat p = ts / sqrt(D.XX*D.XX+D.YY*D.YY);
    //warning! this matrix is displayed transposed
    GLfloat mat[16] = {
        (GLfloat)D.YY*p, -(GLfloat)D.XX*p,            0,   0,
        0,                              0, -(GLfloat)ts,   0,
        (GLfloat)D.XX,      (GLfloat)D.YY,            0,   0,
        (GLfloat)A.XX,      (GLfloat)A.YY,            0,   1 };
    glMultMatrixf(mat);
}

//-----------------------------------------------------------------------
void gle::gleRotate(const Vector3 & v1, const Vector3 & v2, const Vector3 & v3, bool inverse)
{
    GLfloat mat[16];
    for ( int ii = 0; ii < 3; ++ii )
    {
        if ( inverse )
        {
            mat[4*ii  ] = v1[ii];
            mat[4*ii+1] = v2[ii];
            mat[4*ii+2] = v3[ii];
        }
        else
        {
            mat[ii  ]   = v1[ii];
            mat[ii+4]   = v2[ii];
            mat[ii+8]   = v3[ii];
        }
        mat[ii+12]  = 0;
        mat[ii*4+3] = 0;
    }
    mat[15] = 1;
    glMultMatrixf(mat);
}

//-----------------------------------------------------------------------
void gle::gleTransRotate(const Vector3 & v1, const Vector3 & v2,
                         const Vector3 & v3, const Vector3 & vt)
{
    //warning! this matrix is displayed transposed
    GLfloat mat[16] = {
        (GLfloat)v1.XX, (GLfloat)v1.YY, (GLfloat)v1.ZZ, 0,
        (GLfloat)v2.XX, (GLfloat)v2.YY, (GLfloat)v2.ZZ, 0,
        (GLfloat)v3.XX, (GLfloat)v3.YY, (GLfloat)v3.ZZ, 0,
        (GLfloat)vt.XX, (GLfloat)vt.YY, (GLfloat)vt.ZZ, 1};
    glMultMatrixf(mat);
}


//-----------------------------------------------------------------------
#pragma mark - 2D Primitives


void gle::gleTriangle0()
{
    const GLfloat H = 0.5 * sqrt(3);
    glVertex2f(0, 1);
    glVertex2f(-H, -0.5);
    glVertex2f( H, -0.5);
}

void gle::gleTriangleS()
{
    glBegin(GL_TRIANGLES);
    glNormal3f(0, 0, 1);
    gleTriangle0();
    glEnd();
}

void gle::gleTriangleL()
{
    glBegin(GL_LINE_LOOP);
    glNormal3f(0, 0, 1);
    gleTriangle0();
    glEnd();
}

//-----------------------------------------------------------------------

void gle::gleNabla0()
{
    const GLfloat H = 0.5 * sqrt(3);
    glVertex2f(0, -1);
    glVertex2f( H, 0.5);
    glVertex2f(-H, 0.5);
}

void gle::gleNablaS()
{
    glBegin(GL_TRIANGLES);
    glNormal3f(0, 0, 1);
    gleNabla0();
    glEnd();
}

void gle::gleNablaL()
{
    glBegin(GL_LINE_LOOP);
    glNormal3f(0, 0, 1);
    gleNabla0();
    glEnd();
}

//-----------------------------------------------------------------------
void gle::gleSquare0()
{
    glVertex2f( 1,  1);
    glVertex2f(-1,  1);
    glVertex2f(-1, -1);
    glVertex2f( 1, -1);
}

void gle::gleSquareS()
{
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, 1);
    gleSquare0();
    glEnd();
}

void gle::gleSquareL()
{
    glBegin(GL_LINE_LOOP);
    glNormal3f(0, 0, 1);
    gleSquare0();
    glEnd();
}

//-----------------------------------------------------------------------
void gle::glePentagon0()
{
    const GLfloat R  = sqrt( 4 * M_PI / sqrt( 5 * ( 5 + 2 * sqrt(5))) );
    const GLfloat C1 = R * cosf(M_PI*0.1);
    const GLfloat S1 = R * sinf(M_PI*0.1);
    const GLfloat C3 = R * cosf(M_PI*0.3);
    const GLfloat S3 = R * sinf(M_PI*0.3);

    glVertex2f(  0,  1);
    glVertex2f(-C1,  S1);
    glVertex2f(-C3, -S3);
    glVertex2f( C3, -S3);
    glVertex2f( C1,  S1);
}

void gle::glePentagonS()
{
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, 1);
    glVertex2f(0, 0);
    glePentagon0();
    glVertex2f(0, 1);
    glEnd();
}

void gle::glePentagonL()
{
    glBegin(GL_LINE_LOOP);
    glNormal3f(0, 0, 1);
    glePentagon0();
    glEnd();
}

//-----------------------------------------------------------------------
/// draw hexagon that has the same surface as the disc or Radius 1.
void gle::gleHexagon0()
{
    const GLfloat R = sqrt( 2 * M_PI / ( 3 * sqrt(3) ));
    const GLfloat H = R * 0.5 * sqrt(3);
    const GLfloat X = R * 0.5;
    glVertex2f( R,  0);
    glVertex2f( X,  H);
    glVertex2f(-X,  H);
    glVertex2f(-R,  0);
    glVertex2f(-X, -H);
    glVertex2f( X, -H);
}

void gle::gleHexagonS()
{
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, 1);
    glVertex2f(0, 0);
    gleHexagon0();
    glVertex2f(1, 0);
    glEnd();
}

void gle::gleHexagonL()
{
    glBegin(GL_LINE_LOOP);
    glNormal3f(0, 0, 1);
    gleHexagon0();
    glEnd();
}

//-----------------------------------------------------------------------

void gle::gleCircle0()
{
    const GLfloat inc = M_PI / ( 8 * finesse );
    const GLfloat max = 2 * M_PI - 0.5 * inc;

    glNormal3f(1, 0, 0);
    glVertex2f(1, 0);
    GLfloat ang = inc;
    while ( ang < max )
    {
        glNormal3f(cosf(ang), sinf(ang), 0);
        glVertex2f(cosf(ang), sinf(ang));
        ang += inc;
    }
}

void gle::gleCircleL()
{
    const GLfloat inc = M_PI / ( 4 * finesse );
    const GLfloat max = 2 * M_PI - 0.5 * inc;

    glBegin(GL_LINE_LOOP);
    glNormal3f(1, 0, 0);
    glVertex2f(1, 0);
    GLfloat ang = inc;
    while ( ang < max )
    {
        glVertex2f(cosf(ang), sinf(ang));
        ang += inc;
    }
    glEnd();
}

void gle::gleCircleS()
{
    const GLfloat inc = M_PI / ( 4 * finesse );
    const GLfloat max = 2 * M_PI - 0.5 * inc;

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, 1);
    glVertex2f(0, 0);
    glVertex2f(1, 0);
    GLfloat ang = inc;
    while ( ang < max )
    {
        glVertex2f(cosf(ang), sinf(ang));
        ang += inc;
    }
    glVertex2f(1, 0);
    glEnd();
}

//-----------------------------------------------------------------------

void gle::gleStar0()
{
    const GLfloat R  = 1.2;
    const GLfloat C1 = R * cosf(M_PI*0.1);
    const GLfloat S1 = R * sinf(M_PI*0.1);
    const GLfloat C3 = R * cosf(M_PI*0.3);
    const GLfloat S3 = R * sinf(M_PI*0.3);
    const GLfloat H = -0.6;

    glVertex2f(    0,     R);
    glVertex2f( H*C3, -H*S3);
    glVertex2f(  -C1,    S1);
    glVertex2f( H*C1,  H*S1);
    glVertex2f(  -C3,   -S3);
    glVertex2f(    0,   H*R);
    glVertex2f(   C3,   -S3);
    glVertex2f(-H*C1,  H*S1);
    glVertex2f(   C1,    S1);
    glVertex2f(-H*C3, -H*S3);
}

void gle::gleStarS()
{
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, 1);
    glVertex2f(0, 0);
    gleStar0();
    glVertex2f(0, 1);
    glEnd();
}

void gle::gleStarL()
{
    glBegin(GL_LINE_LOOP);
    glNormal3f(0, 0, 1);
    gleStar0();
    glEnd();
}

//-----------------------------------------------------------------------

void gle::glePlusS()
{
    const GLfloat R = 1.1;
    const GLfloat C = 0.4;

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, 1);
    glVertex2f( R,  C);
    glVertex2f(-R,  C);
    glVertex2f(-R, -C);
    glVertex2f( R, -C);
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, 1);
    glVertex2f( C,  R);
    glVertex2f(-C,  R);
    glVertex2f(-C,  C);
    glVertex2f( C,  C);
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, 1);
    glVertex2f( C, -C);
    glVertex2f(-C, -C);
    glVertex2f(-C, -R);
    glVertex2f( C, -R);
    glEnd();
}

void gle::glePlusL()
{
    const GLfloat R = 1.2;
    const GLfloat C = 0.6;

    glBegin(GL_LINE_LOOP);
    glNormal3f(0, 0, 1);
    glVertex2f( C,  R);
    glVertex2f(-C,  R);
    glVertex2f(-C,  C);
    glVertex2f(-R,  C);
    glVertex2f(-R, -C);
    glVertex2f(-C, -C);
    glVertex2f(-C, -R);
    glVertex2f( C, -R);
    glVertex2f( C, -C);
    glVertex2f( R, -C);
    glVertex2f( R,  C);
    glVertex2f( C,  C);
    glEnd();
}


//-----------------------------------------------------------------------
#pragma mark - 3D Primitives

void gle::gleTube0(GLfloat a, GLfloat b, int fin)
{
    const GLfloat inc = M_PI / fin;
    const GLfloat max = 2 * M_PI + 0.5 * inc;

    glBegin(GL_TRIANGLE_STRIP);
    GLfloat ang = 0;
    while ( ang < max )
    {
        glNormal3f(cosf(ang), sinf(ang),  0);
        glVertex3f(cosf(ang), sinf(ang),  b);
        glVertex3f(cosf(ang), sinf(ang),  a);
        ang += inc;
    }
    glEnd();
}

void gle::gleTubeZ(GLfloat za, GLfloat ra, gle_color ca, GLfloat zb, GLfloat rb, gle_color cb)
{
    const GLfloat inc = M_PI / finesse;
    const GLfloat max = 2 * M_PI + 0.5 * inc;

    glBegin(GL_TRIANGLE_STRIP);
    GLfloat ang = 0;
    while ( ang < max )
    {
        cb.color();
        glNormal3f(cosf(ang), sinf(ang), 0);
        glVertex3f(rb*cosf(ang), rb*sinf(ang), zb);

        ca.color();
        glNormal3f(cosf(ang), sinf(ang), 0);
        glVertex3f(ra*cosf(ang), ra*sinf(ang), za);
        ang += inc;
    }
    glEnd();
}

//-----------------------------------------------------------------------

void gle::gleTube1()
{
    gleTube0(0, 1, finesse/2);
}

void gle::gleLongTube1()
{
    gleTube0(-0.5, 1.5, finesse/2);
}

void gle::gleTube2()
{
    gleTube0(0, 1, finesse);
}

void gle::gleLongTube2()
{
    gleTube0(-0.5, 1.5, finesse);
}

void gle::gleCylinder1()
{
    gleTube1();
    gleTranslate(0,0,1);
    gleCircleS();
    gleTranslate(0,0,-1);
    glRotated(180,0,1,0);
    gleCircleS();
}

//-----------------------------------------------------------------------

/// using a GLU quadric to draw a smooth ball
void gleSphereGLU(unsigned int fin)
{
    static GLUquadricObj * qobj = 0;
    if ( qobj == 0 )
    {
        qobj = gluNewQuadric();
        gluQuadricDrawStyle(qobj, GLU_FILL);
        gluQuadricNormals(qobj, GLU_SMOOTH);
        gluQuadricOrientation(qobj, GLU_OUTSIDE);
    }
    gluSphere(qobj, 1.0, fin, fin/2);
}

void gle::gleSphere1() { gleSphereGLU(finesse); }
void gle::gleSphere2() { gleSphereGLU(2*finesse); }
void gle::gleSphere4() { gleSphereGLU(4*finesse); }


//-----------------------------------------------------------------------
#pragma mark -

/**
 A cylindrical band of specified width drawn on the equator of a sphere of radius 1.
 The band is in the XY plane. The axis of the cylinder is Z.
 */
void gle::gleArrowedBand(GLfloat width)
{
    const int fin = 8 * finesse;
    GLfloat w = 0.5 * width;

    glBegin(GL_TRIANGLES);
    glVertex3f(1, 0,-w);
    glVertex3f(1, 0, w);
    for ( int ii = 0; ii < fin; ++ii )
    {
        GLfloat ang = ii * 2 * M_PI / (GLfloat) fin;
        GLfloat c = cosf(ang);
        GLfloat s = sinf(ang);

        glNormal3f(c, s, 0);
        glVertex3f(c, s, 0);
        glVertex3f(c, s, w);
        glVertex3f(c, s,-w);
    }
    glVertex3f(1, 0, 0);
    glEnd();
}

void gle::gleDecoratedSphere()
{
    gleSphere4B();
    const GLfloat width = 0.02;
    glPushMatrix();
    gleArrowedBand(width);
    glRotated(-90,1,0,0);
    gleArrowedBand(width);
    glRotated(90,0,1,0);
    gleArrowedBand(width);
    glPopMatrix();
}

//-----------------------------------------------------------------------
inline void icoFace(GLfloat* a, GLfloat* b, GLfloat* c)
{
    glNormal3f((a[0]+b[0]+c[0])/3.0, (a[1]+b[1]+c[1])/3.0, (a[2]+b[2]+c[2])/3.0);
    glVertex3fv(a);
    glVertex3fv(b);
    glVertex3fv(c);
}

void gle::gleIcosahedron1()
{
    const GLfloat tau=0.8506508084;      /* t=(1+sqrt(5))/2, tau=t/sqrt(1+t^2)  */
    const GLfloat one=0.5257311121;      /* one=1/sqrt(1+t^2) , unit sphere     */

    /* Twelve vertices of icosahedron on unit sphere */
    GLfloat pts[] = {
        +tau,  one,    0 , // 0
        -tau, -one,    0 , // 1
        -tau,  one,    0 , // 2
        +tau, -one,    0 , // 3
        +one,   0 ,  tau , // 4
        -one,   0 , -tau , // 5
        +one,   0 , -tau , // 6
        -one,   0 ,  tau , // 7
        0   ,  tau,  one , // 8
        0   , -tau, -one , // 9
        0   , -tau,  one , // 10
        0   ,  tau, -one };// 11

    /* The faces are ordered with increasing Z */
    glBegin(GL_TRIANGLES);
    icoFace(pts+3*5, pts+3*6,  pts+3*9);
    icoFace(pts+3*5, pts+3*11, pts+3*6);

    icoFace(pts+3*6, pts+3*3,  pts+3*9);
    icoFace(pts+3*2, pts+3*11, pts+3*5);
    icoFace(pts+3*1, pts+3*5,  pts+3*9);
    icoFace(pts+3*0, pts+3*6,  pts+3*11);

    icoFace(pts+3*0, pts+3*3,  pts+3*6);
    icoFace(pts+3*1, pts+3*2,  pts+3*5);

    icoFace(pts+3*1, pts+3*9,  pts+3*10);
    icoFace(pts+3*0, pts+3*11, pts+3*8);
    icoFace(pts+3*8, pts+3*11, pts+3*2);
    icoFace(pts+3*9, pts+3*3,  pts+3*10);

    icoFace(pts+3*0, pts+3*4,  pts+3*3);
    icoFace(pts+3*1, pts+3*7,  pts+3*2);

    icoFace(pts+3*0, pts+3*8,  pts+3*4);
    icoFace(pts+3*1, pts+3*10, pts+3*7);
    icoFace(pts+3*3, pts+3*4,  pts+3*10);
    icoFace(pts+3*7, pts+3*8,  pts+3*2);

    icoFace(pts+3*4, pts+3*8,  pts+3*7);
    icoFace(pts+3*4, pts+3*7,  pts+3*10);
    glEnd();
}

//-----------------------------------------------------------------------
void gle::gleCylinderH()
{
    const GLfloat inc = M_PI / finesse;
    const GLfloat max = 2 * M_PI + 0.5 * inc;


    glBegin(GL_TRIANGLE_FAN);
    glNormal3f( 0, 0, -1 );
    glVertex3f( 0, 0, -1 );
    GLfloat ang = 0;
    while ( ang < max )
    {
        glVertex3f( cosf(ang), -sinf(ang), -0.5 );
        ang += inc;
    }
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    ang = 0;
    while ( ang < max )
    {
        glNormal3f( cosf(ang), sinf(ang),  0 );
        glVertex3f( cosf(ang), sinf(ang),  0.5 );
        glVertex3f( cosf(ang), sinf(ang), -0.5 );
        ang += inc;
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f( 0, 0, 1 );
    glVertex3f( 0, 0, 0 );
    ang = 0;
    while ( ang < max )
    {
        glVertex3f( cosf(ang), sinf(ang), 0.5 );
        ang += inc;
    }
    glEnd();
}


void gle::gleCone1()
{
    const GLfloat inc = M_PI / finesse;
    const GLfloat max = 2 * M_PI + 0.5 * inc;


    glBegin(GL_TRIANGLE_FAN);
    glNormal3f( 0, 0, -1 );
    glVertex3f( 0, 0, -1 );
    GLfloat ang = 0;
    while ( ang < max )
    {
        glVertex3f( cosf(ang), -sinf(ang), -1 );
        ang += inc;
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f( 0, 0, 1 );
    glVertex3f( 0, 0, 2 );
    GLfloat cn = 3.f/sqrt(10), sn = 1.f/sqrt(10);
    ang = 0;
    while ( ang < max )
    {
        glNormal3f( cn*cosf(ang), cn*sinf(ang), sn );
        glVertex3f( cosf(ang), sinf(ang), -1 );
        ang += inc;
    }
    glEnd();
}


void gle::gleArrowTail1()
{
    const GLfloat inc = M_PI / finesse;
    const GLfloat max = 2 * M_PI + 0.5 * inc;


    GLfloat cn = 1.f/sqrt(2);

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f( 0, 0, -1 );
    glVertex3f( 0, 0, -0.5 );
    GLfloat ang = 0;
    while ( ang < max )
    {
        glNormal3f( -cn*cosf(ang), cn*sinf(ang), -cn );
        glVertex3f( cosf(ang), -sinf(ang), -1.5 );
        ang += inc;
    }
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    ang = 0;
    while ( ang < max )
    {
        glNormal3f( cosf(ang), sinf(ang),  0 );
        glVertex3f( cosf(ang), sinf(ang),  0.5 );
        glVertex3f( cosf(ang), sinf(ang), -1.5 );
        ang += inc;
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f( 0, 0, 1 );
    glVertex3f( 0, 0, 1.5 );
    ang = 0;
    while ( ang < max )
    {
        glNormal3f( cn*cosf(ang), cn*sinf(ang), cn );
        glVertex3f( cosf(ang), sinf(ang), 0.5 );
        ang += inc;
    }
    glEnd();
}

/**
 Draw three fins like in the tail of an arrow
 */
void gle::gleArrowTail2()
{
    GLfloat r = 0.1;  //bottom inner radius
    GLfloat c = 0.5, d = -0.5;
    GLfloat s = sqrt(3)/2, t = -s;
    GLfloat rc = r * c;
    GLfloat rs = r * s;
    GLfloat rt = -rs;

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(  0, -1, 0 );
    glVertex3f( rc, rt, -0.5 );
    glVertex3f(  1,  0, -1.5 );
    glVertex3f(  1,  0,  0.5 );
    glVertex3f(  0,  0,  1.5 );
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(  0, +1, 0 );
    glVertex3f( rc, rs, -0.5 );
    glVertex3f(  0,  0,  1.5 );
    glVertex3f(  1,  0,  0.5 );
    glVertex3f(  1,  0, -1.5 );
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(  s,  d, 0 );
    glVertex3f( rc, rt, -0.5 );
    glVertex3f(  0,  0,  1.5 );
    glVertex3f(  d,  t,  0.5 );
    glVertex3f(  d,  t, -1.5 );
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(  t, c, 0 );
    glVertex3f( -r, 0, -0.5 );
    glVertex3f(  d, t, -1.5 );
    glVertex3f(  d, t,  0.5 );
    glVertex3f(  0, 0,  1.5 );
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(  s, c, 0 );
    glVertex3f( rc, rs, -0.5 );
    glVertex3f(  d,  s, -1.5 );
    glVertex3f(  d,  s,  0.5 );
    glVertex3f(  0,  0,  1.5 );
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(  t, d, 0 );
    glVertex3f( -r, 0, -0.5 );
    glVertex3f(  0, 0,  1.5 );
    glVertex3f(  d, s,  0.5 );
    glVertex3f(  d, s, -1.5 );
    glEnd();

    // closing the bottom gaps
    glBegin(GL_TRIANGLES);
    glNormal3f(  c,  t, -1 );
    glVertex3f( rc, rs, -0.5 );
    glVertex3f( -r,  0, -0.5 );
    glVertex3f(  d,  s, -1.5 );

    glNormal3f(  c,  s, -1 );
    glVertex3f( -r,  0, -0.5 );
    glVertex3f( rc, rt, -0.5 );
    glVertex3f(  d,  t, -1.5 );

    glNormal3f( -1,  0, -1 );
    glVertex3f( rc, rt, -0.5 );
    glVertex3f( rc, rs, -0.5 );
    glVertex3f(  1,  0, -1.5 );
    glEnd();
}


GLfloat dumbbellRadius(GLfloat z)
{
    return sin(M_PI*z) * ( 1.3 + cos(2*M_PI*z) );
}


void gle::gleDumbbell1()
{
    gleRevolution(dumbbellRadius);
}

GLfloat barrelRadius(GLfloat z)
{
    return sin(M_PI*z);
}

void gle::gleBarrel1()
{
    gleRevolution(barrelRadius);
}


//-----------------------------------------------------------------------
#pragma mark - Primitives with Display Lists

#ifdef GLE_USES_DISPLAY_LISTS

/// index of OpenGL display lists
GLuint gle::dlist = 0;

void makeDisplayList(GLint list, void (*primitive)())
{
    if ( glIsList(list) )
    {
        glNewList(list, GL_COMPILE);
        primitive();
        glEndList();
    }
    else
    {
        fprintf(stderr, "gle::makeDisplayList failed\n");
    }
}


void callDisplayList(GLint list)
{
    assert_true( glIsList(list) );
    glCallList(list);
}

void gle::initializeDL()
{
    if ( dlist == 0 )
    {
        dlist = glGenLists(15);

        makeDisplayList(dlist+0, gleCircleL);
        makeDisplayList(dlist+1, gleCircleS);
        makeDisplayList(dlist+2, gleSphere1);
        makeDisplayList(dlist+3, gleSphere2);
        makeDisplayList(dlist+4, gleSphere4);
        makeDisplayList(dlist+5, gleTube1);
        makeDisplayList(dlist+6, gleTube2);
        makeDisplayList(dlist+7, gleLongTube1);
        makeDisplayList(dlist+8, gleLongTube2);
        makeDisplayList(dlist+9, gleCone1);
        makeDisplayList(dlist+10, gleCylinderH);
        makeDisplayList(dlist+11, gleDumbbell1);
        makeDisplayList(dlist+12, gleIcosahedron1);
        makeDisplayList(dlist+13, gleArrowTail1);
        makeDisplayList(dlist+14, gleArrowTail2);
    }
}

void gle::releaseDL()
{
    if ( dlist > 0 )
        glDeleteLists(dlist, 15);
}

#endif


//-----------------------------------------------------------------------
#pragma mark -

#if defined GLE_USES_DISPLAY_LISTS

void gle::gleSphere1B()   { callDisplayList(dlist+2); }
void gle::gleSphere2B()   { callDisplayList(dlist+3); }
void gle::gleSphere4B()   { callDisplayList(dlist+4); }

#else

void gle::gleSphere1B()   { gleSphere1(); }
void gle::gleSphere2B()   { gleSphere2(); }
void gle::gleSphere4B()   { gleSphere4(); }

#endif


//-----------------------------------------------------------------------
#pragma mark -

/**
 Draw a surface of revolution around the Z-axis.
 The surface goes from Z=0 to Z=1, and its radius is
 given by the function `radius`(z) provided as argument.
 */
void gle::gleRevolution(GLfloat (*radius)(GLfloat))
{
    GLfloat r0, z0, z1=0, r1=radius(z1), dr, dn;
    GLfloat dz = 0.25 / (GLfloat) finesse;

    GLfloat s[2*finesse+1], c[2*finesse+1];
    for ( int ii = 0; ii <= 2*finesse; ++ii )
    {
        GLfloat ang = ii * M_PI / (GLfloat) finesse;
        c[ii] = cosf(ang);
        s[ii] = sinf(ang);
    }

    for ( int jj = 0; jj <= 4*finesse; ++jj )
    {
        z0 = z1;
        r0 = r1;
        z1 = jj * dz;
        r1 = radius(z1);

        dr = ( r1 - r0 ) / dz;
        dn = 1.0 / sqrt( 1 + dr * dr );
        dr = dr*dn;

        glBegin(GL_TRIANGLE_STRIP);
        for ( int ii = 0; ii <= 2*finesse; ++ii )
        {
            glNormal3f(dn*c[ii], dn*s[ii], -dr);
            glVertex3f(r1*c[ii], r1*s[ii], z1);
            glVertex3f(r0*c[ii], r0*s[ii], z0);
        }
        glEnd();
    }
}

//-----------------------------------------------------------------------
#pragma mark - Object Placement


/**
 draw back first, and then front of object,
 GL_CULL_FACE should be enabled
 */
void gle::gleDualPass(void primitive())
{
    assert_true(glIsEnabled(GL_CULL_FACE));
    glCullFace(GL_FRONT);
    primitive();
    glCullFace(GL_BACK);
    primitive();
}


void gle::gleObject( const real radius, void (*obj)() )
{
    glPushMatrix();
    gleScale(radius);
    obj();
    glPopMatrix();
}

void gle::gleObject( const Vector1 & x, const real radius, void (*obj)() )
{
    glPushMatrix();
    gleTranslate(x);
    gleScale(radius);
    obj();
    glPopMatrix();
}

void gle::gleObject( const Vector2 & x, const real radius, void (*obj)() )
{
    glPushMatrix();
    gleTranslate(x);
    gleScale(radius);
    obj();
    glPopMatrix();
}

void gle::gleObject( const Vector3 & x, const real radius, void (*obj)() )
{
    glPushMatrix();
    gleTranslate(x);
    gleScale(radius);
    obj();
    glPopMatrix();
}


//-----------------------------------------------------------------------
void gle::gleObject(const Vector1 & a, const Vector1 & b, void (*obj)())
{
    glPushMatrix();
    if ( a.XX < b.XX )
        glRotated(  90, 0.0, 1.0, 0.0 );
    else
        glRotated( -90, 0.0, 1.0, 0.0 );
    obj();
    glPopMatrix();
}

void gle::gleObject(const Vector2 & a, const Vector2 & b, void (*obj)())
{
    glPushMatrix();
    gleAlignZ(a, b);
    obj();
    glPopMatrix();
}

void gle::gleObject(const Vector3 & a, const Vector3 & b, void (*obj)())
{
    glPushMatrix();
    Vector3 dir = b-a;
    const real dn = dir.norm();
    Vector3 P1  = dir.orthogonal(dn);
    Vector3 P2  = vecProd(dir, P1) / dn;
    gleTransRotate( P1, P2, dir, a );
    obj();
    glPopMatrix();
}


//-----------------------------------------------------------------------
void gle::gleObject( const Vector1 & x, const Vector1 & d, const real r, void (*obj)() )
{
    glPushMatrix();
    gleTranslate(x);
    if ( d.XX < 0 )
        glRotated(90, 0, 0, 1);
    gleScale(r);
    obj();
    glPopMatrix();
}

void gle::gleObject( const Vector2 & x, const Vector2 & d, const real r, void (*obj)() )
{
    glPushMatrix();
    gleAlignZ(x, x+d.normalized(r), r);
    obj();
    glPopMatrix();
}

void gle::gleObject( const Vector3 & x, const Vector3 & d, const real r, void (*obj)() )
{
    glPushMatrix();
    Vector3 P1 = d.orthogonal(r);
    Vector3 P2 = vecProd(d.normalized(), P1);
    gleTransRotate( P1, P2, d.normalized(r), x );
    obj();
    glPopMatrix();
}

//-----------------------------------------------------------------------
void gle::gleObject( const Vector1 & x, const Vector1 & d, const real r,
                     const real l, void (*obj)() )
{
    glPushMatrix();
    gleTranslate(x);
    if ( d.XX < 0 )
        glRotated(90, 0, 0, 1);
    gleScale(l,r,r);
    obj();
    glPopMatrix();
}

void gle::gleObject( const Vector2 & x, const Vector2 & d, const real r,
                     const real l, void (*obj)() )
{
    glPushMatrix();
    gleAlignZ(x, x+d.normalized(l), r);
    obj();
    glPopMatrix();
}

void gle::gleObject( const Vector3 & x, const Vector3 & d, const real r,
                     const real l, void (*obj)() )
{
    glPushMatrix();
    Vector3 P1 = d.orthogonal(r);
    Vector3 P2 = vecProd(d.normalized(), P1);
    gleTransRotate(P1, P2, d.normalized(l), x);
    obj();
    glPopMatrix();
}

//-----------------------------------------------------------------------
#pragma mark - Tubes


void gle::gleTube(const Vector1 & a, const Vector1 & b, real radius, void (*obj)())
{
    glPushMatrix();
    if ( a.XX < b.XX )
        glRotated(  90, 0.0, 1.0, 0.0 );
    else
        glRotated( -90, 0.0, 1.0, 0.0 );
    gleScale(1,radius,1);
    obj();
    glPopMatrix();
}

void gle::gleTube(const Vector2 & a, const Vector2 & b, real radius, void (*obj)())
{
    glPushMatrix();
    gleAlignZ(a, b, radius);
    obj();
    glPopMatrix();
}

void gle::gleTube(const Vector3 & a, const Vector3 & b, real radius, void (*obj)())
{
    glPushMatrix();
    Vector3 dir = b-a;
    Vector3 P1  = dir.orthogonal(radius);
    Vector3 P2  = vecProd(dir, P1).normalized(radius);
    gleTransRotate( P1, P2, dir, a );
    obj();
    glPopMatrix();
}


//-----------------------------------------------------------------------
void gle::gleTube(const Vector1 & a, real ra, gle_color ca,
                  const Vector1 & b, real rb, gle_color cb)
{
    glPushMatrix();
    gleTranslate(-a);
    if ( a.XX < b.XX )
        glRotated(  90, 0.0, 1.0, 0.0 );
    else
        glRotated( -90, 0.0, 1.0, 0.0 );
    gleTubeZ(a.XX, ra, ca, b.XX, rb, cb);
    glPopMatrix();
}

void gle::gleTube(const Vector2 & a, real ra, gle_color ca,
                  const Vector2 & b, real rb, gle_color cb)
{
    glPushMatrix();
    gleAlignZ(a, b);
    //gleTube1();
    gleTubeZ(0, ra, ca, 1, rb, cb);
    glPopMatrix();
}

void gle::gleTube(const Vector3 & a, real ra, gle_color ca,
                  const Vector3 & b, real rb, gle_color cb)
{
    glPushMatrix();
    Vector3 dir = b-a;
    Vector3 P1  = dir.orthogonal(1);
    Vector3 P2  = vecProd(dir, P1).normalized();
    gleTransRotate(P1, P2, dir, a);
    gleTubeZ(0, ra, ca, 1, rb, cb);
    glPopMatrix();
}


//-----------------------------------------------------------------------
void gle::gleBand(const Vector1 & a, real ra,
                  const Vector1 & b, real rb)
{
    glBegin(GL_TRIANGLE_FAN);
    gleVertex(a.XX,+ra);
    gleVertex(a.XX,-ra);
    gleVertex(b.XX,-rb);
    gleVertex(b.XX,+rb);
    glEnd();
}

void gle::gleBand(const Vector2 & a, real ra,
                  const Vector2 & b, real rb)
{
    Vector2 d = ( b - a ).orthogonal();
    real n = d.norm();
    if ( n > 0 )
        d /= n;
    glBegin(GL_TRIANGLE_FAN);
    gleVertex(a+ra*d);
    gleVertex(a-ra*d);
    gleVertex(b-rb*d);
    gleVertex(b+rb*d);
    glEnd();
}

void gle::gleBand(const Vector1 & a, real ra, gle_color ca,
                  const Vector1 & b, real rb, gle_color cb)
{
    glBegin(GL_TRIANGLE_FAN);
    ca.color();
    gleVertex(a.XX,+ra);
    gleVertex(a.XX,-ra);
    cb.color();
    gleVertex(b.XX,-rb);
    gleVertex(b.XX,+rb);
    glEnd();
}

void gle::gleBand(const Vector2 & a, real ra, gle_color ca,
                  const Vector2 & b, real rb, gle_color cb)
{
    Vector2 d = ( b - a ).orthogonal();
    real n = d.norm();
    if ( n > 0 )
        d /= n;
    glBegin(GL_TRIANGLE_FAN);
    ca.color();
    gleVertex(a+ra*d);
    gleVertex(a-ra*d);
    cb.color();
    gleVertex(b-rb*d);
    gleVertex(b+rb*d);
    glEnd();
}

/**
 Two hexagons linked by a rectangle
 */
void gle::gleDumbbell(const Vector2 & a, const Vector2 & b, real diameter)
{
    //side of hexagon that has the same surface as the disc or Radius 1.
    const GLfloat S = sqrt( 2 * M_PI / ( 3 * sqrt(3) ));
    const GLfloat R = diameter * S;
    const GLfloat H = R * 0.5 * sqrt(3);
    const GLfloat X = R * 0.5;

    Vector2 x = ( b - a ).normalized(H);
    Vector2 y = x.orthogonal(X);

    glPushMatrix();
    gleTranslate(a);

    // this is an hexagon centered around 'a':
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0,0);
    gleVertex(x+y);
    gleVertex(2*y);
    gleVertex(-x+y);
    gleVertex(-x-y);
    gleVertex(-2*y);
    gleVertex(x-y);
    gleVertex(x+y);
    glEnd();

    // a band from 'a' to 'b'
    glBegin(GL_TRIANGLE_FAN);
    gleVertex(+y+x);
    gleVertex(-y+x);
    gleVertex(b-a-y-x);
    gleVertex(b-a+y-x);
    glEnd();

    // an hexagon centered around 'b'
    gleTranslate(b-a);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0,0);
    gleVertex(x+y);
    gleVertex(2*y);
    gleVertex(-x+y);
    gleVertex(-x-y);
    gleVertex(-2*y);
    gleVertex(x-y);
    gleVertex(x+y);
    glEnd();

    glPopMatrix();
}

//-----------------------------------------------------------------------
#pragma mark - Arrows

void gle::gleCone(const Vector1 & center, const Vector1 & dir, const real scale)
{
    real dx = scale*dir.XX, cx = center.XX;
    glBegin(GL_TRIANGLES);
    gleVertex( cx+dx+dx, 0 );
    gleVertex( cx-dx,    dx );
    gleVertex( cx-dx,   -dx );
    glEnd();
}

void gle::gleCone(const Vector2 & center, const Vector2 & dir, const real scale)
{
    real dx = scale*dir.XX,  cx = center.XX;
    real dy = scale*dir.YY,  cy = center.YY;
    glBegin(GL_TRIANGLES);
    gleVertex( cx+dx+dx, cy+dy+dy );
    gleVertex( cx-dx-dy, cy-dy+dx );
    gleVertex( cx-dx+dy, cy-dy-dx );
    glEnd();
}

void gle::gleCone(const Vector3 & center, const Vector3 & dir, const real scale)
{
    glPushMatrix();
    //build the rotation matrix, assuming dir is normalized
    Vector3 P1 = dir.orthogonal(scale);
    Vector3 P2 = vecProd(dir, P1);
    gleTransRotate( P1, P2, dir*scale, center );
    gleCone1B();
    glPopMatrix();
}

//-----------------------------------------------------------------------

void gle::gleCylinder(const Vector1 & center, const Vector1 & dir, const real scale)
{
    real cx = center.XX;
    glBegin(GL_TRIANGLE_STRIP);
    real dx = 0.5 * scale * dir.XX;
    gleVertex( cx-dx, -scale );
    gleVertex( cx-dx,  scale );
    gleVertex( cx+dx, -scale );
    gleVertex( cx+dx,  scale );
    glEnd();
}

void gle::gleCylinder(const Vector2 & center, const Vector2 & dir, const real scale)
{
    real dx = scale * dir.XX, cx = center.XX - 0.5 * dx;
    real dy = scale * dir.YY, cy = center.YY - 0.5 * dy;
    glBegin(GL_TRIANGLE_STRIP);
    gleVertex( cx+dy, cy-dx );
    gleVertex( cx-dy, cy+dx );
    gleVertex( cx+dx+dy, cy+dy-dx );
    gleVertex( cx+dx-dy, cy+dy+dx );
    glEnd();
}

void gle::gleCylinder(const Vector3 & center, const Vector3 & dir, const real scale)
{
    glPushMatrix();
    //build the rotation matrix, assuming dir is normalized
    Vector3 P1 = dir.orthogonal(scale);
    Vector3 P2 = vecProd(dir, P1);
    gleTransRotate( P1, P2, dir*scale, center );
    gleCylinderHB();
    glPopMatrix();
}


//-----------------------------------------------------------------------

void gle::gleArrowTail(const Vector1 & center, const Vector1 & dir, const real scale)
{
    GLfloat dx = scale * dir.XX;
    GLfloat cx = center.XX - 0.5 * dx;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f( cx,       0  );
    glVertex2f( cx-dx,   -dx );
    glVertex2f( cx+dx,   -dx );
    glVertex2f( cx+dx+dx, 0  );
    glVertex2f( cx+dx,    dx );
    glVertex2f( cx-dx,    dx );
    glEnd();
}

void gle::gleArrowTail(const Vector2 & center, const Vector2 & dir, const real scale)
{
    GLfloat dx = scale * dir.XX;
    GLfloat dy = scale * dir.YY;
    GLfloat cx = center.XX - 1.5 * dx;
    GLfloat cy = center.YY - 1.5 * dy;
    GLfloat ex = cx + 2 * dx;
    GLfloat ey = cy + 2 * dy;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f( cx+dx, cy+dy );
    glVertex2f( cx+dy, cy-dx );
    glVertex2f( ex+dy, ey-dx );
    glVertex2f( ex+dx, ey+dy );
    glVertex2f( ex-dy, ey+dx );
    glVertex2f( cx-dy, cy+dx );
    glEnd();
}

void gle::gleArrowTail(const Vector3 & center, const Vector3 & dir, const real scale)
{
    glPushMatrix();
    //build the rotation matrix, assuming dir is normalized
    Vector3 P1 = dir.orthogonal(scale);
    Vector3 P2 = vecProd(dir, P1);
    gleTransRotate( P1, P2, dir*scale, center );
    gleArrowTail2B();
    glPopMatrix();
}

//-----------------------------------------------------------------------
void gle::gleArrow(const Vector1 & a, const Vector1 & b, real radius)
{
    glPushMatrix();
    if ( a.XX < b.XX )
        glRotated(  90, 0.0, 1.0, 0.0 );
    else
        glRotated( -90, 0.0, 1.0, 0.0 );
    gleScale(1,radius,1);
    gleTube1B();
    glTranslatef(0, 0, 1);
    glScalef(3.0, 3.0, 3*radius);
    gleCone1B();
    glPopMatrix();
}

void gle::gleArrow(const Vector2 & a, const Vector2 & b, real radius)
{
    glPushMatrix();
    gleAlignZ(a, b, radius);
    gleTube1B();
    glTranslatef(0, 0, 1);
    glScalef(3.0, 3.0, 3*radius);
    gleCone1B();
    glPopMatrix();
}

void gle::gleArrow(const Vector3 & a, const Vector3 & b, real radius)
{
    glPushMatrix();
    Vector3 dir = b-a;
    Vector3 P1  = dir.orthogonal(radius);
    Vector3 P2  = vecProd(dir, P1).normalized(radius);
    gleTransRotate( P1, P2, dir, a );
    gleTube1B();
    glTranslatef(0, 0, 1);
    glScalef(3.0, 3.0, 3*radius);
    gleCone1B();
    glPopMatrix();
}


//-----------------------------------------------------------------------
#pragma mark - Text


int gle::gleLineHeight(void* font)
{
    if ( font == GLUT_BITMAP_8_BY_13 )        return 13;
    if ( font == GLUT_BITMAP_9_BY_15 )        return 15;
    if ( font == GLUT_BITMAP_TIMES_ROMAN_10 ) return 11;
    if ( font == GLUT_BITMAP_TIMES_ROMAN_24 ) return 26;
    if ( font == GLUT_BITMAP_HELVETICA_10 )   return 11;
    if ( font == GLUT_BITMAP_HELVETICA_12 )   return 15;
    if ( font == GLUT_BITMAP_HELVETICA_18 )   return 22;
    return 13;
}


/**
 Compute the max width of all the lines in the given text
 This uses GLUT, which should be initialized.
*/
int gle::gleComputeTextSize(const char text[], void* font, int& lines)
{
    int width = 0;
    lines = 0;
    int w = 0;
    for (const char* c = text; *c != '\0' ; ++c)
    {
        if ( *c == '\n' )
        {
            if ( w > width ) width = w;
            ++lines;
            w = 0;
        }
        else if ( isprint(*c))
        {
            w += glutBitmapWidth(font, *c);
        }
    }
    if ( w > width )
        width = w;
    if ( width > 0 && lines == 0 )
        lines = 1;
    return width;
}

//-----------------------------------------------------------------------
/**
 draw the string character per character using:
 glutBitmapCharacter()
 */
void gle::gleDrawText(const char text[], void* font, GLfloat vshift)
{
    assert_true(font);

    GLfloat ori[4], pos[4];
    glGetFloatv(GL_CURRENT_RASTER_POSITION, ori);

    for (const char* p = text; *p; ++p)
    {
        if ( *p == '\n' )
        {
            glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
            glBitmap(0, 0, 0, 0, ori[0]-pos[0], vshift, 0);
        }
        else if ( isprint(*p) )
        {
            glutBitmapCharacter(font, *p);
        }
    }
}


/**
 set the current raster position to `w`
 */
void gle::gleDrawText(const Vector3& vec, const char text[], void* font)
{
    assert_true(font);
    glPushAttrib(GL_LIGHTING_BIT|GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    gleRasterPos(vec);
    gleDrawText(text, font, -gleLineHeight(font));
    glPopAttrib();
}

void gle::gleDrawText(const Vector1& w, const char text[], void* font)
{
    gleDrawText(Vector3(w.XX, 0, 0 ), text, font);
}

void gle::gleDrawText(const Vector2& w, const char text[], void* font)
{
    gleDrawText(Vector3(w.XX, w.YY, 0 ), text, font);
}

//-----------------------------------------------------------------------

/**
 The text is displayed in the current color.
 A background rectangle is displayed only if `bcol` is visible.

 @code
 glColor3f(1,1,1);
 gleDisplayText(fKeyString, GLUT_BITMAP_8_BY_13, 0x0, 1);
 @endcode

 Possible values for `position`:
 - 0: bottom-left, text going up
 - 1: bottom-right, text going up
 - 2: top-right, text going down
 - 3: top-left, text going down
 - 4: center, text going down
 .

 Note: width and height are the current size of the viewport (window)
 */
void gle::gleDisplayText(const char text[], void* font, const gle_color bcol,
                         const int position, int width, int height)
{
    assert_true( width > 0 );
    assert_true( height > 0 );

    int lineHeight = gleLineHeight(font);
    int textWidth = 0;
    int nblines = 1;

    GLint px, py;
    switch( position )
    {
        case 0: {
            //bottom-left, text going up
            px = 0.5*lineHeight;
            py = 0.5*lineHeight;
        } break;
        case 1: {
            //bottom-right, text going up
            textWidth = gleComputeTextSize(text, font, nblines);
            px = width - textWidth - 0.5*lineHeight;
            if ( px < 0 ) px = 0;
            py = 0.5*lineHeight;
        } break;
        case 2: {
            //top-right, text going down
            textWidth = gleComputeTextSize(text, font, nblines);
            px = width - textWidth - 0.5*lineHeight;
            if ( px < 0 ) px = 0;
            py = height - lineHeight;
            lineHeight *= -1;
        } break;
        default:
        case 3: {
            //top-left, text going down
            px = 0.5*lineHeight;
            py = height - lineHeight;
            lineHeight *= -1;
        } break;
        case 4: {
            //center, text going down
            textWidth = gleComputeTextSize(text, font, nblines);
            px = ( width - textWidth ) / 2;
            if ( px < 0 ) px = 0;
            py = ( height + nblines*lineHeight ) / 2;
            lineHeight *= -1;
        } break;
    }

    //set pixel coordinate system:

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, 0, 1);

    glRasterPos2i(0, 0);
    glBitmap(0, 0, 0, 0, px, py, 0);

    glPushAttrib(GL_LIGHTING_BIT|GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);

    if ( bcol.visible() )
    {
        GLfloat col[4] = { 1 };
        glGetFloatv(GL_CURRENT_COLOR, col);
        int rd = std::abs(lineHeight);
        int bt = py;
        int tp = py + nblines*lineHeight;
        if ( lineHeight < 0 )
        {
            int x = tp;
            tp = bt;
            bt = x;
        }

        int rect[4] = { px-rd, bt, px+textWidth+rd, static_cast<int>(tp+1.75*rd) };

        bcol.color();
        glBegin(GL_TRIANGLE_FAN);
        gleNiceRectangle(rect, 4);
        glEnd();

        glColor4fv(col);

        glLineWidth(1);
        glBegin(GL_LINE_STRIP);
        gleNiceRectangle(rect, 4);
        glEnd();
    }

    gleDrawText(text, font, lineHeight);

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
}


//-----------------------------------------------------------------------
#pragma mark - Misc

/**
 Draw an array of pixels using GL_TRIANGLE_STRIP

 The array rgba[] should ( nbc * width * height ) bytes,
 containing nbc-components (eg. RGBA) per pixel and
 stored by columns:

 @code
 color(i,j) = rgba[ nbc*(i+height*j) ]
 0 <= i < height
 0 <= j < width
 @endcode

 `pos` is the position of the top-left corner
 `dx` is the direction of the width
 `dy` is the direction of the height
 The magnitudes of `dx` and `dy` indicates the dimensions of a pixel.
 They may be of different magnitudes, and not necessarily orthogonal.
 */
void gle::gleDrawPixels(int width, int height, int nbc, GLubyte rgba[], Vector2 pos, Vector2 dx, Vector2 dy)
{
    glPushAttrib(GL_ENABLE_BIT|GL_POLYGON_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);

    Vector2 left, right;
    GLubyte * col = rgba;

    for ( int jj = 0; jj < width; ++jj )
    {
        left  = pos + dx * jj;
        right = left + dx;
        for ( int ii = 0; ii < height; ++ii )
        {
            if ( nbc == 3 )
                glColor3ubv(col);
            else
                glColor4ubv(col);
            glBegin(GL_TRIANGLE_STRIP);
            col += nbc;
            gle::gleVertex(left);
            gle::gleVertex(right);
            left  += dy;
            right += dy;
            gle::gleVertex(left);
            gle::gleVertex(right);
            glEnd();
        }
    }

    glPopAttrib();
}

//-----------------------------------------------------------------------


/**
 rectangle should be specified as [ left, bottom, right, top ]

 The rectangle will be drawn counter-clockwise
*/
void gle::gleRectangle(const int rect[4])
{
    glVertex2i(rect[0], rect[1]);
    glVertex2i(rect[2], rect[1]);
    glVertex2i(rect[2], rect[3]);
    glVertex2i(rect[0], rect[3]);
    glVertex2i(rect[0], rect[1]);
}


void gle::gleNiceRectangle(const int rect[4], const int rad)
{
    glVertex2i(rect[0], rect[1]+rad);
    glVertex2i(rect[0]+rad, rect[1]);
    glVertex2i(rect[2]-rad, rect[1]);
    glVertex2i(rect[2], rect[1]+rad);
    glVertex2i(rect[2], rect[3]-rad);
    glVertex2i(rect[2]-rad, rect[3]);
    glVertex2i(rect[0]+rad, rect[3]);
    glVertex2i(rect[0], rect[3]-rad);
    glVertex2i(rect[0], rect[1]+rad);
}


void gle::gleDrawRectangle(const int rect[4], int width, int height)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, 0, 1);
    //disable advanced features
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glBegin(GL_LINE_LOOP);
    gleRectangle(rect);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
}


void gle::gleDrawResizeBox(const gle_color rgb, int width, int height)
{
    //set the matrices
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(width, 0, 0, height, 0, 1 );

    //draw lines at 45 degrees
    rgb.color();
    glBegin(GL_LINES);
    glVertex2i( 16,  1 );    glVertex2i( 1,  16 );
    glVertex2i( 12,  1 );    glVertex2i( 1,  12 );
    glVertex2i( 8,   1 );    glVertex2i( 1,   8 );
    glVertex2i( 4,   1 );    glVertex2i( 1,   4 );
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


//-----------------------------------------------------------------------
void gle::gleDrawAxes(const GLfloat S, int dim)
{
    const GLfloat R = S * 0.1;

    glMatrixMode(GL_MODELVIEW);

    for( int ii=0; ii<dim; ++ii)
    {
        glPushMatrix();
        switch( ii )
        {
            case 0:
                glColor3f(1, 0, 0);
                glRotatef(+90, 0, 1, 0);
                break;
            case 1:
                glColor3f(0, 1, 0);
                glRotatef(-90, 1, 0, 0);
                break;
            case 2:
                glColor3f(0, 0, 1);
                break;
        }
        glScalef(R/4, R/4, S-R);
        gleTube1B();
        glTranslatef(0, 0, 1);
        glScalef(4, 4, R/(S-R));
        gleCone1B();
        glRasterPos3d(0, 0, 2);
        glNormal3f(0, 0, 1);
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 'X'+ii);
        glPopMatrix();
    }

    // display a white ball at the origin
    glColor3f(1, 1, 1);
    glPushMatrix();
    gleScale(R);
    gleSphere4B();
    glPopMatrix();
}

//-----------------------------------------------------------------------
/**
 This is similart to glutReportError,
 but the additional argument can provide useful feedback for debugging
 */
void gle::gleReportErrors(FILE * out, const char* msg)
{
    GLenum glError = glGetError();
    while ( glError != GL_NO_ERROR )
    {
        fprintf(out, "OpenGL error `%s' in %s\n", gluErrorString(glError), msg);
        glError = glGetError();
    }
}
