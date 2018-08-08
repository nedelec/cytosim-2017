// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef DISPLAY_PROP_H
#define DISPLAY_PROP_H

#include "property.h"
#include "gle_color.h"
#include "inventoried.h"
class View;


/// Property for Play
class DisplayProp : public Property
{

public:
    
    /**
     @defgroup DisplayPar Display parameters: World
     @ingroup DisplayParameters
     @{
     */
    
    /// if true, repeat the display for periodic boundary conditions
    int            tiled;
    
    /// if true, translate objects to place them in the root cell for periodic boundary conditions
    int            fold;
    
    /// color of background
    gle_color      back_color;
    
    /// color used for highlighting objects
    gle_color      front_color;
    
    /// color used to paint inside of objects
    gle_color      inner_color;
    
    /// default size of points (= diameter)
    real           point_size;
    
    /// default width of lines (= diameter)
    real           line_width;
    
    /// if set > 0, this defines the unit size used for \a point_size and \a line_width
    /**
     Set this parameter to specify the fiber radius and point size in real units.

     \a point_size and \a line_width are normally set in pixels, but if \a point_value is set,
     then the specifications are understood in multiples of \a point_value,
     which itself is given as real distance.
     
     For example, if you set line_width=2.5 and point_value=0.01,
     the fibers will be displayed with a diameter of 0.025.
     
     <em> default = 0 </em>
     */
    real           point_value;
    
    /// reference number of the field that is display
    Number         field_number;
    
    /// maximum value for the displayed field
    real           field_max;
    
    /// selection bitfield for Couples
    unsigned int   couple_select;
    
    /// selection bitfield for Singles
    unsigned int   single_select;
    
    /// @}
    
public:
    /// constructor
    DisplayProp(const std::string& n, int i=-1) : Property(n, i) { clear(); }
    
    /// destructor
    ~DisplayProp() { }
    
    /// identifies the property
    std::string kind() const { return "simul:display"; }
        
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// return a carbon copy of object
    Property* clone() const { return new DisplayProp(*this); }
    
    /// write all values
    void write_data(std::ostream &) const;

};


#endif


