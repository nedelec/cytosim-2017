// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef ORGANIZER_H
#define ORGANIZER_H

#include "assert_macro.h"
#include "mecable.h"
#include "buddy.h"

class Meca;
class Simul;
class PointDisp;

/// an assemblage of Object
/** 
Organizer contains an Array of pointers of type Mecable*.
These Mecables are organized by Organizer::setInteraction()
which is implemented in the derived classes, eg. Bundle, Aster & Nucleus.
*/
class Organizer: public Object, private Buddy
{
    
    friend class OrganizerSet;
    
private:

    typedef std::vector<Mecable*> MecableList;
    
    /// list of Object beeing organized
    MecableList   objs;
    
public:

    /// default constructor
    Organizer() { }
    
    /// destructor
    virtual      ~Organizer();
    
    /// construct all the dependent Objects of the Organizer
    virtual ObjectList    build(Glossary&, Simul&) = 0;

    //--------------------------------------------------------------------------

    /// number of objects currently organized
    size_t                nbOrganized() const  { return objs.size(); }
    
    /// return Mecable at index \a n
    Mecable *             organized(unsigned int n) const { assert_true(n<objs.size()); return objs[n]; }
    
    /// add Mecable at end of list
    void                  grasp(Mecable *);

    /// add Mecable at index \a n
    void                  grasp(Mecable *, unsigned int);

    /// handles the disapearance of one of the organized object
    void                  goodbye(Buddy *);
    
    //--------------------------------------------------------------------------

    /// return the center of gravity
    virtual Vector        position() const;

    /// return the average of object->posPoint(0)
    virtual Vector        positionP(unsigned int) const;
    
    /// monte-carlo simulation step
    virtual void          step() {}
    
    /// add interactions to the Meca
    virtual void          setInteractions(Meca &) const {}

    //--------------------------------------------------------------------------
    
    /// move all associated objects
    virtual void          translate(Vector const& T);
    
    /// transform all associated objects
    virtual void          rotate(Rotation const& T);
    
    //--------------------------------------------------------------------------

    /// number of connections to be displayed
    virtual unsigned int  nbLinks() const { return 0; }
    
    /// first position of connection \a ii
    virtual Vector        posLink1(unsigned int ii) const { return Vector(0,0,0); }
    
    /// second position of connection \a ii
    virtual Vector        posLink2(unsigned int ii) const { return Vector(0,0,0); }
    
    /// display parameters 
    virtual PointDisp *   pointDisp() const { return 0; }
    
    //--------------------------------------------------------------------------
    
    /// a static_cast<> of Node::next()
    Organizer *   next()  const  { return static_cast<Organizer*>(nNext); }
    
    /// a static_cast<> of Node::prev()
    Organizer *   prev()  const  { return static_cast<Organizer*>(nPrev); }
    
    //--------------------------------------------------------------------------

    /// read
    void          read(InputWrapper&, Simul&);
    
    /// write
    void          write(OutputWrapper&) const;
};



#endif
