// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "point_disp.h"
#include "glossary.h"
#include "opengl.h"
#include "glut.h"
#include "gle.h"
#include <cctype>

//#define POINTDISP_USES_PIXEL_BUFFERS


PointDisp::PointDisp(const std::string& k, const std::string& n)
: Property(n)
{
    mKind=k;
#ifdef POINTDISP_USES_BITMAPS
    bmA    = 0;
    pbo[0] = 0;
    pbo[1] = 0;
#endif
    clear();
}


PointDisp::PointDisp(PointDisp const& o) : Property(o)
{
    visible      = o.visible;
    color        = o.color;
    color2       = o.color2;
    coloring     = o.coloring;
    size         = o.size;
    width        = o.width;
    shape        = o.shape;
    style        = o.style;
    symbol       = o.symbol;
    symbol_color = o.symbol_color;

#ifdef POINTDISP_USES_BITMAPS
    bmA    = 0;
    pbo[0] = 0;
    pbo[1] = 0;
#endif
}


PointDisp& PointDisp::operator =(PointDisp const& o)
{
    visible      = o.visible;
    color        = o.color;
    color2       = o.color2;
    coloring     = o.coloring;
    size         = o.size;
    width        = o.width;
    shape        = o.shape;
    style        = o.style;
    symbol       = o.symbol;
    symbol_color = o.symbol_color;
    
#ifdef POINTDISP_USES_BITMAPS
    bmA    = 0;
    pbo[0] = 0;
    pbo[1] = 0;
#endif
    
    return *this;
}


PointDisp::~PointDisp()
{
#ifdef POINTDISP_USES_BITMAPS
    if ( bmA )
    {
        delete(bmA);
        bmA = 0;
    }
    
#ifdef POINTDISP_USES_PIXEL_BUFFERS
    if ( pbo[0] )
    {
        glDeleteBuffers(2, pbo);
        pbo[0] = 0;
        pbo[1] = 0;
    }
#endif
#endif
}


void PointDisp::clear()
{
    visible      = 1;
    color        = 0x888888FF;
    color2       = 0x777777FF;
    coloring     = 0;
    size         = 4;
    width        = 2;
    shape        = 'o';
    style        = 15;
    symbol       = 0;
    symbol_color = 0xFFFFFFFF;
}


void PointDisp::drawVectorA() const
{
    color.color();

    switch ( shape )
    {
        case 'v': gle::gleNablaS();    break;
        case 't': gle::gleTriangleS(); break;
        case 'q': gle::gleSquareS();   break;
        case 'p': gle::glePentagonS(); break;
        case 'h': gle::gleHexagonS();  break;
        case 's': gle::gleStarS();     break;
        default:  gle::gleCircleSB();  break;
    }
    
    if ( symbol )
    {
        glLineWidth(2);
        glScalef(1.0/80, 1.0/80, 1);
        /*  glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, C) stokes
         character C of width ~104.76 units, and ~150 unit high max
         The translation brings it near the center. */
        if ( islower(symbol) )
            glTranslatef(-52.35, -35, 0);
        else
            glTranslatef(-52.35, -50, 0);
        symbol_color.color();
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, symbol);
    }
}

void PointDisp::drawVectorI() const
{
    color2.color();
    
    switch ( shape )
    {
        case 'v': gle::gleNablaS();    break;
        case 't': gle::gleTriangleS(); break;
        case 'q': gle::gleSquareS();   break;
        case 'p': gle::glePentagonS(); break;
        case 'h': gle::gleHexagonS();  break;
        case 's': gle::gleStarS();     break;
        default:  gle::gleCircleSB();  break;
    }
    
    glScalef(0.6, 0.6, 0.6);
    glDisable(GL_ALPHA_TEST);
    glColor4f(0,0,0,0);
    gle::gleCircleSB();
    glEnable(GL_ALPHA_TEST);
}


#pragma mark -
#pragma mark Bitmaps

#ifdef POINTDISP_USES_BITMAPS

void PointDisp::allocateBitmap(real uFactor)
{
    unsigned d = bmD;
    
    bmD = ceil(uFactor*size);
    bmR = 0.5 * bmD;
    bmS = bmD * bmD;
    
    // allocate only if size has changed
    if ( !bmA || d != bmD )
    {
        if ( bmA ) delete(bmA);
    
        bmA = new GLubyte[8*bmS];
        bmI = bmA + 4*bmS;
        
        for ( unsigned y = 0; y < 8*bmS; ++y )
            bmA[y] = 0;
    }
}

void PointDisp::getBitmap(GLubyte* bitmap, GLuint pbi)
{
    glReadPixels(0, 0, bmD, bmD, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
#ifdef POINTDISP_USES_PIXEL_BUFFERS
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbi);
    glBufferData(GL_PIXEL_PACK_BUFFER_ARB, 4*bmS, 0, GL_STATIC_DRAW);
    glReadPixels(0, 0, bmD, bmD, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
#endif
}


void PointDisp::drawBitmap(GLubyte* bitmap, GLuint pbi) const
{
    glBitmap(0,0,0,0,-bmR,-bmR,0);
#ifdef POINTDISP_USES_PIXEL_BUFFERS
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pbi);
    glDrawPixels(bmD, bmD, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
#else
    glDrawPixels(bmD, bmD, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
#endif
}


void PointDisp::makeBitmaps(real uFactor)
{
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    glOrtho(0, vp[2], 0, vp[3], 0, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    glLoadIdentity();
    glTranslatef(bmR, bmR, 0);
    real s = 0.5 * uFactor * size;
    glScalef(s, s, s);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawVectorA();
    getBitmap(bmA, pbo[0]);

    glLoadIdentity();
    glTranslatef(bmR, bmR, 0);
    glScalef(s, s, s);
    glClear(GL_COLOR_BUFFER_BIT);
    drawVectorI();
    getBitmap(bmI, pbo[1]);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
    
    /// check and print OpenGL error(s)
    gle::gleReportErrors(stderr, "PointDisp::prepare()");

    //std::cerr << name() << " has bitmaps of size " << bmD << "  " << bmR << std::endl;
}


void PointDisp::prepare(real uFactor)
{
#ifdef POINTDISP_USES_PIXEL_BUFFERS
    if ( pbo[0] == 0 )
        glGenBuffers(2, pbo);
    assert_true(pbo[0]);
#endif
    
    allocateBitmap(uFactor);
    makeBitmaps(uFactor);
}


#endif

#pragma mark -
#pragma mark I/O



void PointDisp::read(Glossary& glos)
{
    glos.set(visible,      "visible", KeyList<int>("yes", 1, "no", 0, "transparent",  -1));
    glos.set(color,        "color");
    if ( glos.set(color,   "color") )
        color2 = color;
    glos.set(color2,       "color", 1);
    glos.set(size,         "size");
    // alternative syntax:
    glos.set(size,         "point");
    glos.set(color,        "point", 1);
    
    glos.set(coloring,     "coloring");
    glos.set(width,        "width");
    glos.set(style,        "style");
    glos.set(shape,        "shape");
    glos.set(symbol,       "symbol");
    glos.set(symbol_color, "symbol", 1);

    if ( ! isprint(symbol) )
        symbol = 0;
}


void PointDisp::write_data(std::ostream & os) const
{
    write_param(os, "visible",  visible);
    write_param(os, "color",    color, color2);
    write_param(os, "coloring", coloring);
    write_param(os, "size",     size);
    write_param(os, "width",    width);
    write_param(os, "shape",    shape);
    write_param(os, "style",    style);
    write_param(os, "symbol",   symbol, symbol_color);
}

