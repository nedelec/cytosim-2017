// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef GLE_H
#define GLE_H

#include "real.h"
#include "opengl.h"
#include "gle_color.h"
#include "vector1.h"
#include "vector2.h"
#include "vector3.h"

#define GLE_USES_DISPLAY_LISTS

/// Simple geometrical objects drawn with OpenGL
/**
 @todo namespace gle should be a class -> we use GL.vertex(v)
 Problem: the gle prefix is already used by standard OpenGL elements
 */
namespace gle
{
    /// this defines the number of triangles used to draw shapes, bigger = nicer
    /** finesse = 8 is good, 16 is nice, 32 should be very nice.
     To be able to change it during a run, we would need to update all the display lists */
    const int finesse = 8;

    /// initialize the arrays
    void initialize();
    
    /// release any memory
    void release();
    
    /// initialize the Display lists
    void initializeDL();
    
    /// release Display lists
    void releaseDL();
    
#pragma mark -
    
#ifdef REAL_IS_FLOAT
    
    inline void gleScale(real x)                          { glScalef(x,x,x); }
    inline void gleScale(real x, real y, real z)          { glScalef(x,y,z); }
    
    inline void gleVertex(const Vector1 & v)              { glVertex2f(v.XX, 0); }
    inline void gleVertex(const Vector2 & v)              { glVertex2f(v.XX, v.YY); }
    inline void gleVertex(const Vector3 & v)              { glVertex3f(v.XX, v.YY, v.ZZ); }
    inline void gleVertex(real x, real y)                 { glVertex2f(x, y); }
    inline void gleVertex(real x, real y, real z)         { glVertex3f(x, y, z); }

    inline void gleNormal(const Vector1 & v)              { glNormal3f(v.XX, 0, 0); }
    inline void gleNormal(const Vector2 & v)              { glNormal3f(v.XX, v.YY, 0); }
    inline void gleNormal(const Vector3 & v)              { glNormal3f(v.XX, v.YY, v.ZZ); }
    
    inline void gleTranslate(const Vector1 & v)           { glTranslatef(v.XX, 0, 0); }
    inline void gleTranslate(const Vector2 & v)           { glTranslatef(v.XX, v.YY, 0); }
    inline void gleTranslate(const Vector3 & v)           { glTranslatef(v.XX, v.YY, v.ZZ); }
    
    inline void gleTranslate(real x, real y, real z)      { glTranslatef(x, y, z); }
    inline void gleRotate(real a, real x, real y, real z) { glRotatef(a, x, y, z); }
    inline void gleLoadMatrix(real * m)                   { glLoadMatrixf(m); }
    
    inline void gleRasterPos(const Vector1 & v)           { glRasterPos2f(v.XX, 0); }
    inline void gleRasterPos(const Vector2 & v)           { glRasterPos2f(v.XX, v.YY); }
    inline void gleRasterPos(const Vector3 & v)           { glRasterPos3f(v.XX, v.YY, v.ZZ); }
    
#else
    
    inline void gleScale(real x)                          { glScaled(x,x,x); }
    inline void gleScale(real x, real y, real z)          { glScaled(x,y,z); }
    
    inline void gleVertex(const Vector1 & v)              { glVertex2d(v.XX, 0); }
    inline void gleVertex(const Vector2 & v)              { glVertex2d(v.XX, v.YY); }
    inline void gleVertex(const Vector3 & v)              { glVertex3d(v.XX, v.YY, v.ZZ); }
    inline void gleVertex(real x, real y)                 { glVertex2d(x, y); }
    inline void gleVertex(real x, real y, real z)         { glVertex3d(x, y, z); }

    inline void gleNormal(const Vector1 & v)              { glNormal3d(v.XX, 0, 0); }
    inline void gleNormal(const Vector2 & v)              { glNormal3d(v.XX, v.YY, 0); }
    inline void gleNormal(const Vector3 & v)              { glNormal3d(v.XX, v.YY, v.ZZ); }
    
    inline void gleTranslate(const Vector1 & v)           { glTranslated(v.XX, 0, 0); }
    inline void gleTranslate(const Vector2 & v)           { glTranslated(v.XX, v.YY, 0); }
    inline void gleTranslate(const Vector3 & v)           { glTranslated(v.XX, v.YY, v.ZZ); }
    
    inline void gleTranslate(real x, real y, real z)      { glTranslated(x, y, z); }
    inline void gleRotate(real x, real y, real z, real t) { glRotated(x, y, z, t); }
    inline void gleLoadMatrix(real * m)                   { glLoadMatrixd(m); }
    
