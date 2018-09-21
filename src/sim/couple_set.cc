// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "couple_set.h"
#include "couple_prop.h"
#include "fork_prop.h"
#include "crosslink_prop.h"
#include "shackle_prop.h"
#include "bridge_prop.h"
#include "glossary.h"
#include "simul.h"


/**
 @defgroup CoupleGroup Couple and Derived Activities
 @ingroup ObjectGroup
 @ingroup NewObject
 @brief A Couple contains two Hand, and can thus crosslink two Fibers.

 The plain Couple may crosslink two Fiber irrespective of their configuration.
 Derived classes implement specificity, angular stiffness, etc.
 
 List of classes accessible by specifying `couple:activity`.

 `activity`    |   Clas        | Parameters
 --------------|---------------|-----------------------------------
 `diffuse`     | Couple        | @ref CouplePar (default)
 `fork`        | Fork          | @ref ForkPar
 `crosslink`   | Crosslink     | @ref CrosslinkPar
 `bridge`      | Bridge        | @ref BridgePar
 `slide`       | Shackle       | @ref ShacklePar
 
 Example:
 @code
 set couple complex
 {
   hand1 = kinesin
   hand2 = kinesin
   stiffness = 100
   diffusion = 10
   activity = crosslink
   length = 0.02
 }
 @endcode
 */

Property* CoupleSet::newProperty(const std::string& kd, const std::string& nm, Glossary& opt) const
{
    CoupleProp * sp = 0;
    if ( kd == kind() )
    {
        std::string a;
        if ( opt.peek(a, "activity") )
        {
            if ( a == "fork" )
                sp = new ForkProp(nm);
            else if ( a == "crosslink" )
                sp = new CrosslinkProp(nm);
            else if ( a == "bridge" )
                sp = new BridgeProp(nm);
            else if ( a == "slide" )
                sp = new ShackleProp(nm);
            else if ( a == "diffuse" )
                sp = new CoupleProp(nm);
            else 
                throw InvalidParameter("unknown single:activity `"+a+"'");
        }
        if ( sp == 0 )
            sp = new CoupleProp(nm);
    }
    return sp;
}

//------------------------------------------------------------------------------

void CoupleSet::prepare(PropertyList& properties)
{
    uni = uniPrepare(properties);
}


void CoupleSet::step(FiberSet const& fibers, FiberGrid const& fgrid)
{
    // use alternate attachment strategy:
    if ( uni )
        uniAttach(fibers);

    /*
     ATTENTION: we have multiple lists, and Objects are automatically transfered
     from one list to another if their Hands bind or unbind. We ensure here that
     step() is called exactly once for each object.
     The code relies on the fact that a Couple will be moved to the start of the
     list to which it is transfered. By proceeding always from the node, which was
     first before any transfer could occur, we process each Couple only once.
     Moreover, we get the 'next' in the list always before calling 'step()', because
     'step()' may transfer the node to another list, changing the value of 'next()'
     */
    
    /*
     Cytosim::MSG(9,"CoupleSet::stepCouple entry :  free %5i, bound %5i, bridge %5i\n", 
         fList.size(), aList.size(), bList.size());
     */
    
    Couple *const ffHead = firstFF();
    Couple *const afHead = firstAF();
    Couple *const faHead = firstFA();

    
    Couple * obj, * nxt;

    
    obj = firstAA();
    while ( obj )
    {
        nxt = obj->next();
        obj->stepAA();
        obj = nxt;
    }
    
    obj = faHead;
    while ( obj )
    {
        nxt = obj->next();
        obj->stepFA(fgrid);
        obj = nxt;
    }
    
    obj = afHead;
    while ( obj )
    {
        nxt = obj->next();
        obj->stepAF(fgrid);
        obj = nxt;
    }
    
    //std::clog << "CoupleSet::step : FF " << ffList.size() << " head " << ffHead << std::endl;
    
    obj = ffHead;
    while ( obj )
    {
        nxt = obj->next();
        obj->stepFF(fgrid);
        obj = nxt;
    }
}


