// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef GLE_COLOR_H
#define GLE_COLOR_H


#include <iostream>
#include "gle_color_int.h"
#include "gle_color_float.h"


/*
4-component colors (RGBA) are implemented in two different ways:
 - gle_color_int uses a 32-bit unsigned integer, and 8 bits for each components,
 - gle_color_float uses one GLfloat for each component
 .
 */
typedef gle_color_int gle_color;
//typedef gle_color_float gle_color;




/// input operator:
std::istream & operator >> ( std::istream & is, gle_color & );

/// output operator:
std::ostream & operator << ( std::ostream & os, const gle_color & );



namespace gle
{
    
    ///conversion function from RGB to HSV color space
    void RGBtoHSV(GLfloat r, GLfloat g, GLfloat b, GLfloat* h, GLfloat* s, GLfloat* v);
    
    ///conversion functions from HSV to RGB color space
    void HSVtoRGB(GLfloat h, GLfloat s, GLfloat v, GLfloat* r, GLfloat* g, GLfloat* b);
        
    ///set a RGB color from a factor in [0, 1], continuously varying through all colors
    void setHueColor(GLfloat& r, GLfloat& g, GLfloat& b, GLfloat h);

    /// return colors aturated color with given Hue in [0, 1]
    gle_color hueColor(GLfloat h, GLfloat a);
    
    ///set saturated color with given Hue in [0, 1]
    void gleHueColor(GLfloat h);
    
    ///set saturated color with given Hue in [0, 1] and transparency `a`
    void gleHueColor(GLfloat h, GLfloat a);

    
    ///set a RGB color from a factor in [0, 1], continuously varying through blue, green, red, white
    void setJetColor(GLfloat& r, GLfloat& g, GLfloat& b, GLfloat h, GLfloat min);
    
    ///set the current color from a factor in [0, 1]
    void gleJetColor(GLfloat h, GLfloat alpha, GLfloat min=0.25);
    
    /// return color
    gle_color jetColor(GLfloat h, GLfloat alpha, GLfloat min=0.25);
}

#endif
