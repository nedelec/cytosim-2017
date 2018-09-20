// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef PLAY_PROP_H
#define PLAY_PROP_H


#include "property.h"


/// Property for Play
class PlayProp : public Property
{
    
public:
    
    /// number of programmable keys
    static const int NB_MAGIC_KEYS = 4;
    
    /// a flag for live simulation
    unsigned int   live;
    
public:
    
    /**
     @defgroup PlayPar Parameters of Play
     @ingroup DisplayParameters
     @{
     */
       
    /// index of displayed frame in trajectory file
    int            frame;
    
    /// direction of replay: 1: forward and -1: reverse
    int            play;

    /// if true, jump to first frame after last frame
    unsigned int   loop;
    
    /// number of simulation steps between consecutive displays
    /**
     if period==2, only every other frame will be displayed
     */
    unsigned int   period;
    
    /// number of milli-seconds between refresh
    unsigned int   delay;
    
    /// style of display { 1, 2, 3 }
    /**
     3 styles are implemented:
     - style 1 used OpenGL lines and points. It is suitable for 2D work.
     - style 2 is a faster display, also suitable for 2D.
     - style 3 draw real tubes and uses OpenGL lighting for rendering. It is nice for 3D.
     .
     */
    unsigned int   style;
    
    /// specifies information displayed near the bottom left corner of window
    std::string    report;
    
    /// associate a piece of custom code to a key
    /**
     Example:
     @code
     % define magic key 1 to delete fibers:
     set simul:display *
     {
        magic_key1 = m, ( delete 10 fiber microtubule )
        magic_key2 = C, ( cut fiber * { plane = 1 0 0, 0 } )
        label = (Press 'm' to delete fibers!)
     }
     @endcode
     up to 4 keys (magic_key0, magic_key1 ... 3) can be defined.
     */
    char           magic_key[NB_MAGIC_KEYS];    
    
    /// format of exported images [png, ppm]
    std::string    image_format;
    
    /// directory where images are exported
    std::string    image_dir;
    
    /// @}
    
    /// list of report-strings accessible from play
    std::string    report1, report2, report3, report4, report5;

    /// index used to build the name of the exported image
    int            image_index;
    
    /// index used to build the name of the exported poster
    int            poster_index;
   
    /// the piece of cytosim code executed when \c magic_key is pressed (set as magic_key[1])
    std::string    magic_code[NB_MAGIC_KEYS];
    
public:

    /// constructor
    PlayProp(const std::string& n, int i=-1) : Property(n,i)  { clear(); }
    
    /// destructor
    ~PlayProp() { }
    
    /// identifies the property
    std::string kind() const { return "simul:display"; }

    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// return a carbon copy of object
    Property* clone() const { return new PlayProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;
    
    /// change \a report to be one of \a report?
    void toggleReport(bool alt);
};


#endif


