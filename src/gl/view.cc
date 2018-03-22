// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "assert_macro.h"
#include "view.h"
#include "gle.h"
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#include <cstdarg>
using namespace gle;

extern void flashText(const char fmt[], ...);

//------------------------------------------------------------------------------

View::View(const std::string& n) : ViewProp(n)
{    
    mWindowId = 0;
    
    visRegion[0] = view_size;
    visRegion[1] = view_size;
    visRegion[2] = view_size;

    mWindowSize[0] = window_size[0];
    mWindowSize[1] = window_size[1];
    
    hasMatrices = false;
    setPixelSize();
}

View::~View()
{
}


//------------------------------------------------------------------------------
#pragma mark -

void View::initGL(int depth_test, int multisample, int depth_clamp) const
{
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_DITHER);
    
    if ( 1 )
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
        glDisable(GL_BLEND);
    
    if ( multisample )
    {
        glEnable(GL_MULTISAMPLE);
    }
    else
    {
        //glDisable(GL_MULTISAMPLE);
        glEnable(GL_POINT_SMOOTH);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        /*
         Do not enable POLYGON_SMOOTH, which destroys joints of triangulated surfaces
         glEnable(GL_POLYGON_SMOOTH);
         glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
         */      
    }
    
    if ( depth_clamp )
        glEnable(GL_DEPTH_CLAMP);
    else
        glDisable(GL_DEPTH_CLAMP);

    
    if ( depth_test )
    {
        //std::cerr << "no depth-test" << std::endl;
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        //glDepthFunc(GL_LEQUAL);
        // enable Alpha Test to discard transparent pixels:
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, 0);
    }
    else
        glDisable(GL_DEPTH_TEST);
}

/**
 Set two light sources and enable GL_COLOR_MATERIAL
 */
void View::setLights(bool local) const
{
    glMatrixMode(GL_MODELVIEW);
    if ( local )
    {
        glPushMatrix();
        glLoadIdentity();
    }
    
    glShadeModel(GL_SMOOTH);
    
    GLfloat matWhite[]      = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat matGray[]       = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat matBlack[]      = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat shininess[]     = { 50.0 };
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matBlack);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matBlack);
    
    glMaterialfv(GL_FRONT, GL_SPECULAR,  matWhite);
    glMaterialfv(GL_BACK,  GL_SPECULAR,  matBlack);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
    
    GLfloat lightDiffuse[]  = { 0.8, 0.8, 0.8, 1.0 };
    GLfloat lightSpecular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat lModelAmbient[] = { 0.2, 0.2, 0.2, 1.0 };
    
    GLfloat light0Pos[] = { 5.0,-3.0, 3.0, 0.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glEnable(GL_LIGHT0);
    
    GLfloat light1Pos[] = {-4.0, 0.0,-3.0, 0.0 };
    glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  lightDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
    glEnable(GL_LIGHT1);
    
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lModelAmbient);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    
    // let GL normalize the normals:
    glEnable(GL_NORMALIZE);
    
    // set a gray color for the back-side of everything
    glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, matGray);

    // enable the automatic material color specification by any call to glColor
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    if ( local )
        glPopMatrix();
}



//------------------------------------------------------------------------------
#pragma mark -


void View::setProjection()
{
    //std::cerr << "setProjection win " << window() << std::endl;
    int const* wSize = mWindowSize;
    
    //calculate the visible region in the 3 directions:
    if ( wSize[0] >  wSize[1] )
    {
        visRegion[0] = view_size;
        visRegion[1] = view_size * wSize[1] / real(wSize[0]);
    }
    else
    {
        visRegion[0] = view_size * wSize[0] / real(wSize[1]);
        visRegion[1] = view_size;
    }
    visRegion[2] = view_size;
    
    glViewport(0, 0, wSize[0], wSize[1]);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // The back-plane is set far back to avoid any clipping there
    glOrtho(-visRegion[0], visRegion[0],
            -visRegion[1], visRegion[1],
            0, 2*visRegion[2]);
    
    glMatrixMode(GL_MODELVIEW);
    setPixelSize();
}


void View::setModelView() const
{
    //std::cerr<<"setModelView win " << window() << std::endl;

    // setup the OpenGL transformation matrix
    glMatrixMode(GL_MODELVIEW);
#if ( 1 )
    real mat[16];
    rotation.setOpenGLMatrix(mat, eyeTranslation);
    gleLoadMatrix(mat);
#else
    glLoadIdentity();
    real rot[3];
    real ang = 180 / M_PI * rotation.getAngle(rot);
    gleTranslate(eyeTranslation[0], eyeTranslation[1], eyeTranslation[2]);
    gleRotate(ang, rot[0], rot[1], rot[2]);
#endif
    gleScale(zoom);
    
    // point-of-focus:
    gleTranslate(-(focus+focus_shift));
}


void View::setPixelSize()
{
    mPixelSize = 2 * visRegion[0] / ( zoom * mWindowSize[0] );
}


