// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MOTOR_PROP_H
#define MOTOR_PROP_H

#include "hand_prop.h"

/// Additional Property for Motor
/**
 @ingroup Properties
 */
class MotorProp : public HandProp
{
    friend class Motor;
    
public:
    
    /**
     @defgroup MotorPar Parameters of Motor
     @ingroup Parameters
     Inherits @ref HandPar.
     Check the examples!
     @{
    */
    
    /// force at which speed becomes zero (positive)
    real    stall_force;
    
    /// speed when the force is zero
    /** 
     A positive value specifies a plus-end directed motor.
     A negative value specifies a minus-end directed motor.
     */
    real    max_speed;
    
    /// if true, the speed is limited to the range [0, 2*max_speed]
    /** 
     With ( limit_speed = 1 ), a plus-end directed motor will not move towards the minus-end,
     and also will not exceed 2x its max speed.
     For a minus-end directed motor, the permitted range is [2*max_speed, 0], 
     thus also excluding backward steps.
     */
    bool    limit_speed;
    
    /// @}
    //------------------ derived variables below ----------------
    
    /// limits for a displacement in one time_step apply if ( limit_speed = true )
    real    min_dabs, max_dabs;
    
private:
    
    real max_speed_dt, abs_speed_dt, var_speed_dt;
    
public:

    /// constructor
    MotorProp(const std::string& n) : HandProp(n)  { clear(); }
    
    /// destructor
    ~MotorProp() { }
    
    /// return a Hand with this property
    virtual Hand * newHand(HandMonitor* h) const;
    
    /// set default values
    void clear();
    
    /// set from a Glossary
    void read(Glossary&);
    
    /// compute values derived from the parameters
    void complete(SimulProp const*, PropertyList*);
    
    /// perform more checks, knowing the elasticity
    void checkStiffness(real stiff, real len, real mul, real kT) const;
    
    /// return a carbon copy of object
    Property* clone() const { return new MotorProp(*this); }

    /// write all values
    void write_data(std::ostream &) const;    
    
};

#endif

