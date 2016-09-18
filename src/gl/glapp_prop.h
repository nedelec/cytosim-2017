// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef GLAPP_PROP_H
#define GLAPP_PROP_H

#include "real.h"
#include "gle_color.h"
#include "vector3.h"
#include "quaternion.h"
#include "property.h"

/// Parameter set for glApp
class glAppProp : public Property
{    
public:
    
    /**
     @defgroup glAppPar Display parameters: Graphics
     @ingroup DisplayParameters
     @{
     */
    
    
    /// flag to use a double buffer for smoother rendering (default=1)
    /**
    http://en.wikipedia.org/wiki/Multiple_buffering#Double_buffering_in_computer_graphics
    */
    int          buffered;
    
    /// flag to enable OpenGL depth buffer (default=1)
    /**
     This is useful for 3D rendering.
     http://en.wikipedia.org/wiki/Z-buffering
     */
    int          depth_test;

    /// flag to enable OpenGL stencil buffer (default=0)
    int          stencil_buffer;
    
    /// flag to perform depth-clamp (default=false)
    /** http://www.opengl.org/registry/specs/NV/depth_clamp.txt */
    int          depth_clamp;
    
    /// enables OpenGL full scene anti-aliasing (default=0)
    /**
     This defines the number of samples used to build an image.
     Higher values result in nicer (but slower) display.
     http://en.wikipedia.org/wiki/Multisample_anti-aliasing
     try 4 or 8.
     */
    int          multisample;
    
    /// flag to switch to full-screen mode
    int          full_screen;
    
    /// flag to the current frame rate
    int          show_fps;
    
    /// flag to display information on screen
    int          show_message;
    
    /// string added at the start of message_left
    std::string  label;
    
    /// text displayed in center of window
    std::string  message;
    
    /// the label on the bottom left of window
    std::string  message_left;
    
    /// the labe on the bottom right of window
    std::string  message_right;
    
    /// @}
    
public:
    
    /// constructor
    glAppProp(const std::string& n, int i=-1) : Property(n, i)  { clear(); }
    
    /// destructor
    ~glAppProp()  { }
    
    /// identifies the property
    std::string kind() const { return "simul:display"; }
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// return a carbon copy of object
    Property* clone() const { return new glAppProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
    
};

#endif