void View::reshaped(int w, int h)
{
    //std::cerr << "View::reshaped win " << window() << " " << w << " " << h <<std::endl;
    mWindowSize[0] = w;
    mWindowSize[1] = h;
    
    window_size[0] = w;
    window_size[1] = h;
    
    setProjection();
}

//------------------------------------------------------------------------------
#pragma mark -

void View::reset()
{
    zoom = 1;
    auto_scale = 1;
    focus.set(0,0,0);
    focus_shift.set(0,0,0);
    rotation.set(1,0,0,0);
    setModelView();
    setPixelSize();
    //flashText("Reset view");
}


void View::zoom_to(real z)
{
    //std::cerr << "zoom_to " << z << " " << this << std::endl;
    zoom = z;
    setModelView();
    setPixelSize();
}


void View::setScale(real s)
{
    //std::cerr << "View::setScale("<<s<<") win=" << window() << std::endl;
    view_size = s;
    eyeTranslation[2] = -s;
    setProjection();
    setModelView();
}


void View::matchROI(Vector3 a, Vector3 b)
{
    focus = 0.5 * ( a + b );
    real r = 0.5 * ( a - b ).norm_inf();
    
    // require 3 pixels to zoom in:
    if ( r > 3 * mPixelSize )
        zoom = view_size / r;
    
    setModelView();
    setPixelSize();
}

//------------------------------------------------------------------------------
#pragma mark -

void View::move_to(const Vector3& d)
{
    focus = d;
    setModelView();
    //flashText("Focus %.2f %.2f %.2f", focus[0], focus[1], focus[2]);
}


void View::shift_to(const Vector3& d)
{
    focus_shift = d;
    setModelView();
}


//------------------------------------------------------------------------------
#pragma mark -


void View::rotate_to(const Quaternion<real> & q)
{
    rotation = q.normalized();
    setModelView();
    //flashText("Quat %.2f %.2f %.2f %.2f", rotation[0], rotation[1], rotation[2], rotation[3]);
}


void View::rotate_to(const Vector3 & dir)
{
    Vector3 axis( 0, dir.ZZ, -dir.YY );
    real cos = dir.XX, sin = axis.norm();
    if ( sin == 0 ) axis.set(0, 0, 1);
    rotation.setFromAxis(axis, atan2(sin, cos));
    setModelView();
}


void View::travelingMotion(real dt)
{
    focus += dt * auto_translation;
    zoom = zoom * ( 1 + dt * auto_zoom );
    Quaternion<real> Q = auto_rotation.scaledAngle(dt) * rotation;
    rotation = Q.normalized();
    setModelView();
}


//------------------------------------------------------------------------------
#pragma mark -

void View::getGLMatrices()
{
    //get the transformation matrices, to be used for mouse control
    glGetIntegerv(GL_VIEWPORT,         mViewport);
    glGetDoublev(GL_PROJECTION_MATRIX, mProjection);
    glGetDoublev(GL_MODELVIEW_MATRIX,  mModelview);
    hasMatrices = true;
}

//------------------------------------------------------------------------------
/**
 Transforms the given window coordinates into user coordinates.
 
 It uses the matrices obtained at the last call of getGLMatrices(),
 or the current matrices if get_matrices == true.
 
 For more info, enter: man gluUnProject()
 */
Vector3 View::unproject(GLdouble x, GLdouble y, GLdouble z, bool get_matrices)
{
    GLdouble ux, uy, uz;
    if ( get_matrices )
    {
        GLint      vp[4];
        GLdouble   mv[16];
        GLdouble   pj[16];
        
        glGetIntegerv(GL_VIEWPORT,         vp);
        glGetDoublev(GL_PROJECTION_MATRIX, pj);
        glGetDoublev(GL_MODELVIEW_MATRIX,  mv);
        gluUnProject(x, y, z, mv, pj, vp, &ux, &uy, &uz);
    }
    else
    {
        if ( hasMatrices )
            gluUnProject(x, y, z, mModelview, mProjection, mViewport, &ux, &uy, &uz);
        else
        {
            ux = 0; uy = 0; uz = 0;
        }
    }
    //printf("unproject( %.2f, %.2f, %.2f ) = ( %.2f, %.2f, %.2f )\n", x, y, z, ux, uy, uz);
    return Vector3(ux, uy, uz);
}


//------------------------------------------------------------------------------
#pragma mark -

void View::setFog() const
{
    if ( fog_type!=GL_LINEAR  &&  fog_type!=GL_EXP  &&  fog_type!=GL_EXP2 )
    {
        glDisable(GL_FOG);
        return;
    }
    
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, fog_type);
    
    if ( fog_type == GL_LINEAR )
    {
        glFogf(GL_FOG_START, 0);
        glFogf(GL_FOG_END,   2*visRegion[2]);
    }
    else
    {
        glFogf(GL_FOG_DENSITY, fog_density / visRegion[2]);
    }
    
    GLfloat rgba[4] = { fog_color.redf(), fog_color.greenf(), fog_color.bluef(), fog_color.alphaf() };
    glFogfv(GL_FOG_COLOR,  rgba);
}

