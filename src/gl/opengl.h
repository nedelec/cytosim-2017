// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

// include the OpenGL header depending on platform

#ifdef __APPLE__
    #include <OpenGL/gl.h>
#else
    #include <GL/glew.h>
#endif


/// These values should be defined in OpenGL/glext.h

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

#ifndef GL_DEPTH_CLAMP
#define GL_DEPTH_CLAMP 0x864F
#endif
