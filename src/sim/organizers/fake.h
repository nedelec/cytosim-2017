// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FAKE_H
#define FAKE_H

#include "object.h"
#include "organizer.h"
#include "fake_prop.h"
#include "point_exact.h"
class Solid;

//------------------------------------------------------------------------------

///a set of two asters held together by a Solid 
/**
 This object cannot handle the destruction of the Asters
 */

class Fake : public Organizer
{

private:

    /// Property
    FakeProp const* prop;
    
    /// connections
    std::vector<PointExact> asterPoints, solidPoints;

    /// central Solid
    Solid *  mSolid;
    
public:
    
    /// constructor
    Fake(FakeProp const* p) : prop(p), mSolid(0) { }
 
    /// construct all the dependent Objects of the Organizer
    ObjectList build(Glossary&, Simul&);
 
    /// destructor  
    virtual   ~Fake();

    /// perform one Monte-Carlo step
    void       step();
    
    /// add interactions to the Meca
    void       setInteractions(Meca &) const;
    
    /// return pointer to central Solid
    Solid *    solid() const { return mSolid; }
    
    //------------------------------ read/write --------------------------------

    /// a unique character identifying the class
    static const Tag TAG = 'k';
    
    /// return unique character identifying the class
    Tag        tag() const { return TAG; }
    
    /// return Object Property
    const Property* property() const { return prop; }
    
 };


#endif

