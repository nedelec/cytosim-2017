// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FIBER_DISP_H
#define FIBER_DISP_H

#include "real.h"
#include "assert_macro.h"
#include "gle_color.h"
#include "property.h"
#include "vector.h"

class Glossary;

/// Display parameters for a class of Fiber
/**
 Holds the display attributes for a certain class of Fiber.
 
 There is one FiberDisp for each FiberProp.
 */
class FiberDisp : public Property
{
public:

    /// possible values for fiber:coloring
    enum ColoringModes {
        COLORING_NONE,
        COLORING_NUMBER,
        COLORING_DIRECTION,
        COLORING_MARK,
        COLORING_FLECK };
    
public:
    
    /**
     @defgroup FiberDispPar Display parameters: Fibers
     @ingroup DisplayParameters
     @{
     */
    
    
    /// visibility flag : 0=hidden, 1=opaque (-1 is used in 3D for transparent objects)
    int          visible;
    
    /// color of fiber
    gle_color    color;
    
    /// if true, use different colors to display some property of the fibers
    /**
     Values for \a coloring:
     - 0 : no coloring,
     - 1 : color fibers according to ID-number,
     - 2 : color fibers depending on direction,
     - 3 : color fibers depending on the mark,
     - 4 : color fibers by connectivity.
     .
     */
    int          coloring;
    
    
    /// width of lines (also known as \c line[0])
    real         line_width;

    /// style for lines (also known as \c line[1])
    /**
     Possible line_style:
     - 0 : hide,
     - 1 : plain lines,
     - 2 : rainbow coloring based on compressive forces,
     - 3 : color set by the angle of the fiber in XY plane,
     - 4 : local color gradient that indicates polarity.
     .
     */
    int          line_style;
    
    
    /// size of points (also known as \c point[0])
    /**
     \a point_size and \a line_width are normally set in pixels, 
     but if \a display:point_value is set, their value is understood 
     in multiples of \a point_value, which itself is a distance.
     
     For example, if you set line_width=2.5 and point_value=0.01,
     the fibers will be displayed with a diameter of 0.025.
     */
    real         point_size;
    
    /// style for display of points (also known as \c point[1])
    /**
     Possible point_style:
     - 0 : hide,
     - 1 : show model points,
     - 2 : show arrow-heads along fiber, separated by \c interval,
     - 3 : indicate abscissa.
     .
     */
    int          point_style;
    
    
    /// style of fiber tips for { PLUS_END, MINUS_END }
    /**
     end_style[0] determines the style of the PLUS_END,
     and end_style[1] the style of the MINUS_END.
     
     Possible end_style:
     - 0 : hide,
     - 1 : display a disc/sphere,
     - 2 : display a cone,
     - 3 : display a disc,
     - 4 : draw arrowhead,
     - 5 : draw arrowhead in the inverted direction (for actin)
     .
     */
    int          end_style[2];
    
    /// size of fiber tips for { PLUS_END, MINUS_END }
    /**
     You can also specify:
     @code
     plus_end  = SIZE, STYLE
     minus_end = SIZE, STYLE
     @endcode
     */
    real         end_size[2];
    
    /// length of a section displayed near the fiber tips
    /**
     if \a end_section[0] > 0, a section near the PLUS_ENDD
     is displayed with the color of the PLUS_END.
     if \a end_section[1] > 0, a section near the MINUS_END
     is displayed with the color of the MINUS_END.
     */
    real         end_section[2];
    
    /// colors of the different FiberTip states
    /**
     This determines the set of color that are used to display the fiber tips,
     according to their assembly state (Fiber::dynamicState):
     - static ends (dynamic-state 0) use end_color[0],
     - growing end (dynamic-state 1), use end_color[1],
     - shrinking end (dynamic-state 4), use end_color[4]
     .
     */
    gle_color    end_color[5];

    
    /// style for speckle display (also know as \a speckles)
    /**
     Possible speckle_style:
     - 0 : hide,
     - 1 : random speckles, separated on average by \c interval,
     - 2 : regular speckes, separated by \c interval.
     .
     */
    int          speckle_style;
    
    /// distance between speckles (also known as \c speckles[1])
    real         interval;
    
    
    /// a bit-field to hide certain categories of fibers
    /**
     Possible values for \a exclude:
     - 0 : all fibers are displayed,
     - 1 : show only right-pointing fibers,
     - 2 : show only left-pointing fibers,
     - 4 : show only counter-clockwise fibers,
     - 8 : show only clockwise fibers.
     .
     
     You may also address each bit directly, knowning that:
     - bit 1 on: hide left-pointing fibers
     - bit 2 on: hide right-pointing fibers
     - bit 3 on: hide clockwise fibers
     - bit 4 on: hide counter-clockwise fibers
     .
     */
    int          exclude;
    
    /// the reference direction used for hiding left- or right-pointing fibers, etc. (also known as \c exclude[1])
    Vector       right;
    
    
    /// period used to attribute colors with the coloring option
    unsigned int mask;
    
    /// phase used to attribute colors with the coloring option (also known as \c mask[1])
    int          phase;
    
    
    /// conversion coefficient from tension to color, for line_style==2
    /** 
     Possible values for \a rainbow:
     - 0 : disabled,
     - rainbow > 0 : compressive forces are highlighted in red,
     - rainbow < 0 : traction/elongation forces are highlighted.
     .
     */
    real         rainbow;
    
    /// ( if > 0 ) display the net forces FP acting on model points
    /**
     The force is displayed as segments of length forces*PF.
     A color can be specified as forces[1]
     */
    real         forces;

    
    /// the 'explosion' effect shift the fibers in space
    /**
     This can be useful to visualize dense regions,
     but is only implemented for style=2
     */
    int          explode;
    
    /// amount of lateral shift to separate fibers when display is exploded (known as \c explode[1])
    real         explode_range;
    
    
    /// if true, display the average fiber
    /**
     The 'average fiber' is calculated from the centroid of the fiber tips,
     and the centroid of the polymer mass.
     It is useful to evaluate the amount of order in the network.
     */
    int          show_average;

    /// @}
    
    /// this color is specified as forces[1]
    gle_color    forces_color;
    
public:
    
    /// constructor
    FiberDisp(const std::string& n) : Property(n)  { clear(); }
    
    /// destructor
    ~FiberDisp() { }
    
    /// identifies the property
    std::string kind() const { return "fiber:display"; }

    /// clear to default values
    void clear();
    
    /// set from glossary
    void read(Glossary&);
    
    /// return a carbon copy of object
    Property* clone() const { return new FiberDisp(*this); }

    /// write all values
    void write_data(std::ostream&) const;
    
};


#endif