    inline void gleRasterPos(const Vector1 & v)           { glRasterPos2d(v.XX, 0); }
    inline void gleRasterPos(const Vector2 & v)           { glRasterPos2d(v.XX, v.YY); }
    inline void gleRasterPos(const Vector3 & v)           { glRasterPos3d(v.XX, v.YY, v.ZZ); }

#endif
    
    inline void gleVertex3v(const float* v)               { glVertex3fv(v); }
    inline void gleVertex3v(const double* v)              { glVertex3dv(v); }
    inline void gleNormal3v(const float* v)               { glNormal3fv(v); }
    inline void gleNormal3v(const double* v)              { glNormal3dv(v); }

    //------------------------------------------------------------------------------
#pragma mark -
        
    /// align the X-axis to the given vector, by rotating around Z
    void gleAlignX(const Vector2 & v1);
    /// translate by A, then rotate to align Z with AB (which is in the XY-plane)
    void gleAlignZ(const Vector2 & A, const Vector2 & B);
    /// translate by A, then rotate to align Z with AB, Z replaces X. The X-Y plane is scaled by ts
    void gleAlignZ(const Vector2 & A, const Vector2 & B, real ts);
    ///  align the view to the three orthogonal vectors given
    void gleRotate(const Vector3 & v1, const Vector3 & v2, const Vector3 & v3, bool inverse=false);
    /// translate by T, then rotate to align X with v1, Y with v2 and Z with v3
    void gleTransRotate(const Vector3 & v1, const Vector3 & v2, const Vector3 & v3, const Vector3 & T);
    
    //------------------------------------------------------------------------------
#pragma mark -

    /// call glVertex() along a centered 2D circle of radius 1 in plane XY
    void gleCircle0();
    void gleCircleL();
    void gleCircleS();
    /// draw a triangle of radius 1 in plane XY, normals pointing in +Z
    void gleTriangle0();
    void gleTriangleS();
    void gleTriangleL();
    /// draw a triangle of radius 1 in plane XY, normals pointing in +Z
    void gleNabla0();
    void gleNablaS();
    void gleNablaL();
    /// draw a square of radius 1 in plane XY, normals pointing in +Z
    void gleSquare0();
    void gleSquareL();
    void gleSquareS();
    /// draw a square of radius 1 in plane XY, normals pointing in +Z
    void gleRectangle0();
    void gleRectangleL();
    void gleRectangleS();
    /// draw a PLUS of radius 1 in plane XY, normals pointing in +Z
    void glePlusS();
    void glePlusL();    
    /// draw a pentagon of radius 1 in plane XY, normals pointing in +Z
    void glePentagon0();
    void glePentagonS();
    void glePentagonL();
    /// draw an hexagon of surface M_PI in plane XY, normals pointing in +Z
    void gleHexagon0();
    void gleHexagonS();
    void gleHexagonL();
    /// draw a star of radius 1 in plane XY, normals pointing in +Z
    void gleStar0();
    void gleStarS();
    void gleStarL();
    
    /// draw a sphere of radius 1 at origin, using a refined icosahedron
    void gleSphereN(int);
    /// draw a sphere of radius 1 at origin
    void gleSphere1();
    /// draw a nice sphere of radius 1 at origin
    void gleSphere2();
    /// draw a very nice sphere of radius 1 at origin
    void gleSphere4();
    /// draw a icosahedron of radius 1
    void gleIcosahedron1();
    /// a tube from `a` to `b` , radius 1 drawn with finesse `fin`
    void gleTube0(GLfloat a, GLfloat b, int fin);
    /// draw an open tube along Z, of diameter 1 and length 1, Z=[0, 1]
    void gleTube1();
    /// draw a tube along Z, of diameter 1 and length 2, Z=[-0.5, 1.5]
    void gleLongTube1();
    /// draw a nicer open tube along Z, of diameter 1 and length 1
    void gleTube2();
    /// draw a tube along Z, of diameter 1 and length 2, Z=[-0.5, 1.5]
    void gleLongTube2();
    /// draw a closed tube along Z, or diameter 1 and length 1
    void gleCylinder1();
    /// draw a tube along Z, of length 1, with specified radius and colors at the extermities
    void gleTubeZ(GLfloat za, GLfloat ra, gle_color, GLfloat zb, GLfloat rb, gle_color);
    /// draw a 3-portion cylinder with a larger central section
    void gleBarrel1();
    /// display a cone directed along Z, of radius 1 in Z=[-1, +2]
    void gleCone1();
    /// display an flat-head at the origin, directed along Z, of length 1, radius 1 in Z=[-0.5, +0.5]
    void gleCylinderH();
    /// display a dumbbell align with the Z axis, or radius 1/3, lenth 1
    void gleDumbbell1();
    /// display a arrowtail aligned with the Z axis, or radius 1, lenth 2, Z=[-0.5, 1.5]
    void gleArrowTail1();
    /// display 3 arrow fins aligned with the Z axis, or radius 1, lenth 2, Z=[-0.5, 1.5]
    void gleArrowTail2();

