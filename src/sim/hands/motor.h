// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MOTOR_H
#define MOTOR_H

#include "hand.h"
class MotorProp;

/// a Hand that can move smoothly on a Fiber
/**
 The Motor is a Hand, and thus can bind and unbind from fibers.
 
 A bound Motor can move along its fiber.
 The direction of the motor is set by the sign of @ref MotorPar "max_speed".
 If the speed is positive, the motor attempts to move towards the PLUS_END.
 
 The speed is linearly proportional to the load of the motor.
 The load is the projection of the force vector on the direction of the fiber.
 
 @code
 real load = force * direction_of_fiber;
 real speed = max_speed * ( 1 + load / stall_force );
 @endcode
 
 The actual movement is calculated from the time step:
 @code
 real displacement = speed * time_step;
 @endcode
 
 
 As defined in Hand, detachment increases exponentially with force.

 See Examples and the @ref MotorPar.
 @ingroup HandGroup
 */
class Motor : public Hand
{
private:
    
    /// disabled default constructor
    Motor();
    
    /// Property
    MotorProp const* prop;
    
    /// clamp a in [0,b]
    void limitSpeedRange(real& a, const real b);

public:
    
    /// constructor
    Motor(MotorProp const* p, HandMonitor* h);
    
    /// destructor
    ~Motor() {}
    
    
    /// simulate when \a this is attached but not under load
    void   stepUnloaded();
    
    /// simulate when \a this is attached and under load
    void   stepLoaded(Vector const & force);
    
};

#endif