void CoupleSet::relax()
{
    uniRelax();
}

//------------------------------------------------------------------------------
#pragma mark -

Object * CoupleSet::newObjectT(const Tag tag, int idx)
{
    assert_true( tag == Couple::TAG );
    Property * p = simul.properties.find_or_die(kind(), idx);
    return static_cast<CoupleProp*>(p)->newCouple();
}

/**
 @addtogroup CoupleGroup

 You can attach the hands of a Couple:
 @code
 new couple protein
 {
    attach1 = INTEGER, REAL
    attach2 = INTEGER, REAL
 }
 @endcode
 
 where:
 - INTEGER designates the fiber:
     - 1 for the first fiber
     - 2 for the second, etc.
     - 0 designates the last fiber,
     - -1 is the penultimate one, etc.
     .
 - REAL is the abscissa of the attachment point
 .
 */

ObjectList CoupleSet::newObjects(const std::string& kd, const std::string& nm, Glossary& opt)
{
    ObjectList res;
    Object * obj = 0;
    
    if ( kd == kind() )
    {
        Property * p = simul.properties.find_or_die(kd, nm);
        obj = static_cast<CoupleProp*>(p)->newCouple();

        res.push_back(obj);
        
        long io = 1;
        /*
         This provides a way for the user to attach hand1:
         */
        if ( opt.set(io, "attach1") )
        {
            Fiber * fib = simul.findFiber(io);
            if ( fib == 0 )
                throw InvalidParameter("Could not find Fiber in couple::attach1");

            real abs = 0;
            opt.set(abs, "attach1", 1);
            if ( !fib->within(abs) )
                throw InvalidParameter("out of range abscissa in couple::attach1");
            
            assert_true( obj->tag() == Couple::TAG );
            static_cast<Couple*>(obj)->attachTo1(fib, abs, ORIGIN);
        }

        /*
         This provides a way for the user to attach hand2:
         */
        if ( opt.set(io, "attach2") )
        {
            Fiber * fib = simul.findFiber(io);
            if ( fib == 0 )
                throw InvalidParameter("Could not find Fiber in couple::attach2");
            
            real abs = 0;
            opt.set(abs, "attach2", 1);
            if ( !fib->within(abs) )
                throw InvalidParameter("out of range abscissa in couple::attach2");

            assert_true( obj->tag() == Couple::TAG );
            static_cast<Couple*>(obj)->attachTo2(fib, abs, ORIGIN);
        }
    }
    return res;
}

//------------------------------------------------------------------------------
#pragma mark -

void CoupleSet::link(Object * cx)
{
    assert_true( !cx->linked() );
    assert_true( cx->tag() == Couple::TAG );
    
    if ( static_cast<Couple*>(cx)->attached1() )
    {
        if ( static_cast<Couple*>(cx)->attached2() )
            aaList.push_back(cx);
        else
            afList.push_back(cx);
    }
    else
    {
        if ( static_cast<Couple*>(cx)->attached2() )
            faList.push_back(cx);
        else
            ffList.push_back(cx);
    }
}


void CoupleSet::foldPosition(const Modulo * s) const
{
    Couple * cx;
    for ( cx=firstAA(); cx; cx=cx->next() )  cx->foldPosition(s);
    for ( cx=firstFA(); cx; cx=cx->next() )  cx->foldPosition(s);
    for ( cx=firstAF(); cx; cx=cx->next() )  cx->foldPosition(s);
    for ( cx=firstFF(); cx; cx=cx->next() )  cx->foldPosition(s);
}


void CoupleSet::write(OutputWrapper & out) const
{
    ObjectSet::write(ffList, out);
    ObjectSet::write(afList, out);
    ObjectSet::write(faList, out);
    ObjectSet::write(aaList, out);
}


void CoupleSet::mix()
{
    ffList.mix(RNG);
    afList.mix(RNG);
    faList.mix(RNG);
    aaList.mix(RNG);
}


