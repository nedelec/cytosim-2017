// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "fiber_set.h"
#include "iowrapper.h"
#include "glossary.h"
#include "fiber_prop.h"
#include "dynamic_fiber_prop.h"
#include "classic_fiber_prop.h"
#include "treadmilling_fiber_prop.h"
#include "messages.h"
#include "picket.h"
#include "simul.h"
#include "sim.h"

extern Random RNG;

//------------------------------------------------------------------------------

/**
 @defgroup FiberGroup Fiber and Derived Activities
 @ingroup ObjectGroup
 @ingroup NewObject
 @brief The default Fiber is of fixed length, but derived class can change length.

 A fiber is a filament of constant length.
 Derived classes are available, where different models of how length may change
 have been implemented.
 
 List of classes accessible by specifying `fiber:activity`.
 
 `activity`    |   Class             | Parameter
 --------------|---------------------|-----------------------------------
 `none`        | Fiber               | @ref FiberPar (default)
 `classic`     | ClassicFiber        | @ref ClassicFiberPar
 `dynamic`     | DynamicFiber        | @ref DynamicFiberPar
 `treadmill`   | TreadmillingFiber   | @ref TreadmillingFiberPar
 
 */
Property* FiberSet::newProperty(const std::string& kd, const std::string& nm, Glossary& opt) const
{
    if ( kd == kind() )
    {
        std::string a;
        if ( opt.peek(a, "activity") )
        {
            if ( a == "classic" )
                return new ClassicFiberProp(nm);
            else if ( a == "dynamic" )
                return new DynamicFiberProp(nm);
            else if ( a == "treadmill" || a == "grow" )
                return new TreadmillingFiberProp(nm);
            else if ( a == "none" )
                return new FiberProp(nm);
            else
                throw InvalidParameter("unknown fiber:activity `"+a+"'");
        }
        return new FiberProp(nm);
    }
    return 0;
}


/**
 @addtogroup FiberGroup 
 @{
 <hr>
 
 You may add a Picket to immobilize a Fiber to the ground:
 
 @code
 new fiber microtubule
 {
   single = NAME_OF_SINGLE, MODE
 }
 @endcode
 
 NAME should be the name of a defined single.
 Possible MODE:
 - minus_end
 - plus_end
 - minus_dir, [distance]
 - center
 .
 
 For MODE == minus_dir, 2 Singles are added, 
 and their distance should be specified as the third argument:
 @code
 new fiber actin
 {
   single = NAME_OF_SINGLE, MODE, DISTANCE
 }
 @endcode
 
 
 Add Couple pre-attached to the Fiber:
 
 @code
 new fiber microtubule
 {
   couple = NAME_OF_COUPLE, NUMBER, LEN
 }
 @endcode

 Couples are attached via their first Hand,
 and they are distributed along Fiber, at distance LEN from the MINUS_END.
 If LEN is not specified, they are distributed all along the fiber.

 @}
 */
ObjectList FiberSet::newObjects(const std::string& kd, const std::string& nm, Glossary& opt)
{
    ObjectList res;
    Fiber * obj = 0;
    if ( kd == kind() )
    {
        Property * p = simul.properties.find_or_die(kd, nm);
        obj = static_cast<FiberProp*>(p)->newFiber(opt);
    }
    
    //add optional singles to the Fiber:
    if ( obj )
    {
        res.push_back(obj);
        assert_true( obj->tag()==Fiber::TAG );
        Fiber * fib = static_cast<Fiber*>(obj);

        std::string nam, mod;
        if ( opt.set(nam, "single") && opt.set(mod, "single", 1) )
        {
            SingleProp * sip = simul.findSingleProp(nam);
            
            if ( mod == "minus_end" )
            {
                Single * gh = sip->newSingle();
                gh->setPosition(fib->posEnd(MINUS_END));
                gh->attachToEnd(fib, MINUS_END);
                res.push_back(gh);
            }
            else if ( mod == "plus_end" )
            {
                Single * gh = sip->newSingle();
                gh->setPosition(fib->posEnd(PLUS_END));
                gh->attachToEnd(fib, PLUS_END);
                res.push_back(gh);
            }   
            else if ( mod == "center" )
            {
                Single * gh = sip->newSingle();
                gh->setPosition(fib->posEnd(CENTER));
                gh->attachToEnd(fib, CENTER);
                res.push_back(gh);
            }   
            else if ( mod == "minus_dir" )
            {
                real len = 1;
                opt.set(len, "single", 2);
                
                Single * gh = sip->newSingle();
                gh->setPosition(fib->posEnd(MINUS_END));
                gh->attachToEnd(fib, MINUS_END);
                res.push_back(gh);
                
                gh = sip->newSingle();
                gh->setPosition(fib->pos(len, MINUS_END));
                gh->attachTo(fib, len, MINUS_END);
                res.push_back(gh);
            }
            else
            {
                throw InvalidParameter("unknown fiber:single type");
            }
        }

        if ( opt.set(nam, "couple") )
        {
            CoupleProp * cop = static_cast<CoupleProp*>(simul.properties.find_or_die("couple", nam));
            int cnt = 1;
            opt.set(cnt, "couple", 1);
            real len = fib->length();
            opt.set(len, "couple", 2);
            
            for ( int n = 0; n < cnt; ++n )
            {
                real abs = fib->abscissa(len * n / (cnt-1), MINUS_END);
                if ( fib->within(abs) )
                {
                    Couple * cp = new Couple(cop, fib->posM(abs));
                    cp->attachTo1(fib, abs);
                    res.push_back(cp);
                }
            }
        }
        
    }
    return res;
}

