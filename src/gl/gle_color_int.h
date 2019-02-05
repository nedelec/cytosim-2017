// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef GLE_COLOR_INT_H
#define GLE_COLOR_INT_H


#include "opengl.h"
#include <iostream>



/// RGBA (Red, Green, Blue, Alpha) color, with one byte per component
class gle_color_int
{
    static uint32_t combine(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
    {
        return ( uint32_t(r) << 24 ) | ( uint32_t(g) << 16 ) | ( uint32_t(b) << 8 ) | uint32_t(a);
    }

private:

    uint32_t rgba;

public:

    gle_color_int()           : rgba(0) {}
    gle_color_int(uint32_t s) : rgba(s) {}

    void set_rgbaf(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
    {
        rgba = combine(r*255, g*255, b*255, a*255);
    }

    void set_rgba(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
    {
        rgba = combine(r, g, b, a);
    }

    void set_white() { rgba=0xFFFFFFFF; }
    void set_black() { rgba=0x000000FF; }

    bool operator ==(gle_color_int col) { return rgba == col.rgba; }

    GLubyte red()    const { return (GLubyte)( 0xFF & ( rgba >> 24 ) ); }
    GLubyte green()  const { return (GLubyte)( 0xFF & ( rgba >> 16 ) ); }
    GLubyte blue()   const { return (GLubyte)( 0xFF & ( rgba >> 8  ) ); }
    GLubyte alpha()  const { return (GLubyte)( 0xFF & ( rgba       ) ); }

    GLfloat redf()   const { return (GLfloat)(0xFF & ( rgba >> 24 )) / 255.f; }
    GLfloat greenf() const { return (GLfloat)(0xFF & ( rgba >> 16 )) / 255.f; }
    GLfloat bluef()  const { return (GLfloat)(0xFF & ( rgba >> 8  )) / 255.f; }
    GLfloat alphaf() const { return (GLfloat)(0xFF & ( rgba       )) / 255.f; }

    GLfloat brightness()  const { return (redf()+greenf()+bluef())*alphaf(); }
    bool    opaque()      const { return ((rgba & 0xFF) == 0xFF); }
    bool    transparent() const { return ((rgba & 0xFF) != 0xFF); }
    bool    visible()     const { return (rgba & 0xFF); }

    gle_color_int fade(GLubyte s) const
    {
         return gle_color_int(combine(red()>>s, green()>>s, blue()>>s, alpha()));
    }

    gle_color_int fade_alpha(GLubyte s) const
    {
        return (rgba & 0xFFFFFF00) | ((rgba & 0xFF) >> s);
    }

    gle_color_int set_alpha(GLubyte s) const
    {
        return (rgba & 0xFFFFFF00) | s;
    }

    gle_color_int match_alpha(gle_color_int col) const
    {
        return (rgba & 0xFFFFFF00) | (col.rgba & 0xFF);
    }

    gle_color_int mix(gle_color_int col) const
    {
        return (rgba>>1 & 0x7F7F7F7F) | (col.rgba>>1 & 0x7F7F7F7F);
    }


    /// set current OpenGL color by calling glColor
    void color() const
    {
        GLubyte ub[] = { red(), green(), blue(), alpha() };
        glColor4ubv(ub);
    }


    /// set current color, with alpha = 7F
    void colorT() const
    {
        GLubyte ub[] = { red(), green(), blue(), static_cast<GLubyte>(alpha()>>1) };
        glColor4ubv(ub);
    }

    /// set current color, but divide alpha component by \c 2^s
    void colorT(short s) const
    {
        GLubyte ub[] = { red(), green(), blue(), static_cast<GLubyte>(alpha()>>1) };
        glColor4ubv(ub);
    }

    /// set current color, but with \c s as alpha component
    void colorA(GLfloat s) const
    {
        if ( s < 1.0 )
            glColor4f(red(), green(), blue(), s);
        else
        {
            GLubyte ub[] = { red(), green(), blue(), alpha() };
            glColor4ubv(ub);
        }
    }

    void clearColor() const
    {
        glClearColor(redf(), greenf(), bluef(), alphaf());
    }

    /// set FRONT material property for lighting
    void front() const
    {
        GLfloat col[4] = { redf(), greenf(), bluef(), alphaf() };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);
    }

    /// set BACK material property for lighting
    void back() const
    {
        GLfloat col[4] = { redf(), greenf(), bluef(), alphaf() };
        glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    }

    /// set BACK material property for lighting
    void back(short s) const
    {
        GLfloat x = 1.0 / ( 1 << s );
        GLfloat col[4] = { redf()*x, greenf()*x, bluef()*x, alphaf() };
        glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    }

    /// conversion to string
    std::string str() const;

};


#endif
