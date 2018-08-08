// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
/**
 @file
 @brief Common definitions: FiberEnd, etc.
 */

#ifndef COMMON_H
#define COMMON_H

//------------------------------------------------------------------------------

/// Designates the tip of a Fiber, but also the origin and center points
enum FiberEnd
{
    NOT_END     = 0,   ///< not an end
    PLUS_END    = 1,   ///< highest abscissa = last model-point
    MINUS_END   = 2,   ///< lowest abscissa = fist model-point at index 0
    NEAREST_END = 3,   ///< used to designate the closest of the two ends
    ORIGIN      = 7,   ///< refers to the origin of abscissa
    CENTER      = 8    ///< the mid-point between the two ends
};

//------------------------------------------------------------------------------

/// Possible dynamic states for the tip of a Fiber [dynamic instability]
/** 
 We prefer to not be too specific here about the nature of the different states,
 to allow the interpretation to be different in different types of Fiber.
 */
enum AssemblyState
{
    STATE_WHITE  = 0,   ///<  Used to indicate a non-dynamic end
    STATE_GREEN  = 1,   ///<  First dynamic state: usually growing
    STATE_YELLOW = 2,   ///<  Intermediate dynamic state
    STATE_ORANGE = 3,   ///<  Intermediate dynamic state
    STATE_RED    = 4    ///<  Third dynamic state: usually shrinking
};

//------------------------------------------------------------------------------

/// Possible actions, when a Fiber becomes shorter that the allowed length
enum Fate
{
    FATE_DESTROY = 1,   ///< the Fiber is destroyed: \a destroy
    FATE_RESCUE  = 2,   ///< the Fiber is rescued: \a rescue
    FATE_NONE    = 3    ///< nothing is done: \a none
};

//------------------------------------------------------------------------------


/// Possible modes of confinements
enum Confinement
{
    CONFINE_NOT         = 0,   ///< not confined
    CONFINE_INSIDE      = 1,   ///< confine inside the Space
    CONFINE_OUTSIDE     = 2,   ///< confine outside the Space
    CONFINE_SURFACE     = 3,   ///< confine on the surface of the Space
    CONFINE_ALL_INSIDE  = 4,   ///< confine the entire bead inside
    CONFINE_ALL_OUTSIDE = 5,   ///< confine the entire bead outside
    CONFINE_PLUS_END    = 10,  ///< confine the PLUS_END of fibers to the surface of the Space
    CONFINE_MINUS_END   = 11,  ///< confine the MINUS_END of fibers to the surface of the Space
};



#endif


