// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MODULO_H
#define MODULO_H

#include "real.h"
#include "vector.h"


/// used to implement periodic boundary conditions
class Modulo
{
public:
    
    /// constructor
    Modulo() {}

    /// destructor
    virtual ~Modulo() {}
    
    /// true if periodic boundary conditions exist in direction \a d
    virtual bool     isPeriodic(int) const { return true; }
    
    /// the d-th direction of periodicity
    virtual Vector   period(int d)   const = 0;
    
    /// set vector to its periodic representation closest to origin
    virtual void     fold(real[])    const = 0;
    
    /// remove periodic repeats in pos[], to bring it closest to ref[]
    virtual void     fold(real pos[], const real ref[]) const = 0;
    
    /// bring ref[] closest to origin, returning translation done in trans[]
    virtual void     foldOffset(real pos[], real off[])  const = 0;
    
};

#endif


