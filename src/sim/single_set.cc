// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "single_set.h"
#include "single_prop.h"
#include "glossary.h"
#include "iowrapper.h"

#include "simul.h"
#include "wrist.h"
#include "wrist_long.h"

//------------------------------------------------------------------------------
/**
 @copydetails SingleGroup
 */
Property* SingleSet::newProperty(const std::string& kd, const std::string& nm, Glossary& opt) const
{
    if ( kd == kind() )
        return new SingleProp(nm);
    else
        return 0;
}

//------------------------------------------------------------------------------
void SingleSet::step(FiberSet const&, FiberGrid const& fgrid)
{
    /*
     ATTENTION: we have multiple lists, and Objects are automatically 
     transfered from one list to another if their Hand bind or unbind.
     We avoid here calling step() twice for the same object, by relying on the
     fact that a transfered node would be linked at the end of the new list.
     This is done by stopping after the node, which was last in the list,
     when the routine started.
     */
    
    const Node *const fLast = fList.last();
    const Node *const aLast = aList.last();
    
    if ( fLast )
    {
        Single * obj = firstF(), * nxt;
        do {
            nxt = obj->next();
            obj->stepFree(fgrid);
            if ( obj == fLast ) break;
            obj = nxt->next();
            nxt->stepFree(fgrid);
        } while ( nxt != fLast );
    }

    if ( aLast )
    {
        Single * obj = firstA(), * nxt;
        do {
            nxt = obj->next();
            obj->stepAttached();
            if ( obj == aLast ) break;
            obj = nxt->next();
            nxt->stepAttached();
        } while ( nxt != aLast );
    }
}

//------------------------------------------------------------------------------
void SingleSet::erase()
{
    fList.erase();
    aList.erase();
    inventory.clear();
}

//------------------------------------------------------------------------------
Object * SingleSet::newObjectT(const Tag tag, int ix)
{
    Property * p = simul.properties.find_or_die(kind(), ix);
    SingleProp * sp = static_cast<SingleProp*>(p);

    if ( tag == Wrist::TAG )
        return sp->newWrist(PointExact(0,0));
    
    if ( tag == Single::TAG )
        return sp->newSingle();

    return 0;
}

/**
 @addtogroup SingleGroup
 
 A newly created Single can be anchored to a Mecable:
 @code
 new single NAME {
   base = CLASS_NAME, INTEGER_REF, INTEGER_INDEX
 }
 @endcode
 
 where:
 - CLASS_NAME is the name of the object class (eg. solid)
 - INTEGER_REF designates the object:
     - 1 for first object
     - 2 for second...
     - 0 designates the last object,
     - -1 is the penultimate one, etc.
     .
 - INTEGER_INDEX designates a point on this object:
     - 0 = first point
     - 1 = second point...
     .
 .
 
 if INTERGER1 is negative, the last object is used.
 */
Object * SingleSet::newObject(const std::string& kd, const std::string& nm, Glossary& opt)
{
    if ( kd == kind() )
    {
        Property * p = simul.properties.find_or_die(kd, nm);
        SingleProp * sp = static_cast<SingleProp*>(p);
        
        std::string nam;
        if ( opt.set(nam, "base") )
        {
            long io = 1;
            unsigned ip = 0;
            opt.set(io, "base", 1);  // object number
            opt.set(ip, "base", 2);  // index of point
            Mecable * mec = static_cast<Mecable*>(simul.findObject(nam, io));
            if ( mec == 0 )
                throw InvalidParameter("Could not find Mecable in single:base");
            if ( ip >= mec->nbPoints() )
                throw InvalidParameter("index out of range in single:base");
            
            return sp->newWrist(PointExact(mec, ip));
        }
        return sp->newSingle();
    }
    return 0;
}


/**
 @addtogroup SingleGroup

 You can attach a Single to a fiber:
 @code
 new single protein
 {
    attach = INTEGER, REAL
 }
 @endcode
 
 where:
 - INTEGER designates the fiber:
    - 1 for the first fiber
    - 2 for the second, etc
    - a negative number indicates the last fiber created
 - REAL is the abscissa of the attachment point (0=MINUS_END)
 .
 */
ObjectList SingleSet::newObjects(const std::string& kind, const std::string& prop, Glossary& opt)
{
    ObjectList res;
    Object * obj = newObject(kind, prop, opt);
    if ( obj )
    {
        res.push_back(obj);
        
        /*
         This provides a way for the user to attach the Single to an existing fiber
         */
        long io = 0;
        if ( opt.set(io, "attach") )
        {
            Fiber * fib = simul.findFiber(io);
            if ( fib == 0 )
                throw InvalidParameter("Could not find Fiber in single::attach");
            
            real abs = 0;
            opt.set(abs, "attach", 1);
            if ( !fib->within(abs) )
                throw InvalidParameter("out of range abscissa in single:attach");
            
            static_cast<Single*>(obj)->attachTo(fib, abs, ORIGIN);
        }
    }
    return res;
}


//------------------------------------------------------------------------------
void SingleSet::link(Object * gh)
{
    assert_true( gh->tag()==Single::TAG  ||  gh->tag()==Wrist::TAG );
    assert_true( !gh->linked() );

    if ( static_cast<Single*>(gh)->attached() )
        aList.push_back(gh);
    else
        fList.push_back(gh);
}

//------------------------------------------------------------------------------
void SingleSet::removeWrists(Object * obj)
{
    Single * ghi = firstF();
    Single * gh  = ghi;
    // check free list
    while ( gh )
    {
        ghi = ghi->next();
        if ( gh->foot() == obj )
            delete(gh);
        gh = ghi;
    }
    
    // check attached list
    ghi = firstA();
    gh  = ghi;
    while ( gh )
    {
        ghi = ghi->next();
        if ( gh->foot() == obj )
            delete(gh);
        gh = ghi;
    }
}

//------------------------------------------------------------------------------
unsigned int SingleSet::size() const
{
    return aList.size() + fList.size();
}

void SingleSet::mix()
{
    aList.mix(RNG);
    fList.mix(RNG);
}

void SingleSet::freeze()
{
    fIce.transfer(fList);
    aIce.transfer(aList);
}

void SingleSet::thaw(bool erase)
{
    if ( erase )
    {
        forget(fIce);
        forget(aIce);
        fIce.erase();
        aIce.erase();
    }
    else
    {
        fList.transfer(fIce);
        aList.transfer(aIce);
    }
}


ObjectList SingleSet::collect(bool (*func)(Object const*, void*), void* arg) const
{
    ObjectList res = ObjectSet::collect(fList, func, arg);
    res.append( ObjectSet::collect(aList, func, arg) );
    return res;
}


//------------------------------------------------------------------------------
void SingleSet::foldPosition(const Modulo * s) const
{
    Single * gh;
    for ( gh=firstF(); gh; gh=gh->next() )  gh->foldPosition(s);
    for ( gh=firstA(); gh; gh=gh->next() )  gh->foldPosition(s);
}

void SingleSet::write(OutputWrapper & out) const
{
    ObjectSet::write(aList, out);
    ObjectSet::write(fList, out);
}

//------------------------------------------------------------------------------
int SingleSet::bad() const
{
    int code = 0;
    Single * ghi;
    code = fList.bad();
    if ( code ) return code;
    for ( ghi = firstF(); ghi ; ghi=ghi->next() )
    {
        if ( ghi->attached() ) return 100;
    }
    
    code = aList.bad();
    if ( code ) return code;
    for ( ghi = firstA();  ghi ; ghi=ghi->next() )
    {
        if ( !ghi->attached() ) return 101;
    }
    
    return code;
}