void View::setFog(const int type, const real density, gle_color color)
{
    fog_type = type;
    fog_density = density;
    fog_color = color;
    setFog();
}

void View::setFog(const int type, const real density)
{
    fog_type = type;
    fog_density = density;
    setFog();
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 The plane equations is relative to the model
 */
void View::setClipPlane(GLenum glp, Vector3 dir, real sca) const
{
    GLdouble eq[] = {dir.XX, dir.YY, dir.ZZ, sca};
    glClipPlane(glp, eq);
    glEnable(glp);
}

/**
 The plane equation is relative to the camera
 */
void View::setClipPlaneEye(GLenum glp, Vector3 dir, real sca) const
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    setClipPlane(glp, dir, sca);
    glPopMatrix();
}


void View::setClipPlanes() const
{
    for ( unsigned int ix = 0; ix < NB_CLIP_PLANES; ++ix )
    {
        if ( clip_plane_mode[ix] == 1 )
            setClipPlane(GL_CLIP_PLANE0+ix, clip_plane_vector[ix], clip_plane_scalar[ix]);
        else if ( clip_plane_mode[ix] == 2 )
            setClipPlaneEye(GL_CLIP_PLANE0+ix, clip_plane_vector[ix], clip_plane_scalar[ix]);
    }
}

void View::endClipPlanes() const
{
    for ( unsigned int ix = 0; ix < NB_CLIP_PLANES; ++ix )
        glDisable(GL_CLIP_PLANE0+ix);
}

void View::enableClipPlane(unsigned int ix, Vector3 dir, real sc, bool model)
{
    if ( ix < NB_CLIP_PLANES )
    {
        clip_plane_mode[ix]   = ( model ? 1 : 2 );
        clip_plane_vector[ix] = dir;
        clip_plane_scalar[ix] = sc;
    }
}

void View::disableClipPlane(unsigned int ix)
{
    if ( ix < NB_CLIP_PLANES )
    {
        clip_plane_mode[ix] = 0;
        glDisable(GL_CLIP_PLANE0+ix);
    }
}

bool View::hasClipPlane(unsigned int ix) const
{
    if ( ix < NB_CLIP_PLANES )
        return clip_plane_mode[ix];
    return false;
}

//------------------------------------------------------------------------------
#pragma mark -

void View::displaySubScaleBar(const bool vertical, const GLfloat scale) const
{
    glBegin(GL_LINES);
    
    GLfloat w = scale;
    if ( vertical )
    {
        for ( int ii = -5; ii <= 5; ++ii )
        {
            glVertex2f( -w, ii*scale );
            glVertex2f(  0, ii*scale );
        }
    }
    else
    {
        for ( int ii = -5; ii <= 5; ++ii )
        {
            glVertex2f( ii*scale, 0 );
            glVertex2f( ii*scale, w );
        }
    }
    glEnd();
}

//------------------------------------------------------------------------------
void View::displayScaleBar(const bool vertical, const GLfloat sc) const
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    GLfloat shift = 32 * pixelSize() * zoom;
    
    if ( vertical )
        gleTranslate(visRegion[0]-shift, 0, 0);
    else
        gleTranslate(0, shift-visRegion[1], 0);
    
    gleScale(zoom);
    
    //draw a box of length 'sc'
    glLineWidth(3);
    glBegin(GL_LINE_LOOP);
    if ( vertical )
    {
        glVertex2f( -sc/10, -sc/2 );
        glVertex2f(    0.0, -sc/2 );
        glVertex2f(    0.0,  sc/2 );
        glVertex2f( -sc/10,  sc/2 );
    }
    else
    {
        glVertex2f( -sc/2,    0.0 );
        glVertex2f( -sc/2,  sc/10 );
        glVertex2f(  sc/2,  sc/10 );
        glVertex2f(  sc/2,    0.0 );
    }
    glEnd();
    
    //draw lines every 10th of scale
    glLineWidth(3.0);
    displaySubScaleBar(vertical, 0.1 * sc);
    
    //draw small lines every 100th of scale
    if ( pixelSize() < 0.05 )
    {
        glLineWidth(2);
        displaySubScaleBar(vertical, 0.01 * sc);
    }
    
    //draw very tiny lines every 1000th of scale
    if ( pixelSize() < 0.005 )
    {
        glLineWidth(1.0);
        displaySubScaleBar(vertical, 0.001 * sc);
    }
    
    //draw very tiny lines every 1000th of scale
    if ( pixelSize() < 0.0005 )
    {
        glLineWidth(0.5);
        displaySubScaleBar(vertical, 0.0001 * sc);
    }
    
    glPopMatrix();
}