void CoupleSet::erase()
{
    uni = false;
    uniRelax();
    ffList.erase();
    afList.erase();
    faList.erase();
    aaList.erase();
    inventory.clear();
}


void CoupleSet::freeze()
{
    uniRelax();
    ffIce.transfer(ffList);
    faIce.transfer(faList);
    afIce.transfer(afList);
    aaIce.transfer(aaList);
}


void CoupleSet::thaw(bool erase)
{
    if ( erase )
    {
        forget(ffIce);
        forget(faIce);
        forget(afIce);
        forget(aaIce);
        ffIce.erase();
        faIce.erase();
        afIce.erase();
        aaIce.erase();
    }
    else
    {
        ffList.transfer(ffIce);
        faList.transfer(faIce);
        afList.transfer(afIce);
        aaList.transfer(aaIce);
    }
}


ObjectList CoupleSet::collect(bool (*func)(Object const*, void*), void* arg) const
{
    ObjectList res = ObjectSet::collect(ffList, func, arg);
    res.append( ObjectSet::collect(afList, func, arg) );
    res.append( ObjectSet::collect(faList, func, arg) );
    res.append( ObjectSet::collect(aaList, func, arg) );
    return res;
}



int CoupleSet::bad() const
{
    int code = 0;
    Couple * cxi;
    code = ffList.bad();
    if ( code ) return 100+code;
    for ( cxi=firstFF(); cxi ; cxi = cxi->next() )
    {
        if ( cxi->attached1() || cxi->attached2() )
            return 100;
    }
    
    code = afList.bad();
    if ( code ) return 200+code;
    for ( cxi=firstAF(); cxi ; cxi = cxi->next() )
    {
        if ( !cxi->attached1() || cxi->attached2() )
            return 200;
    }
    
    code = faList.bad();
    if ( code ) return 300+code;
    for ( cxi=firstFA(); cxi ; cxi = cxi->next() )
    {
        if ( cxi->attached1() || !cxi->attached2() )
            return 300;
    }
    
    code = aaList.bad();
    if ( code ) return 400+code;
    for ( cxi=firstAA(); cxi ; cxi = cxi->next() )
    {
        if ( !cxi->attached1() || !cxi->attached2() )
            return 400;
    }
    return code;
}


//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Fast Diffusion

/**
 Estimate attachment propensity per unit length of fiber
 */
real attachDensity(const HandProp * hp)
{
    real density = hp->binding_rate_dt;
#if ( DIM == 2 )
    density *= 2 * hp->binding_range;
#elif ( DIM == 3 )
    density *= M_PI * hp->binding_range * hp->binding_range;
#endif
    return density;
}


/**
 Implements a Monte-Carlo approach for attachments of free Couple,
 under the assumption that diffusion is sufficiently fast to
 maintain a uniform spatial distribution, and also assuming that
 the distribution of fibers is more-or-less uniform such that the
 attachments are uniformly distributed along the fibers.
 
 Diffusing free Couple are removed from the standard list, and this
 will skip the random walk that is used for simulating diffusion,
 and the detection of neighboring fibers used for simulating attachments.
 
 Algorithm:
 - Remove any free Couple from the simulation, transfering them to a 'reserve'.
 - Estimate how many free hands should bind, from:
   - the total length of fibers,
   - the volume of the Space,
   - the binding parameters of Hands.
   .
 - Perform the estimated number of binding events:
   - find a random position on a Fiber, uniformly according to length,
   - attach a couple from the reserve.
   .
 .
 */
