// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef POINT_DISP_H
#define POINT_DISP_H

#include "gle_color.h"
#include "property.h"
#include "vector.h"

class Glossary;

#define POINTDISP_USES_BITMAPS


/// the parameters necessary to display a point-like object
class PointDisp : public Property
{    
    /// used to differentiate between different uses of the class
    std::string mKind;
    
    /// draw active state with OpenGL vector primitives
    void drawVectorA() const;
    
    /// draw inactive state with OpenGL vector primitives
    void drawVectorI() const;

#ifdef POINTDISP_USES_BITMAPS
    
    /// space to hold 4 square bitmaps of size bmD*bmD
    GLubyte   *bmA, *bmI;

    /// index to the Server size Pixel Buffer Objects
    GLuint     pbo[2];
    
    /// center of bitmap
    GLfloat    bmR;
    
    /// dimension of bitmap
    unsigned   bmD, bmS;
    
    /// allocate bitmap memory
    void allocateBitmap(real);
    
    /// draw bitmap
    void getBitmap(GLubyte*, GLuint);
    
    /// create the bitmaps
    void makeBitmaps(real);

    /// export bitmaps to file
    void saveBitmaps();
    
    /// draw using a bitmap (current raster position has to be set)
    void drawBitmap(GLubyte*, GLuint) const;
    
#endif
    
public:
    
    /**
     @defgroup PointDispPar Display parameters: Points
     @ingroup DisplayParameters
     @{
     */
    
    
    /// visibility flag : 0=hidden, 1=opaque (-1 is used in 3D for transparent objects)
    int        visible;
    
    /// color of object
    gle_color  color;
    
    /// second color (set as color[1])
    /**
     This is used to display unattached Single and unbridging Couple,
     and the inner surfaces of objects such as Sphere, Solid, Bead and Space.
     Unless it is set directly as color[1], `color2` is set to be a darker tone of `color`.
     */
    gle_color  color2;

    /// if true, use various colors to display different objects
    int        coloring;
    
    /// size of point
    real       size;
    
    /// width of line
    real       width;
    
    /// shape: circle, hexagon, star, etc.
    char       shape;
    
    /// a bitfield to set different display options
    int        style;
    
    /// character displayed, if not zero
    char       symbol;
    
    /// color of symbol
    gle_color  symbol_color;
    
    /// @}
    
public:
    
    /// constructor
    PointDisp(const std::string& k, const std::string& n);
    
    /// copy constructor
    PointDisp(PointDisp const&);
    
    /// copy assignment
    PointDisp& operator =(PointDisp const&);

    /// destructor
    ~PointDisp();
    
    /// identifies the property
    std::string kind() const { return mKind; }

    /// clear to default values
    void clear();
    
    /// set from glossary
    void read(Glossary&);
    
    /// return a carbon copy of object
    Property* clone() const { return new PointDisp(*this); }

    /// write all values
    void write_data(std::ostream&) const;
    
#ifdef POINTDISP_USES_BITMAPS

    /// recalculate bitmaps
    void      prepare(real);

    /// draw active state
    void      drawA() const { drawBitmap(bmA, pbo[0]); }

    /// draw inactive state
    void      drawI() const { drawBitmap(bmI, pbo[1]); }

#else
    
    /// recalculate bitmaps
    void      prepare(real) {}
    
    /// draw active state
    void      drawA() const { drawVectorA(); }
    
    /// draw inactive state
    void      drawI() const { drawVectorI(); }

#endif

};


#endif