    /// draw a circular band composed of triangles
    void gleArrowedBand(GLfloat width);
    /// gleSphere4() with 3 ArrowedBands separating 8 quadrants
    void gleDecoratedSphere();
    
    /// a rectangle ( rect = [ left, bottom, right, top ] )
    void gleRectangle(const int rect[4]);
    
    /// a rectangle with cut corners
    void gleNiceRectangle(const int rect[4], int);

    //------------------------------------------------------------------------------
    
#ifndef GLE_USES_DISPLAY_LISTS
    
    inline void gleCircleLB()      { gleCircleL();      }
    inline void gleCircleSB()      { gleCircleS();      }
    inline void gleTube1B()        { gleTube1();        }
    inline void gleTube2B()        { gleTube2();        }
    inline void gleLongTube1B()    { gleLongTube1();    }
    inline void gleLongTube2B()    { gleLongTube2();    }
    inline void gleCone1B()        { gleCone1();        }
    inline void gleCylinderHB()    { gleCylinderH();    }
    inline void gleDumbbell1B()    { gleDumbbell1();    }
    inline void gleIcosahedron1B() { gleIcosahedron1(); }
    inline void gleArrowTail1B()   { gleArrowTail1();   }
    inline void gleArrowTail2B()   { gleArrowTail2();   }
    
#else
    
    extern GLuint dlist;

    inline void gleCircleLB()      { glCallList(dlist+0); }
    inline void gleCircleSB()      { glCallList(dlist+1); }
    inline void gleTube1B()        { glCallList(dlist+5); }
    inline void gleTube2B()        { glCallList(dlist+6); }
    inline void gleLongTube1B()    { glCallList(dlist+7); }
    inline void gleLongTube2B()    { glCallList(dlist+8); }
    inline void gleCone1B()        { glCallList(dlist+9); }
    inline void gleCylinderHB()    { glCallList(dlist+10); }
    inline void gleDumbbell1B()    { glCallList(dlist+11); }
    inline void gleIcosahedron1B() { glCallList(dlist+12); }
    inline void gleArrowTail1B()   { glCallList(dlist+13); }
    inline void gleArrowTail2B()   { glCallList(dlist+14); }

#endif
    
    void gleSphere1B();
    void gleSphere2B();
    void gleSphere4B();

    //------------------------------------------------------------------------------
#pragma mark -
    
    /// display a surface of revolution around the Z axis
    void gleRevolution(GLfloat (*radius)(GLfloat));
    
    /// display back first, and then front
    void gleDualPass(void primitive());
    
    /// draw the object specified by obj, scaled by radius
    void gleObject(real radius, void (*obj)());
    
    /// draw 'obj' scaled by radius at position 'x'
    void gleObject(const Vector1 & x, real radius, void (*obj)());
    void gleObject(const Vector2 & x, real radius, void (*obj)());
    void gleObject(const Vector3 & x, real radius, void (*obj)());
 
    /// draw 'obj' scaled by radius at position 'x', oriented along 'd'
    void gleObject(const Vector1 & x, const Vector1 & d, real radius, void (*obj)());
    void gleObject(const Vector2 & x, const Vector2 & d, real radius, void (*obj)());
    void gleObject(const Vector3 & x, const Vector3 & d, real radius, void (*obj)());

    /// draw 'obj' scaled by radius at position 'x', oriented along 'd'
    void gleObject(const Vector1 & a, const Vector1 & b, void (*obj)());
    void gleObject(const Vector2 & a, const Vector2 & b, void (*obj)());
    void gleObject(const Vector3 & a, const Vector3 & b, void (*obj)());
    
    /// draw 'obj' scaled by radius at position 'x', oriented along 'd'
    void gleObject(const Vector1 & x, const Vector1 & d, real radius, real length, void (*obj)());
    void gleObject(const Vector2 & x, const Vector2 & d, real radius, real length, void (*obj)());
    void gleObject(const Vector3 & x, const Vector3 & d, real radius, real length, void (*obj)());

    //------------------------------------------------------------------------------

