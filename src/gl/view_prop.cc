// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "view_prop.h"
#include "glossary.h"

//------------------------------------------------------------------------------
void ViewProp::clear()
{
    zoom               = 1;
    view_size          = 10;
    auto_scale         = 1;
    focus.set(0,0,0);
    focus_shift.set(0,0,0);
    rotation.set(1,0,0,0);
    
    traveling = 0;
    auto_zoom = 0;
    auto_rotation.set(1,0,0,0);
    auto_translation.set(0,0,0);
    
    window_size[0]     = 768;
    window_size[1]     = 768;
    window_position[0] = 0;
    window_position[1] = 50;
    
    scale_bar          = 10;
    show_scale         = 0;
    show_axes          = 0;
    
    for ( unsigned k = 0; k < NB_CLIP_PLANES; ++k )
    {
        clip_plane_mode[k] = 0;
        clip_plane_vector[k].set(1,0,0);
        clip_plane_scalar[k] = 0;
    }
    
    track_fibers       = 0;
    
    fog_type           = 0;
    fog_density        = 1;
    fog_color          = 0x000000FF;
    
    eyeTranslation[0]  = 0;
    eyeTranslation[1]  = 0;
    eyeTranslation[2]  = -view_size;
}


void ViewProp::read(Glossary& glos)
{
    glos.set(zoom,              "zoom");
    glos.set(view_size,         "view_size");
    glos.set(auto_scale,        "auto_scale");
    glos.set(focus,             "focus");
    glos.set(rotation,          "rotation");
    
    // normalize quaternion:
    if ( rotation.norm() > 0.001 )
        rotation.normalize();
    else
        rotation.set(1,0,0,0);

    glos.set(traveling,         "traveling");
    glos.set(auto_translation,  "traveling", 1);
    glos.set(auto_rotation,     "traveling", 2);
    glos.set(auto_zoom,         "traveling", 3);

#ifdef BACKWARD_COMPATIBILITY
    if ( glos.set(auto_translation,  "auto_translation") )
        traveling = 10;
    
    if ( glos.set(auto_zoom,         "auto_zoom") )
        traveling = 10;
    
    if ( glos.set(auto_rotation,     "auto_rotation") )
        traveling = 10;
#endif
    
    // normalize quaternion:
    if ( auto_rotation.norm() > 0.001 )
        auto_rotation.normalize();
    else
        auto_rotation.set(1,0,0,0);
    
    glos.set(window_position, 2,  "window_position");
    
    // A square window is made if only one value is given.
    if ( glos.set(window_size, 2, "window_size") == 1 )
        window_size[1] = window_size[0];
    
    // 'size' is an alias to set the size of window.
    if ( glos.set(window_size, 2, "size") == 1 )
        window_size[1] = window_size[0];
    /*
     window_size can be changed, but we cannot here resize
     the window, since we do not have access to GLUT 
     */
    //std::cerr << this << " window " << window_size[0] << "x" << window_size[1] << std::endl;
    
    glos.set(scale_bar,         "scale_bar");
    glos.set(show_scale,        "scale_bar", 1);
    glos.set(show_axes,         "show_axes");

    for ( unsigned k = 0; k < NB_CLIP_PLANES; ++k )
    {
        char var[16];
        snprintf(var, sizeof(var), "clip_plane%u", k);
        glos.set(clip_plane_mode[k],   var);
        glos.set(clip_plane_vector[k], var, 1);
        glos.set(clip_plane_scalar[k], var, 2);
    }
    
    glos.set(track_fibers,      "track_fibers");
    
    KeyList<GLint> fogValues("none",         0,
                             "linear",       GL_LINEAR,
                             "exponential",  GL_EXP,
                             "exponential2", GL_EXP2);

    glos.set(fog_type,          "fog_type", fogValues);
    glos.set(fog_density,       "fog_density");
    glos.set(fog_color,         "fog_color");
 
    glos.set(fog_type,          "fog", fogValues);
    glos.set(fog_density,       "fog", 1);
    glos.set(fog_color,         "fog", 2);
}

//------------------------------------------------------------------------------

void ViewProp::write_data(std::ostream & os) const
{
    write_param(os, "zoom",        zoom);
    write_param(os, "focus",       focus+focus_shift);
    write_param(os, "rotation",    rotation);
    write_param(os, "traveling",   traveling, auto_translation, auto_rotation, auto_zoom);
    write_param(os, "window_size", window_size, 2);
    //write_param(os, "window_position", window_position, 2);

    write_param(os, "view_size",   view_size);
    write_param(os, "scale_bar",   scale_bar);
    write_param(os, "show_scale",  show_scale);
    write_param(os, "show_axes",   show_axes);

    for ( unsigned k = 0; k < NB_CLIP_PLANES; ++k )
    {
        char var[16];
        snprintf(var, sizeof(var), "clip_plane%u", k);
        write_param(os, var, clip_plane_mode[k], clip_plane_vector[k], clip_plane_scalar[k]);
    }
    
    write_param(os, "track_fibers", track_fibers);
    write_param(os, "auto_scale",   auto_scale);
    write_param(os, "fog",          fog_type, fog_density, fog_color);
}


