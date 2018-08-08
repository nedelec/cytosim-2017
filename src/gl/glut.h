// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

// GLUT is a platform-independent windowing library built on OpenGL
// The place of the header 'glut.h' is platform-dependent, so we
// include this file "glut.h" rather than <glut.h>

#ifdef __APPLE__
    #include <OpenGL/gl.h>
    #include <GLUT/glut.h>
#else
    #include <GL/glew.h>
    #include <GL/glut.h>
#endif


