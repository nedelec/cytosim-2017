// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef VIEW_H
#define VIEW_H

#include "opengl.h"
#include "view_prop.h"

/// Handles the viewing angle, projection and other aspects of an OpenGL display
/**
 ViewProp does not depend on the window-system (GLUT),
 but only on the rendering engine (OpenGL)
 */
class View : public ViewProp
{
private:
    
    /// calculate mPixelSize
    void       setPixelSize();
    
    /// window number in GLUT
    int        mWindowId;
    
    /// actual width and height of window in pixels
    int        mWindowSize[2];
    
    /// size of pixel in drawing units
    real       mPixelSize;
    
    /// used to check that getGLMatrices() was called
    bool       hasMatrices;
    
    /// viewport obtained by getGLMatrices()
    GLint      mViewport[4];
    
    /// modelview obtained by getGLMatrices()
    GLdouble   mModelview[16];
    
    /// projection obtained by getGLMatrices()
    GLdouble   mProjection[16];
    
    /// half-size of the OpenGL visible region in OpenGL units
    real       visRegion[3];
    
public:
    
    /// constructor
    explicit View(const std::string& n);
    
    /// destructor
    ~View();
    
    /// return window-id
    int        window() const { return mWindowId; }
    
    /// set window-id
    void       window(int w) { mWindowId = w; }
    
    /// handle window resize events
    void       reshaped(int, int);

    /// set OpenGL Projection matrix
    void       setProjection();
    
    /// set OpenGL Model-View matrix
    void       setModelView() const;
    
    /// reset the view (no-rotation, zoom=1), and enable auto_scale
    void       reset();

    /// set the range that is visible in the window if zoom = 1
    void       setScale(real);

    /// size of pixel in drawing units
    real       pixelSize() const { return mPixelSize; }
    
    /// width of window
    int        width()     const { return mWindowSize[0]; }
    
    /// height of window
    int        height()    const { return mWindowSize[1]; }

    //---------------------------------------------------------------------------

    /// init OpenGL display parameters
    void       initGL(int depth_test, int multisample, int depth_clamp) const;

    /// set OpenGL Lights for lighting effects
    void       setLights(bool local = false) const;
    
    /// set OpenGL Fog, with mode (GL_EXP or GL_LINEAR), intensity and color
    void       setFog(int mode, real density, gle_color);
    void       setFog(int mode, real density);
    void       setFog() const;
    
    /// enable cliping plane in OpenGL
    void       setClipPlane(GLenum glp, Vector3 dir, real sca) const;
    void       setClipPlaneEye(GLenum glp, Vector3 dir, real sca) const;
    
    /// call setClipPlane(int) for all enabled clipping planes
    void       setClipPlanes() const;
    
    /// disable cliping planes in OpenGL
    void       endClipPlanes() const;
    
    /// set equations for a clipping plane, and enable it in View
    void       enableClipPlane(unsigned int, Vector3 dir, real scal, bool absolute=true);
    
    /// disable cliping plane in View
    void       disableClipPlane(unsigned int);
    
    /// return enable/disable state
    bool       hasClipPlane(unsigned int) const;
    
    //---------------------------------------------------------------------------

    /// store the matrices defining the current OpenGL ModelView and Projection transformations
    void       getGLMatrices();
    
    /// transform window coordinates to 3D world-coordinates
    Vector3    unproject(GLdouble x, GLdouble y, GLdouble z, bool get_matrices = false);
    
    //---------------------------------------------------------------------------
    
    /// position 'pos' in the center of the display
    void       move_to(const Vector3 & pos);
    
    /// set additional translation of focal point
    void       shift_to(const Vector3 & pos);
    
    /// translate view
    void       move_by(const Vector3 & trans)       { move_to( focus + trans ); }

    //---------------------------------------------------------------------------
    
    /// set rotation to given Quaternion
    void       rotate_to(const Quaternion<real> &);
    
    /// rotate to have 'dir' aligned with the X-axis
    void       rotate_to(const Vector3& dir);

    /// rotate view
    void       rotate_by(const Quaternion<real> &q) { rotate_to( rotation * q ); }
    
    //---------------------------------------------------------------------------

    /// set absolute zoom
    void       zoom_to(real z);
    /// increase zoom (multiplicative)
    
    void       zoom_in(real z) { zoom_to( zoom * z ); }
    
    /// decrease zoom (multiplicative)
    void       zoom_out(real z) { zoom_to( zoom / z ); }
    
    //---------------------------------------------------------------------------
    
    /// apply the transformation specified by ViewProp::auto_translation and auto_rotation
    void       travelingMotion(real dt);
    
    /// adjust zoom and focus to match the ROI specificed by two corner points
    void       matchROI(Vector3, Vector3);
    
    //---------------------------------------------------------------------------
    
    /// display a portion of a scale bar (called by displayScaleBar)
    void       displaySubScaleBar(bool, GLfloat) const;
    
    /// display a scale bar vertical or horizontal
    void       displayScaleBar(bool, GLfloat) const;
    
    
};

#endif
