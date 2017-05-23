// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#include "object_set.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "glossary.h"
#include "modulo.h"
#include "space.h"
#include "simul.h"

extern Modulo * modulo;

//------------------------------------------------------------------------------

/**
 The object is added at the end of the list
 */
void ObjectSet::link(Object * obj)
{
    assert_true( !obj->linked() );
    nodes.push_back(obj);
}

/**
 relink() should only be called for object already linked()
 */
void ObjectSet::relink(Object * obj)
{
    assert_true( obj->linked() );
    obj->list()->pop(obj);
    link(obj);
}

//------------------------------------------------------------------------------

/**
 Translate all objects for which ( Object::translatable()==true ) by \a vec
 */

void ObjectSet::translateObjects(ObjectList const& objs, Vector const& vec)
{
    for ( ObjectList::iterator oi = objs.begin(); oi < objs.end(); ++oi )
    {
        Movable * mv = *oi;
        if ( mv  &&  mv->translatable() )
            mv->translate(vec);
    }
}

/**
 Rotate all objects in \a objs around the origin.
 
 If ( Object::rotatable()==false ) but ( Object::translatable()==true ), 
 the Object is translated by \a [ rot * Object::position() - Object::position() ]
 */

void ObjectSet::rotateObjects(ObjectList const& objs, Rotation const& rot)
{
    for ( ObjectList::iterator oi = objs.begin(); oi < objs.end(); ++oi )
    {
        Movable * mv = *oi;
        if ( mv )
        {
            if ( mv->rotatable() )
                mv->rotate(rot);
            else if ( mv->translatable() )
            {
                Vector pos = mv->position();
                mv->translate(rot*pos-pos);
            }
        }
    }
}


/**
 @return the position where the objects were placed
 @todo implement keyword 'transformation'
 */

Vector ObjectSet::placeObjects(ObjectList const& objs, Glossary& opt, const Space* spc)
{
    std::string str;
    
    // Position
    Vector pos(0,0,0);
    if ( opt.set(str, "position") )
    {
        std::istringstream iss(str);
        pos = Movable::readPosition(iss, spc);
    }
    else if ( spc )
        pos = spc->randomPlace();
    
    // Rotation
    Rotation rot;
    if ( opt.set(str, "orientation") )
    {
        std::istringstream iss(str);
        rot = Movable::readRotation(iss, pos, spc);
    }
    else if ( opt.set(str, "direction") )
    {
        std::istringstream iss(str);
        Vector vec = Movable::readDirection(iss, pos, spc);
        rot = Rotation::rotationToVector(vec, RNG);
    }
    else
        rot = Rotation::randomRotation(RNG);
    
    rotateObjects(objs, rot);

    // objects are wrapped within periodic-boundaries:
    if ( modulo )
        modulo->fold(pos);
    
    translateObjects(objs, pos);
    return pos;
}


/**
 - This returns a list of Objects, that are not necessarily all of the same class.
 For example, the list may contain a Solid, and any number of attached Singles.
 - These Objects are not yet linked in the Simul.
 - They are placed and oriented in Space according to the options provided in 'opt'.
 .
 This calls newObjects()
 
 @code
 new INTEGER CLASS NAME
 {
   position = POSITION
   placement = PLACEMENT, SPACE_NAME
   nb_trials = INTEGER
 }
 @endcode

 
 PLACEMENT can be:
 - if placement = \a inside (default), it tries to find a place inside the Space
 - if placement = \a anywhere, the position is returned
 - if placement = \a outside, the object is created only if it is outside the Space
 - if placement = \a surface, the position is projected on the edge of current Space
 .
 
 By default, the specifications are relative to the last Space that was defined,
 but a different space can be specified as second argument of PLACEMENT.
 
 You can set the density of objects by setting nb_trials=1:
 @code
 new 100 single grafted
 {
    position = ( rectangle 10 10 )
    nb_trials = 1
 }
 @endcode
 This way an object will be created only if the position falls inside,
 and the density will be exactly 100/10*10 = 1.
 */
ObjectList ObjectSet::newPlacedObjects(const std::string& kd, const std::string& nm, Glossary& opt)
{
    int n = 0, nb_trials = 1000;
    opt.set(nb_trials, "nb_trials");
    
    Confinement placement = CONFINE_INSIDE;
    
    opt.set(placement, "placement",
            KeyList<Confinement>("anywhere", CONFINE_NOT,
                                 "inside",   CONFINE_INSIDE,
                                 "outside",  CONFINE_OUTSIDE,
                                 "surface",  CONFINE_SURFACE));
    
    // space is current Space or as specified in placement[1]
    const Space* spc = simul.space();
    std::string str;
    if ( opt.set(str, "placement", 1) )
        spc = simul.findSpace(str);

    
    //we may bail out after 'nb_trials' unsuccessful attempts
    while ( ++n <= nb_trials )
    {
        ObjectList res = newObjects(kd, nm, opt);
        
        if ( res.empty() || res[0]==0 )
            throw InvalidParameter("failed to create "+kd+" `"+nm+"'");
        
        Vector pos = placeObjects(res, opt, spc);
        
        if ( spc == 0 || placement == CONFINE_NOT )
            return res;

        if ( placement == CONFINE_SURFACE )
        {
            Vector prj;
            spc->project(pos, prj);
            translateObjects(res, prj-pos);
            return res;
        }
        if ( spc->inside(pos) )
        {
            if ( placement == CONFINE_INSIDE )
                return res;
        }
        else
        {
            if ( placement == CONFINE_OUTSIDE )
                return res;
        }
        
        //delete objects if the placement was not fulfilled:
        for ( ObjectList::iterator oi = res.begin(); oi < res.end(); ++oi )
        {
            delete(*oi);
            *oi = 0;
        }
    }
    
    //std::cerr << "ObjectSet::newPlacedObjects() exceeded " << nb_trials << " trials" << std::endl;
    //return an empty list:
    return ObjectList();
}




