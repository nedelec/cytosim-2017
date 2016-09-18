// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/** 
 off-screen rendering using OpenGL Frame Buffer Objects
 
 http://en.wikipedia.org/wiki/Framebuffer_Object
 
 https://developer.apple.com/library/mac/#documentation/graphicsimaging/Conceptual/OpenGL-MacProgGuide/opengl_offscreen/opengl_offscreen.html
*/

#include <cstdio>

#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/glu.h>


CGLContextObj context = 0;

GLuint frameBuffer = 0;
GLuint colorBuffer = 0;
GLuint depthBuffer = 0;

//------------------------------------------------------------------------------
char const* glErrorString(GLenum code)
{
    switch ( code )
    {
        case GL_NO_ERROR:          return "GL_NO_ERROR";
        case GL_INVALID_ENUM:      return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:     return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_STACK_OVERFLOW:    return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:   return "GL_STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY:     return "GL_OUT_OF_MEMORY";
        case GL_TABLE_TOO_LARGE:   return "GL_TABLE_TOO_LARGE";
        default:                   return "GL_UNKNOWN_ERROR";
    }
}


void checkError(const char msg[])
{
    GLenum glError = glGetError();
    while ( glError != GL_NO_ERROR )
    {
        fprintf(stderr, "OpenGL error `%s' %s\n", glErrorString(glError), msg);
        glError = glGetError();
    }
}



bool hasGLExtension(const char* ext)
{
    const GLubyte * str = glGetString(GL_EXTENSIONS);
    return gluCheckExtension((const GLubyte *)ext, str);
}



void describePixelFormat(CGLPixelFormatObj const& p, GLint n)
{
    GLint cap = 0;
    printf("pixel format %i:\n", n);
    CGLDescribePixelFormat(p, n, kCGLPFAColorSize, &cap);
    printf("    colors      %i\n", cap);
    CGLDescribePixelFormat(p, n, kCGLPFADepthSize, &cap);
    printf("    depth       %i\n", cap);
    CGLDescribePixelFormat(p, n, kCGLPFAMultisample, &cap);
    printf("    multi       %i\n", cap);
    if (cap)
    {
        CGLDescribePixelFormat(p, n, kCGLPFASampleBuffers, &cap);
        printf("       buffers  %i\n", cap);
        CGLDescribePixelFormat(p, n, kCGLPFASamples, &cap);
        printf("       samples  %i\n", cap);
    }
}

//------------------------------------------------------------------------------

int createContext(bool& multisample)
{
    CGLPixelFormatAttribute attribs[] =
    {
        kCGLPFAMinimumPolicy,
        //kCGLPFAPBuffer,
        //kCGLPFARendererID,
        kCGLPFAColorSize,
        (CGLPixelFormatAttribute)32,
        kCGLPFADepthSize,
        (CGLPixelFormatAttribute)16,
        kCGLPFAMultisample,
        kCGLPFASamples,
        (CGLPixelFormatAttribute)4,
        (CGLPixelFormatAttribute)0
    };

    GLint npix = 0;
    CGLError err;
    CGLPixelFormatObj pix;
    
    err = CGLChoosePixelFormat(attribs, &pix, &npix);
    
    if ( npix == 0 )
    {
        fprintf(stderr, "Could not find suitable pixel format\n");
        return 1;
    }
    
    // find a format with multisample support:
    multisample = false;
    for ( int n = 0; n < npix; ++n )
    {
        //describePixelFormat(pix, n);
        GLint cap = 0;
        CGLDescribePixelFormat(pix, n, kCGLPFAMultisample, &cap);
        if ( cap ) multisample = true;
    }
    
    CGLCreateContext(pix, NULL, &context);
    CGLReleasePixelFormat(pix);
    
    if ( context == 0 )
    {
        fprintf(stderr, "Could not create OpenGL context\n");
        return 2;
    }
    
    if ( CGLSetCurrentContext(context) )
    {
        fprintf(stderr, "Could not switch OpenGL context\n");
        return 3;
    }
    
    checkError("createContext()");
    return 0;
}

//------------------------------------------------------------------------------

/**
 Set up a Frame Buffer object with two Render buffers attached,
 for color and depth data.
 */
int OffScreen::open(const unsigned int width, const unsigned int height)
{
    bool multisample = false;
    
    if ( createContext(multisample) )
    {
        return 1;
    }
    
    if ( ! hasGLExtension("GL_EXT_framebuffer_object") )
    {
        fprintf(stderr, "Error: OpenGL lacks GL_EXT_framebuffer_object\n");
        return 2;
    }
    
    //Set up a FBO with two renderBuffer attachment
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    glGenRenderbuffers(1, &colorBuffer);
    glGenRenderbuffers(1, &depthBuffer);

    if ( multisample )
    {
        if ( ! hasGLExtension("GL_EXT_framebuffer_multisample") )
        {
            fprintf(stderr, "Error: OpenGL lacks GL_EXT_framebuffer_object\n");
            return 3;
        }

        glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_RGBA8, width, height);
        
        glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_DEPTH_COMPONENT, width, height);
        checkError("Offscreen::open::glRenderbufferStorageMultisample()");
    }
    else
    {
        glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
        
        glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        checkError("Offscreen::open::glRenderbufferStorage()");
    }
    
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    checkError("Offscreen::open::glFramebufferRenderbuffer()");

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    
    if ( status == GL_FRAMEBUFFER_UNSUPPORTED )
        return 7;
    if ( status != GL_FRAMEBUFFER_COMPLETE )
        return 8;

    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    checkError("Offscreen::open::glBindFramebuffer()");

    return 0;
}


void OffScreen::close()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &frameBuffer);
    glDeleteRenderbuffers(1, &colorBuffer);
    glDeleteRenderbuffers(1, &depthBuffer);
    CGLSetCurrentContext(0);
    CGLDestroyContext(context);
}

