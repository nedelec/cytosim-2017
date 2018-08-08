// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef GLE_COLOR_FLOAT_H
#define GLE_COLOR_FLOAT_H

#include "opengl.h"
#include <iostream>


/// RGBA (Red, Green, Blue, Alpha) color, with one float in [0,1] per component
class gle_color_float
{
private:

    GLfloat c[4];

public:
    
    gle_color_float() { set_white(); }
    gle_color_float(uint32_t s)
    {
        c[0]= ( 0xFF & ( s >> 24 ) )/255.f;
        c[1]= ( 0xFF & ( s >> 16 ) )/255.f;
        c[2]= ( 0xFF & ( s >>  8 ) )/255.f;
        c[3]= ( 0xFF & s )/255.f;
    }
    gle_color_float(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
    {
        c[0]= r;
        c[1]= g;
        c[2]= b;
        c[3]= a;
    }
    
    void set_rgbaf(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
    {
        c[0]= r;
        c[1]= g;
        c[2]= b;
        c[3]= a;
    }
    
    void set_rgba(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
    {
        c[0]= (GLfloat)r / 255.0;
        c[1]= (GLfloat)g / 255.0;
        c[2]= (GLfloat)b / 255.0;
        c[3]= (GLfloat)a / 255.0;
    }

    void set_white() { c[0]=1; c[1]=1; c[2]=1; c[3]=1; }
    void set_black() { c[0]=0; c[1]=0; c[2]=0; c[3]=1; }
    
    bool operator ==(gle_color_float col)
    {
        return (c[0]==col.c[0] && c[1]==col.c[1] && c[2]==col.c[2] && c[3]==col.c[3]);
    }

    GLubyte red()   const { return c[0]*255; }
    GLubyte green() const { return c[1]*255; }
    GLubyte blue()  const { return c[2]*255; }
    GLubyte alpha() const { return c[3]*255; }
    
    GLfloat redf()   const { return c[0]; }
    GLfloat greenf() const { return c[1]; }
    GLfloat bluef()  const { return c[2]; }
    GLfloat alphaf() const { return c[3]; }
    
    GLfloat brightness()  const { return (redf()+greenf()+bluef())*alphaf();; }
    bool    opaque()      const { return (c[3] > 0.99); }
    bool    transparent() const { return (c[3] < 0.99); }
    bool    visible()     const { return (c[3] > 0.01); }
    
    gle_color_float fade(GLubyte s) const
    {
        return gle_color_float(c[0]/(1<<s), c[1]/(1<<s), c[2]/(1<<s), c[3]);
    }
    
    gle_color_float fade_alpha(GLubyte s) const
    {
        return gle_color_float(c[0], c[1], c[2], c[3]/(1<<s));
    }
    
    gle_color_float set_alpha(GLubyte s) const
    {
        return gle_color_float(c[0], c[1], c[2], s/256.0);
    }
    
    gle_color_float match_alpha(gle_color_float col) const
    {
        return gle_color_float(c[0], c[1], c[2], col.c[3]);
    }

    gle_color_float mix(gle_color_float x) const
    {
        return gle_color_float((c[0]+x.c[0])*0.5, (c[1]+x.c[1])*0.5, (c[2]+x.c[2])*0.5, (c[3]+x.c[3])*0.5);
    }
    
    
    /// set current OpenGL color by calling glColor
    void color() const
    {
        glColor4fv(c);
    }
    
    /// set current color, but divide alpha = 0.5
    void colorT() const
    {
        glColor4f(c[0], c[1], c[2], 0.5*c[3]);
    }
    
    /// set current color, but divide alpha component by \c 2^s
    void colorT(short s) const
    {
        glColor4f(c[0], c[1], c[2], c[3]/(1<<s));
    }
    
    /// set current color, but with \c s as alpha component
    void colorA(GLfloat s) const
    {
        glColor4f(c[0], c[1], c[2], s);
    }
    
    
    void clearColor() const
    {
        glClearColor(c[0], c[1], c[2], c[3]);
    }
    
    /// set FRONT material property for lighting
    void front() const
    {
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c);
    }
    
    /// set BACK material property for lighting
    void back() const
    {
        glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, c);
    }
    
    /// conversion to string
    std::string str() const;
};

#endif

