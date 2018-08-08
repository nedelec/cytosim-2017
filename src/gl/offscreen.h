// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef OFFSCREEN_H
#define OFFSCREEN_H


/// functions to open/close an OpenGL off-screen display
namespace OffScreen
{
    
    ///create off-screen display of requested (width,height) or return error code
    int open(unsigned int width, unsigned int height);
    
    ///close the off-screen display, releasing allocated memory 
    void close();
    
}


#endif