void CoupleSet::uniAttach(FiberSet const& fibers, CoupleList& reserve)
{
    const size_t rsize = reserve.size();
    Couple * obj = reserve.top();
    assert_true( obj );

    CoupleProp const * cp = static_cast<CoupleProp const*>(obj->property());
    assert_true( cp->fast_diffusion );
    
    if ( cp->confineSpace() == 0 )
        throw InvalidParameter("could not get Space necessary for couple:fast_diffusion");
        
    // get Volume in which Couple are confined:
    const real volume = cp->confineSpace()->volume();
    
    if ( volume <= 0 )
        throw InvalidParameter("couple:fast_diffusion requires a non-zero space::volume");
    
    Array<FiberBinder> loc(1024, 1024);
    
    // attach Couple::hand1
    real density = rsize * attachDensity(obj->hand1()->prop);
    if ( density > 0 )
    {
        // the average distance between attachment estimated from the concentration of Hands
        fibers.uniFiberSites(loc, volume/density);
        
        for ( unsigned int s = 0; s < loc.size(); ++s )
        {
            assert_true(!obj->cHand2->attached());
            if ( obj->cHand1->attachmentAllowed(loc[s]) )
            {
                obj->cHand1->attach(loc[s]);
                reserve.pop();
                link(obj);
                if ( reserve.empty() )
                    return;
                obj = reserve.top();
            }
        }
    }
    
    assert_true(!reserve.empty());
    assert_true(obj==reserve.top());
    
    // attach Couple::hand2
    density = rsize * attachDensity(obj->hand2()->prop);
    if ( density > 0 )
    {
        // the average distance between attachment estimated from the concentration of Hands
        fibers.uniFiberSites(loc, volume/density);
        
        for ( unsigned int s = 0; s < loc.size(); ++s )
        {
            assert_true(!obj->cHand1->attached());
            if ( obj->cHand2->attachmentAllowed(loc[s]) )
            {
                obj->cHand2->attach(loc[s]);
                reserve.pop();
                link(obj);
                if ( reserve.empty() )
                    return;
                obj = reserve.top();
            }
        }
    }
}



/**
 Alternative attachment algorithm assuming fast diffusion,
 used if ( couple:fast_diffusion == true )
 
 See CoupleSet::uniAttach
 */
void CoupleSet::uniAttach(FiberSet const& fibers)
{
    // transfer free complex that fast-diffuse to the reserve
    Couple * obj = firstFF(), * nxt = obj;
    while ( nxt )
    {
        nxt = nxt->next();
        CoupleProp const* cp = static_cast<CoupleProp const*>(obj->property());
        if ( cp->fast_diffusion )
        {
            ffList.pop(obj);
            assert_true((size_t)cp->index() < uniLists.size());
            uniLists[cp->index()].push(obj);
        }
        obj = nxt;
    }
    
    // uniform attachment for reserved couples:
    for ( CoupleReserve::iterator c = uniLists.begin(); c < uniLists.end(); ++c )
        if ( ! (*c).empty() )
            uniAttach(fibers, *c);
}


/**
 
 Return true if at least one couple:fast_diffusion is true,
 and in this case allocate uniLists.
 
 The Volume of the Space is assumed to remain constant until the next uniPrepare() 
 */
bool CoupleSet::uniPrepare(PropertyList& properties)
{
    int inx = 0;
    bool res = false;
    
    PropertyList plist = properties.find_all("couple");
    
    for ( PropertyList::const_iterator n = plist.begin(); n != plist.end(); ++n )
    {
        CoupleProp const * p = static_cast<CoupleProp const*>(*n);
        if ( p->fast_diffusion )
            res = true;
        
        if ( p->index() > inx )
            inx = p->index();
    }
    
    if ( res )
        uniLists.resize(inx+1);
    
    return res;
}


/**
 empty uniLists, reversing all Couples in the normal lists.
 This is useful if ( couple:fast_diffusion == true )
 */
void CoupleSet::uniRelax()
{
    for ( CoupleReserve::iterator res = uniLists.begin(); res != uniLists.end(); ++res )
    {
        CoupleList& reserve = *res;
        while( ! reserve.empty() )
        {
            Couple * c = reserve.top();
            c->randomizePosition();
            reserve.pop();
            ffList.push_front(c);
        }
    }
}

