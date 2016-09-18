// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "space_prop.h"
#include "glossary.h"
#include "property_list.h"
#include "simul_prop.h"
#include "simul.h"

#include "space.h"
#include "space_square.h"
#include "space_sphere.h"
#include "space_polygon.h"
#include "space_capsule.h"
#include "space_banana.h"
#include "space_torus.h"
#include "space_dice.h"
#include "space_strip.h"
#include "space_periodic.h"
#include "space_ellipse.h"
#include "space_cylinder.h"
#include "space_cylinderZ.h"
#include "space_cylinderP.h"


/**
 @defgroup SpaceGroup Space and Geometry
 @ingroup ObjectGroup
 @ingroup NewObject
 @brief A Space defines a confined region
 
 A Space is created by specifying a geometry:
 @code
 set space NAME
 {
    geometry = GEOMETRY DIMENSIONS
 }
 @endcode
 
 DIMENSIONS is usually a list of numbers.
 
 List of known `geometry`:
 
 GEOMETRY      |   Class          | DIMENSIONS
 --------------|------------------|-----------------------------------
 `rectangle`   | SpaceSquare      | sizeX sizeY sizeZ
 `sphere`      | SpaceSphere      | radius
 `polygon`     | SpacePolygon     | file_name height
 `capsule`     | SpaceCapsule     | half_length radius
 `torus`       | SpaceTorus       | radius thickness
 `banana`      | SpaceBanana      | total_length width radius_of_curvature
 `dice`        | SpaceDice        | sizeX sizeY sizeZ radius
 `strip`       | SpaceStrip       | sizeX sizeY sizeZ
 `periodic`    | SpacePeriodic    | sizeX sizeY sizeZ
 `ellipse`     | SpaceEllipse     | sizeX sizeY sizeZ
 `cylinder`    | SpaceCylinder    | half_length radius
 `cylinderZ`   | SpaceCylinderZ   | half_length radius
 `cylinderP`   | SpaceCylinderP   | half_length radius

 
 Example:
 @code
 set space cell
 {
   geometry = sphere 5
 }
 @endcode
 */
Space * SpaceProp::newSpace() const
{
    Space * spc = 0;
    
    std::string s = shape;
    
    if ( s=="rectangle" || s=="square" )       spc = new SpaceSquare(this);
    if ( s=="circle" || s=="sphere" )          spc = new SpaceSphere(this);
    if ( s=="polygon" )                        spc = new SpacePolygon(this, file);
    if ( s=="capsule" || s=="spherocylinder" ) spc = new SpaceCapsule(this);
    if ( s=="torus" )                          spc = new SpaceTorus(this);
    if ( s=="banana" )                         spc = new SpaceBanana(this);
    if ( s=="dice" )                           spc = new SpaceDice(this);
    if ( s=="strip" )                          spc = new SpaceStrip(this);
    if ( s=="periodic" )                       spc = new SpacePeriodic(this);
    if ( s=="ellipse" || s=="ellipsoid" )      spc = new SpaceEllipse(this);
    if ( s=="cylinder" )                       spc = new SpaceCylinder(this);
    if ( s=="cylinderZ" )                      spc = new SpaceCylinderZ(this);
    if ( s=="cylinderP" )                      spc = new SpaceCylinderP(this);
    
    if ( spc == 0 )
        throw InvalidParameter("unknown space:shape `"+shape+"'");
    
    // set dimensions:
    spc->readLengths(dimensions);
    
    return spc;
}


//------------------------------------------------------------------------------

void SpaceProp::clear()
{
    geometry   = "";
    shape      = "undefined";
    dimensions = "";
    file       = "";
    display    = "";
}

void SpaceProp::read(Glossary& glos)
{    
    glos.set(shape,      "shape");
    glos.set(dimensions, "dimensions");
#ifdef BACKWARD_COMPATIBILITY
    glos.set(dimensions, "spec");  //BACKWARD_COMPATIBILITY format 36
#endif
    glos.set(geometry,   "geometry");
    glos.set(display,    "display");
}

//------------------------------------------------------------------------------

void SpaceProp::complete(SimulProp const* sp, PropertyList*)
{
    if ( !geometry.empty() )
    {
        std::istringstream iss(geometry);
        iss >> shape;
        
        if ( iss.fail() )
            throw InvalidParameter("invalid geometry `"+geometry+"' for Space");
        
        char c;
        while ( isspace(iss.peek()) )
            iss.get(c);
        
        if ( isalpha(iss.peek()) )
        {
            iss >> file;
        }
        
        // get all remaining characters:
        if ( iss.good() )
            dimensions = geometry.substr(iss.tellg());
    }
}

//------------------------------------------------------------------------------

void SpaceProp::write_data(std::ostream & os) const
{
    write_param(os, "geometry",   geometry);
    write_param(os, "shape",      shape);
    write_param(os, "dimensions", dimensions);
    write_param(os, "display",    "("+display+")");
}



