// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef POINT_EXACT_H
#define POINT_EXACT_H

#include "vector.h"
#include "mecable.h"

/// Indicates a point of interest in a Mecable
class PointExact 
{
    friend class PointInterpolated;
    
private:
    
    /// Mecable containing the point-of-interest 
    Mecable const* mPS;
    
    /// Index of the point-of-interest in the Mecable
    unsigned int   mPoint;
    
public:
        
    /// Reset the member variables (mPS=0)
    void clear()                                   { mPS = 0;  mPoint = 0; }
    
    /// Default constructor reset variables
    PointExact()                                   { mPS = 0;  mPoint = 0; }

    
    /// Build to refer to point p in ps
    PointExact(const Mecable * ps, unsigned int p) { mPS = ps; mPoint = p; }
    
    /// Set to refer to point p in ps
    void   set(const Mecable * ps, unsigned int p) { mPS = ps; mPoint = p; }
    
    /// Constant pointer to the Mecable 
    Mecable const*      mecable()            const { return mPS; }
    
    /// true if the pointer seems to be valid.
    bool                valid()              const { return mPS == 0 || mPoint < mPS->nbPoints(); }
    
    /// Index of point in object
    unsigned int        point()              const { return mPoint; }
        
    /// Position of the point-of-interest in space
    Vector              pos()                const { return mPS->posPoint(mPoint); }
    
    /// Index of the point-of-interest in the big matrix
    Matrix::index_type  matIndex()           const { return mPS->matIndex() + mPoint; }
    
    /// Write to file
    void                write(OutputWrapper&) const;
    
    /// Read from file
    void                read(InputWrapper&, Simul&);
    
    /// test if \a this shares one point with the argument
    bool                overlapping(const PointExact &) const;

    /// test if \a this is one point away from the argument
    bool                neighbors(const PointExact &) const;

    /// print for debug purpose
    friend std::ostream & operator << (std::ostream &, PointExact const&);

};


#endif