/**
 The fiber is not initialized, since this is used for file input
 */
Object * FiberSet::newObjectT(const Tag tag, int idx)
{
    Property * p = simul.properties.find_or_die(kind(), idx);
    Fiber* fib = static_cast<FiberProp*>(p)->newFiber();

#ifdef BACKWARD_COMPATIBILITY
    assert_true( fib  &&  fib->tag()==tag  || tag=='m' );
#else
    assert_true( fib  &&  fib->tag()==tag );
#endif
    
    return fib;
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 Calculate the free monomer concentration. 
 Calls step() once for every Fiber.
 */

void FiberSet::step()
{
    PropertyList plist = simul.properties.find_all(kind());
    
    // calculate the total length for each kind of Fiber:
    for ( unsigned int k = 0; k < plist.size(); ++k )
        static_cast<FiberProp*>(plist[k])->total_length = 0;

    for ( Fiber const* fib = first(); fib; fib = fib->next() )
        const_cast<FiberProp*>(fib->prop)->total_length += fib->length();
    
    // calculate the ratio of free polymer:
    for ( unsigned int k = 0; k < plist.size(); ++k )
    {
        FiberProp * p = static_cast<FiberProp*>(plist[k]);        
        if ( p->total_polymer > 0 )
        {
            // update the normalized monomer concentration:
            p->free_polymer = 1.0 - p->total_length / p->total_polymer;
            
            if ( p->free_polymer < 0 )
            {
                Cytosim::warning("Uhoo: The free monomer concentration is negative!!!\n");
                //this should not happen
                p->free_polymer = 0;
            }
        }
        else
            p->free_polymer = 1.0;
    }

    
    /*
     Fiber::step() may call Fiber::sever(), and that will add
     new fibers at the end of the fiber_list.
     Hence we continue until the end of the list, to consider all Fibers.
     
     In short, this calls step() once for every Fiber
     */
    Fiber * obj = first();

    while ( obj )
    {
        Fiber * nxt = obj->next();
        obj->step();
        obj = nxt;
    }
}


//------------------------------------------------------------------------------
/**
 Cut all Fibers along the plane defined by n.x + a = 0.
 function argument `func` can be specified to specify which fibers can be cut.
 If ( func != 0 ), a fiber `fib` will be cut only if
 @code
 func(fib, arg) == true
 @endcode
 */
void FiberSet::cutAlongPlane(Vector const& n, const real a,
                             bool (*func)(Object const*, void* arg), void* arg)
{
    /*
     we make sure here that each Fiber is processed only once.
     Cutting a Fiber effectively creates a new Fiber,
     but these are added at the end of the list, 
     after 'end' that is stored before starting
     */
    Fiber * obj, * nxt = first();
    
    if ( nxt )
    {
        Fiber *const end = last();
        if ( func )
        {
            do {
                obj = nxt;
                nxt = nxt->next();
                if ( func(obj, arg) )
                    obj->cutAlongPlane(this, n, a);
            } while ( obj != end );
        }
        else
        {
            do {
                obj = nxt;
                nxt = nxt->next();
                obj->cutAlongPlane(this, n, a);
            } while ( obj != end );
        }
    }
}


void FiberSet::foldPosition(const Modulo * s) const
{
    for ( Fiber * o=first(); o; o=o->next() )
        o->foldPosition(s);
}



/**
 Set a list of Locations on the fibers, chosen randomly with uniform sampling.
 The number of site over a stretch of length `len` should be `len/spread` .
 
 Condition: ( spread > 0 )
 */
void FiberSet::uniFiberSites(Array<FiberBinder>& res, const real spread) const
{
    assert_true( spread > 0 );
    
    res.clear();
    Fiber * fib = first();
    real abs = spread * RNG.exponential();
    while ( fib )
    {
        while ( abs < fib->length() )
        {
            res.push_back(FiberBinder(fib, abs+fib->abscissaM()));
            abs += spread * RNG.exponential();
        }
        abs -= fib->length();
        fib = fib->next();
    }
}


//------------------------------------------------------------------------------
#pragma mark -

int FiberSet::count(bool (*func)(Fiber const*, void* arg), void* arg) const
{
    int cnt = 0;
    for ( Fiber * fib=first(); fib; fib=fib->next() )
        if ( func==0  ||  func(fib, arg) )
            ++cnt;
    return cnt;
}


real FiberSet::totalLength() const
{
    real res = 0;
    
    for ( Fiber const* fib=first(); fib; fib=fib->next() )
        res += fib->length();
    
    return res;
}


real FiberSet::totalLength(FiberProp const* p) const
{
    real res = 0;
    
    for ( Fiber const* fib=first(); fib; fib=fib->next() )
        if ( fib->prop == p )
            res += fib->length();
    
    return res;
}


void FiberSet::infoLength(unsigned int& cnt, real& avg, real& dev,
                          bool (*func)(Fiber const*, void*),
                          void* arg) const
{
    cnt = 0;
    avg = 0;
    dev = 0;

    for ( Fiber const* fib=first(); fib; fib=fib->next() )
    {
        if ( func==0  ||  func(fib, arg) ) 
        {
            ++cnt;
            avg += fib->length();
            dev += fib->length() * fib->length();
        }
    }
    
    if ( cnt )
    {
        avg /= cnt;
        real v = dev/cnt - avg*avg;
        // the variance can be numerically negative, which is mathematically impossible
        if ( v > 0 )
            dev = sqrt(v);
        else
            dev = 0;
    }
}


void FiberSet::infoSegments(unsigned& cnt, unsigned& joints, unsigned& kinks,
                            real& mn, real& mx,
                            bool (*func)(Fiber const*, void*),
                            void* arg) const
{
    cnt = 0;
    joints = 0;
    kinks = 0;
    mn = INFINITY;
    mx = 0;
    
    for ( Fiber const* fib=first(); fib; fib=fib->next() )
    {
        real n = fib->segmentation();
        real x = fib->segmentation();
        if ( func==0  ||  func(fib, arg) )
        {
            ++cnt;
            joints += fib->nbPoints() - 2;
            kinks += fib->nbKinks();
            fib->minMaxSegments(n, x);
            if ( n < mn )
                mn = n;
            if ( x > mx )
                mx = x;
        }
    }
}


/**
 Each Fiber segment is weigthed by its length.
 @return G = average center of gravity
 @return D = average direction
 @return N = average nematic direction
 
 To calculate the Nematic direction, we average the square of 
 the imaginary number representing the direction vector.
 
 This works in 2D by producing a nematic tensor, but in 3D this is only correct
 if the average direction is roughly already aligned with the X-axis.
 
 @todo better nematic tensor calculation in 3D.
 */
real FiberSet::infoDirection(Vector& G, Vector& D, Vector& N,
                             bool (*func)(Fiber const*, void*),
                             void* arg) const
{
    real S = 0;
    G.set(0,0,0);
    D.set(0,0,0);
    N.set(0,0,0);
    
    /* To align the Nematic direction, we average the square of 
     the imaginary number representing the direction vector */
    
    for ( Fiber const* fib=first(); fib; fib=fib->next() )
    {
        if ( func==0  ||  func(fib, arg) ) 
        {
            Vector G1 = 0.5 * ( fib->posEnd(PLUS_END) + fib->posEnd(MINUS_END) );
            for ( unsigned int n = 1; n < fib->lastPoint(); ++n )
                G1 += fib->posPoint(n);
            
            Vector N1(0,0,0);
            for ( unsigned int n = 0; n < fib->nbSegments(); ++n )
            {
                Vector m = fib->dirPoint(n);
#if ( DIM == 2 )
                N1 += Vector(m.XX*m.XX-m.YY*m.YY, 2*m.XX*m.YY);
#elif ( DIM == 3 )
                // this works only near the X-axis:
                N1 += Vector(m.XX*m.XX-m.YY*m.YY-m.ZZ*m.ZZ, 2*m.XX*m.YY, 2*m.XX*m.ZZ);
#endif
            }
            const real w = fib->segmentation();
            S += w * fib->nbSegments();
            G += w * G1;
            D += fib->posEnd(PLUS_END) - fib->posEnd(MINUS_END);
            N += w * N1;
        }
    }
    
    if ( S > 0 )
    {
        G /= S;
        D /= S;
        N.normalize();
        // extract the 'square-root' of the nematic direction
        real x = sqrt((N.XX+1)/2);
#if ( DIM == 2 )
        N.set(x, N.YY/(2*x), 0);
#elif ( DIM == 3 )
        N.set(x, N.YY/(2*x), N.ZZ/(2*x));
#endif
    }
    return S;
}


/**
 Counts the number of fiber intersecting the plane defined by <em> n.x + a = 0 </em>
 in two categories, depending on the direction with which they cross the plane:
 - `np` = number of parallel segments ( the scalar product dir.n is strictly positive )
 - `na` = number of anti-parallel segments ( dir.n < 0 )
 .
 */
void FiberSet::infoIntersections(int& np, int& na, Vector const& n, real a) const
{
    np = 0;
    na = 0;
    for ( Fiber * fib=first(); fib; fib=fib->next() )
    {
        for ( unsigned int s = 0; s < fib->nbSegments(); ++s )
        {
            real abs;
            FiberLocus const& seg = fib->segment(s);
            int sec = seg.intersectPlane(n, a, abs);
            if ( sec == 1 )
                ++np;
            else if ( sec == -1 )
                ++na;
        }
    }
}


/**
 Calculate two indices characterizing the organization of the fibers along the axis `n`.
 - `ixa` = average { ( o - i ) }
 - `ixp` = average { ( r - l ) }
 .
 where:
 - `o` = number of fiber pointing outward (away from the mid-plane),
 - `i` = number of fiber pointing inward (toward the mid-plane),
 - `r` = number of fiber pointing right (in direction of \c n),
 - `l` = number of fiber pointing left.
 .
 
 The indices are averaged over planar sections taken every `dm` units of space,
 and the values for each planar section are weighted by the number of fibers.
 The central symmetry plane is defined by n.x+a=0, and the edges correspond to n.x+a=+/-m.
 
 The results characterize broadly the type of fiber organization:
 - `ixa =  1, ixp = 0`:   aster,
 - `ixa = -1, ixp = 0`:   anti-aster,
 - `ixa =  0, ixp = 1`:   parallel overlap,
 - `ixa =  0, ixp = 0`:   anti-parallel overlap (50/50).
 .
 */
void FiberSet::infoSpindle(real& ixa, real& ixp, Vector const& n, real a, real m, real dm) const
{
    ixa = 0;
    ixp = 0;
    int no, ni, nio;
    int sum = 0;
    for ( real p = dm/2 ; p < m ; p += dm )
    {
        // left side
        infoIntersections(ni, no, n, a+p);
        nio = ni + no;
        if ( nio )
        {
            ixa += ( no - ni );
            ixp += ( ni - no );
            sum += nio;
        }
    
        // right side
        infoIntersections(no, ni, n, a-p);
        nio = ni + no;
        if ( nio )
        {
            ixa += ( no - ni );
            ixp += ( no - ni );
            sum += nio;
        }
    }
    if ( sum )
    {
        ixa /= sum;
        ixp /= sum;
    }
}


/**
 Sum tension of all the segments that intersect the plane
 defined by <em> n.x + a = 0 </em>
 The tension dipole along the segment is obtained from the Lagrange
 multiplier associated with the length of each segment.
 The magnitude of the dipole is multiplied by the cosine of the angle
 between the segment and the plane normal.
 
 @return cnt = number of segments intersecting the plane
 @return ten = sum of tension
 */
void FiberSet::infoTension(unsigned int& cnt, real& ten, Vector const& n, real a) const
{
    cnt = 0;
    ten = 0;
    
    for ( Fiber const* fib=first(); fib; fib=fib->next() )
    {
        for ( unsigned s = 0; s < fib->nbSegments(); ++s )
        {
            real abs;
            FiberLocus const& seg = fib->segment(s);
            if ( seg.intersectPlane(n, a, abs) )
            {
                ten += fabs( n * fib->dirPoint(s) ) * fib->tension(s);
                ++cnt;
            }
        }
    }
}


/**
 Sum tension of all the segments
 
 @return cnt = number of segments intersecting the plane
 @return ten = sum of tension
 */
void FiberSet::infoTension(unsigned int& cnt, real& ten) const
{
    cnt = 0;
    ten = 0;
    
    for ( Fiber const* fib=first(); fib; fib=fib->next() )
    {
        for ( unsigned s = 0; s < fib->nbSegments(); ++s )
        {
            ten += fib->tension(s);
            ++cnt;
        }
    }
}


void FiberSet::infoRadius(unsigned int & cnt, real& rad) const
{
    real r = 0;
    cnt = 0;
    
    for ( Fiber const* f=first(); f; f=f->next() )
    {
        for ( unsigned int p = 0; p < f->nbPoints() ; ++p )
        {
            r += f->posPoint(p).norm();
            ++cnt;
        }
    }
    if ( cnt )
        rad = r / cnt;
}


void FiberSet::infoRadius(unsigned int & cnt, real& rad, FiberEnd end) const
{
    real r = 0;
    cnt = 0;
    
    for ( Fiber const* f=first(); f; f=f->next() )
    {
        r += f->posEnd(end).norm();
        ++cnt;
    }
    if ( cnt )
        rad = r / cnt;
}



