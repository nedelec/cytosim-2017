// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef VIEW_PROP_H
#define VIEW_PROP_H

#include "real.h"
#include "vector3.h"
#include "quaternion.h"
#include "property.h"
#include "gle_color.h"

///properties needed to define a view
class ViewProp : public Property
{
public:
    
    /// number of OpenGL clipping planes
    const static unsigned int NB_CLIP_PLANES = 3;
    
    /**
     @defgroup ViewPar Display Parameters: View
     @ingroup DisplayParameters
     @{
     */
    
    
    /// zoom factor = ratio between visible area and \c view_size
    real             zoom;
    
    /// size of area visible in the window, in sim-units (default=10)
    real             view_size;

    /// enables the display area to be set from the size of the simulation space
    /**
     If ( `auto_scale` > 0 ), the view_size is set automatically to match the simulation space.
     This is on by default.
     */
    unsigned int     auto_scale;
    
    /// the point that is in the center of the window in real-world coordinates
    Vector3          focus;

    /// additional translation used by autoTrack
    Vector3          focus_shift;
    
    /// orientation of display
    Quaternion<real> rotation;
    
    
    /// enables auto_translation, auto_zoom or auto_rotation
    /**
     if ( `traveling` > 0 ), this sets the interval of time in milli-seconds
     at which the model-view transformation will be updated, by applying
     `auto_translation`, `auto_zoom` and `auto_rotation`.
     */
    unsigned int     traveling;
    
    /// translation speed of display (known as \c traveling[1])
    /**
     This is a speed in distance / wall-time
     */
    Vector3          auto_translation;
    
    /// rotational speed of display (known as \c traveling[2])
    /**
     This is a speed in quaternion / wall-time
     */
    Quaternion<real> auto_rotation;
    
    /// zooming speed of display (known as \c traveling[3])
    /**
     This is a speed in zoom-unit / wall-time
     - auto_zoom > 0 : zoom closer
     - auto_zoom < 0 : zoom away
     .
     */
    real             auto_zoom;
    
    
    /// position of window on screen (top-left corner, in pixels)
    int              window_position[2];
    
    /// desired size of window in pixels (also known as `size`)
    unsigned int     window_size[2];
    
    /// size of scale-bar in sim-world units
    real             scale_bar;
    
    /// display flag for scale-bar (set as \c scale_bar[1])
    unsigned int     show_scale;
    
    /// display flag for axes
    unsigned int     show_axes;

    /// on/off flags for clipping (defined as \c clip_plane?)
    /**
     Up to 3 clipping planes can be defined: clip_plane0 to clip_plane2
     
     Syntax:
     @code
        clip_plane? = BOOL, VECTOR, REAL
     @endcode
     The Boolean enables the clipping plane.
     The plane is specified by a normal vector \a n (VECTOR) and a scalar \a (REAL).
     The visible half-space is defined by <em> n.x + a > 0 </em>
     
     Example:
     To define a slice perpendicular to the X-axis of width 2: 
     @code
     set simul:display *
     {
        clip_plane1 = 1,  1 0 0, 1
        clip_plane2 = 1, -1 0 0, 1
     }
     @endcode
     */
    unsigned int     clip_plane_mode[NB_CLIP_PLANES];

    /// direction perpendicular to clipping plane (defined as \c clip_plane?[1])
    Vector3          clip_plane_vector[NB_CLIP_PLANES];
    
    /// scalar offset defining the equation of the clipping plane (defined as \c clip_plane?[2])
    real             clip_plane_scalar[NB_CLIP_PLANES];
    
    
    /// automatically adjust view to keep fibers in window
    /**
     Possible values:
     - 0 : off
     - 1 : translate to keep center of gravity in window
     - 2 : translate + rotate to align direction vector
     - 3 : translate + rotate to align with nematic direction tensor
     .
     The translation defined by focus is applied after this adjustment.
     */
    unsigned int     track_fibers;
    
    /// characteristics of OpenGL fog (also known as \c fog[0])
    int              fog_type;
    
    /// density of fog (also known as \c fog[1])
    real             fog_density;
    
    /// color of fog (also known as \c fog[2])
    gle_color        fog_color;
 
    /// @}
    
    /// position between Camera and the origin
    real             eyeTranslation[3];
    
public:
   
    /// constructor
    ViewProp(const std::string& n) : Property(n)  { clear(); }
    
    /// destructor
    ~ViewProp()  { }
    
    /// identifies the property
    std::string kind() const { return "view"; }
     
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// return a carbon copy of object
    Property* clone() const { return new ViewProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;

};

#endif