    /// draw 'obj' with its ends at [a,b], of specified radius
    void gleTube(const Vector1 & a, const Vector1 & b, real radius, void (*obj)()=gleTube1B);
    void gleTube(const Vector2 & a, const Vector2 & b, real radius, void (*obj)()=gleTube1B);
    void gleTube(const Vector3 & a, const Vector3 & b, real radius, void (*obj)()=gleTube1B);
    
    /// draw an open tube between A and B, with specified color and radius in A and B
    void gleTube(const Vector1 & a, real, gle_color, const Vector1 & b, real, gle_color);
    void gleTube(const Vector2 & a, real, gle_color, const Vector2 & b, real, gle_color);
    void gleTube(const Vector3 & a, real, gle_color, const Vector3 & b, real, gle_color);
    
    /// draw a band from A to B, with specified radius in A and B
    void gleBand(const Vector1 & a, real, const Vector1 & b, real);
    void gleBand(const Vector2 & a, real, const Vector2 & b, real);
    
    /// draw a band from A to B, with specified radius and colors in A and B
    void gleBand(const Vector1 & a, real, gle_color, const Vector1 & b, real, gle_color);
    void gleBand(const Vector2 & a, real, gle_color, const Vector2 & b, real, gle_color);

    /// draw two discs in A and B, connected with a line
    void gleDumbbell(const Vector2 & a, const Vector2 & b, real diameter);
    /// draw two spheres in A and B, connected with a cylinder
    void gleDumbbell(const Vector3 & a, const Vector3 & b, real diameter);

    /// display cone, dir should be normalized
    void gleCone(const Vector1 & center, const Vector1 & dir, real scale);
    /// display arrow-head, dir should be normalized
    void gleCone(const Vector2 & center, const Vector2 & dir, real scale);
    /// display arrow-head, dir should be normalized
    void gleCone(const Vector3 & center, const Vector3 & dir, real scale);
    
    /// display arrow-head, dir should be normalized
    void gleCylinder(const Vector1 & center, const Vector1 & dir, real scale);
    /// display arrow-head, dir should be normalized
    void gleCylinder(const Vector2 & center, const Vector2 & dir, real scale);
    /// display arrow-head, dir should be normalized
    void gleCylinder(const Vector3 & center, const Vector3 & dir, real scale);

    /// display arrow-head, dir should be normalized
    void gleArrowTail(const Vector1 & center, const Vector1 & dir, real scale);
    /// display arrow-head, dir should be normalized
    void gleArrowTail(const Vector2 & center, const Vector2 & dir, real scale);
    /// display arrow-head, dir should be normalized
    void gleArrowTail(const Vector3 & center, const Vector3 & dir, real scale);

    /// draw an arrow with ends [a,b], of specified radius
    void gleArrow(const Vector1 & a, const Vector1 & b, real radius);
    void gleArrow(const Vector2 & a, const Vector2 & b, real radius);
    void gleArrow(const Vector3 & a, const Vector3 & b, real radius);

    //------------------------------------------------------------------------------
#pragma mark -

    /// return height in pixel of GLUT font
    int  gleLineHeight(void* font);

    /// compute size of text
    int gleComputeTextSize(const char text[], void* font, int& lines);
    
    /// display text on a rectangle of color `bcol`, in a corner of the center of the display window
    void gleDisplayText(const char text[], void* font, gle_color bcol, int position, int width, int height);
    
    /// draw text at the current OpenGL raster position (specified by glRasterPos)
    void gleDrawText(const char text[], void* font, GLfloat vshift = -13);
    
    /// draw `text` at position `pos`
    void gleDrawText(const Vector1& pos, const char text[], void* font);
    /// draw `text` at position `pos`
    void gleDrawText(const Vector2& pos, const char text[], void* font);
    /// draw `text` at position `pos`
    void gleDrawText(const Vector3& pos, const char text[], void* font);
                        
    //------------------------------------------------------------------------------
#pragma mark -
    
    /// draw pixel array `rgba` containing 4 bytes per pixels
    void gleDrawPixels(int width, int height, int nbc, GLubyte rgba[], Vector2 pos, Vector2 dx, Vector2 dy);
    
    /// display rectangle specified in pixel-coordinates
    void gleDrawRectangle(const int rect[4], int window_width, int window_height);
    
    /// draw a rectangle to indicate the GLUT window-resize handle
    void gleDrawResizeBox(gle_color, int window_width, int window_height);
    
    /// draw a set of 2 or 3 axes, depending on `dim`
    void gleDrawAxes(GLfloat size, int dim=3);
    
    /// check and print OpenGL error(s)
    void gleReportErrors(FILE*, const char* msg);
    
}


#endif