//------------------------------------------------------------------------------
#pragma mark -

void ObjectSet::freeze()
{
    ice.transfer(nodes);
}


void ObjectSet::forget(NodeList & list)
{
    Node * n = list.first();
    while ( n )
    {
        inventory.unassign(static_cast<Object*>(n));
        n = n->next();
    }
}


void ObjectSet::thaw(bool erase)
{
    if ( erase )
    {
        forget(ice);
        ice.erase();
    }
    else
    {
        nodes.transfer(ice);
    }
}


//------------------------------------------------------------------------------
#pragma mark -

void ObjectSet::add(Object * obj)
{
    if ( obj->linked() )
    {
        fprintf(stderr, "Warning: ObjectSet::add(linked Object)\n");
        return;
    }
    inventory.assign(obj);
    link(obj);
    //std::cerr << "ObjectSet::add(" <<  obj->reference() << ")\n";
}


void ObjectSet::add(ObjectList & objs)
{
    for ( ObjectList::iterator oi = objs.begin(); oi < objs.end(); ++oi )
        add(*oi);
}


void ObjectSet::remove(Object * obj)
{
    //std::cerr << "ObjectSet::remove(" <<  obj->reference() << ")\n";
    assert_true( obj->linked() );
    assert_true( obj->objset()==this );
    inventory.unassign(obj);
    obj->list()->pop(obj);
}


void ObjectSet::remove(ObjectList & objs)
{
    for ( ObjectList::iterator oi = objs.begin(); oi < objs.end(); ++oi )
        remove(*oi);
}


void ObjectSet::erase(Object * obj)
{
    remove(obj);
    delete(obj);
}


void ObjectSet::erase()
{
    nodes.erase();
    inventory.clear();
}



/**
 if ( num > 0 ) this will call Inventory::find()
 if ( num < 0 ) objects from the end of the list will be returned:
 num = 0 is the last object
 num = -1 is the previous one,
 etc.
 */
Object * ObjectSet::findObject(long num) const
{
    if ( num > 0 )
        return static_cast<Object*>(inventory.get(num));
    else
    {
        Object * res = last();
        while ( res  &&  ++num <= 0 )
            res = res->prev();
        return res;
    }
}


/**
 return the first object encountered with the given property,
 but it can be any one of them, since the lists are regularly
 shuffled to randomize the order in the list.
 */
Object * ObjectSet::first(const Property* prop) const
{
    for ( Object* obj=first(); obj; obj=obj->next() )
        if ( obj->property() == prop )
            return obj;
    return 0;
}


ObjectList ObjectSet::collect(const NodeList & list,
                              bool (*func)(Object const*, void*), void* arg)
{
    ObjectList res;
    Node * n = list.first();
    while ( n )
    {
        Object * obj = static_cast<Object*>(n);
        n = n->next();
        if ( func(obj, arg) )
            res.push_back(obj);
    }
    return res;
}


ObjectList ObjectSet::collect(bool (*func)(Object const*, void*), void* arg) const
{
    return collect(nodes, func, arg);
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 This writes a new line, the reference and the Object data, for each Object in \a list
 */
void ObjectSet::write(const NodeList& list, OutputWrapper & out)
{
    for ( Node const* n=list.first(); n; n=n->next() )
    {
        const Object * o = static_cast<const Object*>(n);
        out.write('\n');
        o->writeReference(out);
        o->write(out);
    }
}

/**
 We do not allow property()->index() of an Object to change during a file import.
 However, there is no structural reason for that in cytosim.
 If necessary, it should be possible to remove this constraint.
 */
void ObjectSet::readObject(InputWrapper& in, const Tag tag, const char pretag)
{
    int ix = 0, mk = 0;
    Number nb = 0;
    
    Object::readReference(in, ix, nb, mk, pretag);
    
    if ( nb == 0 )
        throw InvalidSyntax("Invalid (null) object reference");
    
    Object * w = find(nb);
    
    if ( w )
    {
        //std::cerr << "read " << w->reference() << std::endl;
        assert_true( w->number() == nb );
        assert_true( w->linked() );
        w->mark(mk);
        w->read(in, simul);
        assert_true(w->property());
        if ( w->property()->index() != ix )
        {
            throw InvalidSyntax("The property of a `"+w->property()->kind()+"' should not change!");
        }
        relink(w);
    }
    else
    {
        //std::cerr << "creating " << tag << nb << std::endl;
        w = newObjectT(tag, ix);
        w->number(nb);
        w->mark(mk);
        //std::cerr << "read " << w->reference() << std::endl;
        w->read(in, simul);
        add(w);
    }
}



