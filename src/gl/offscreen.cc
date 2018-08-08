// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "offscreen.h"
#include "opengl.h"


#if defined(__APPLE__)

// OpenGL Frame Buffer Objects
#include "offscreen_fbo.cc"

#elif defined(__linux)

// X-windows offscreen rendering routines (Linux)
#include "offscreen_glx.cc"

#else

//set dummy routines otherwise
#include <cstdio>

int OffScreen::open(const unsigned int, const unsigned int)
{
    fprintf(stderr,"This program cannot render off-screen\n");
    return 1;
}

void OffScreen::close()
{
}

#endif
