// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "fiber.h"
#include "messages.h"
#include "glossary.h"
#include "iowrapper.h"
#include "fiber_locus.h"
#include "fiber_binder.h"
#include "fiber_prop.h"
#include "simul_prop.h"
#include "simul.h"
#include "sim.h"
#include <algorithm>

#define SEVER_KINKED_FIBERS 0

extern bool functionKey[];

//------------------------------------------------------------------------------
/**
 The rest of the initialization is done in FiberProp::newFiber(), 
 and other newFiber() functions where the initial length is known.
 */
Fiber::Fiber(FiberProp const* p)
: frBinders(0), prop(p), disp(0)
{
    if ( prop )
    {
        segmentation(prop->segmentation);
    }
    
    frGlue = 0;
}

//------------------------------------------------------------------------------
Fiber::~Fiber()
{
    detachBinders();
    
    // if linked in SingleSet, frGlue could be deleted twice
    // when the simulation ends and all objects are deleted. 
    if ( frGlue )
    {
        if ( frGlue->linked() )
            prop->glue_set->remove(frGlue);
        delete(frGlue);
        frGlue = 0;
    }
    
    if ( disp )
    {
        /*
         Note: the destructor will not be called here, which is OK
         if LineDisp is a trivial type that does not allocate resources
         */
        free(disp);
        disp = 0;
    }
    
    prop = 0;
}

#pragma mark -

//------------------------------------------------------------------------------
unsigned int Fiber::allocatePoints(const unsigned int nbp)
{
    unsigned int ms = RigidFiber::allocatePoints(nbp);
    /*
     if RigidFiber::allocatePoints() allocated memory, it will return the 
     size of the new array, and we allocate the same size for other arrays.
     */
    if ( ms )
    {
        //std::cerr << "Fiber::allocatePoints " << ms << std::endl;
        frRods.resize(ms);
        for ( unsigned int ii = 0; ii < ms-1; ++ii )
            frRods[ii] = FiberLocus(this, ii);
    }
    return ms;
}

//------------------------------------------------------------------------------
FiberLocus & Fiber::segment(const unsigned int pos) const
{
    assert_true( pos < nbSegments() );
    assert_true( frRods.size() >= nbSegments() );
    assert_true( frRods[pos].fiber() == this );
    
    return frRods[pos];
}


//------------------------------------------------------------------------------
/**
 The Fiber is cut at point P
 - A new Fiber is created from the section [ P - PLUS_END ],
 - all FiberBinder attached to this section are transferred to the new Fiber,
 - a pointer to the new Fiber is returned, which should be added to the Simul
 .
 @return zero, if `pti` is not an internal point
 */
Fiber* Fiber::severPoint(unsigned int pti)
{
    if ( pti == 0  ||  pti >= lastPoint() )
        return 0;
    
    real abs = abscissaP(pti);

    // create a new Fiber of the same kind:
    Fiber* fib = prop->newFiber();
    assert_true( fib->prop == prop );
    
    // copy the FiberNaked part of the object:
    *(static_cast<FiberNaked*>(fib)) = *this;

    assert_true( fib->abscissaM() == abscissaM() );
    // remove MINUS_END portion on new piece:
    fib->truncateM(pti);
    assert_true(fib->abscissaM() == abs);
    
    // remove PLUS_END portion on self
    truncateP(pti);

    
    // transfer FiberBinders above point P
    // their abscissa should not change in the transfer
    Node * nd = frBinders.first();
    while( nd )
    {
        FiberBinder* ha = static_cast<FiberBinder*>(nd);
        nd = nd->next();
        if ( ha->abscissa() > abs )
            ha->relocate(fib);
        else
            ha->updateBinder();
    }
    
    return fib;
}


//------------------------------------------------------------------------------
/**
The Fiber is cut at distance \a abs from its MINUS_END:
 - current Fiber is truncated to keep only the section [ MINUS_END - abs ]
 - A new Fiber is created from the section [ abs - PLUS_END ],
 - all FiberBinder attached to the section are transfered to the new Fiber,
 - a pointer to the new Fiber is returned, and it should be added to the Simul
 .
 The return value is zero, if `abs` is not within the valid range of abscissa.
 */
Fiber* Fiber::severM(real abs)
{
    if ( abs <= 0 || abs >= length() )
        return 0;
    
    // create a new Fiber of the same kind:
    Fiber* fib = prop->newFiber();
    assert_true( fib->prop == prop );

    // copy the FiberNaked part of the object:
    *(static_cast<FiberNaked*>(fib)) = *this;
    
    assert_small(fib->abscissaM() - abscissaM());
    // remove MINUS_END portion on new piece
    fib->cutM(abs);
    
    assert_small(fib->abscissaM()-abs-abscissaM());
    
    // remove PLUS_END portion on self
    cutP(length()-abs);
    
    assert_small(fib->abscissaM()-abscissaP());

    // transfer all FiberBinders above cut to new piece
    // their abscissa should not change in this transfer
    const real edge = abs + abscissaM();
    Node * nd = frBinders.first();
    while( nd )
    {
        FiberBinder* ha = static_cast<FiberBinder*>(nd);
        nd = nd->next();
        if ( ha->abscissa() >= edge )
            ha->relocate(fib);
        else
            ha->updateBinder();
    }
    return fib;
}


/**
 Performs the delayed severing events.
 This is called at the end of the time-step.
 Multiple cuts can occur. Fiber parts are added to \a set.
 */
void Fiber::delayedSevering()
{
    if ( delayedCuts.empty() )
        return;
    
    // sort cuts in descending order
    std::sort(delayedCuts.rbegin(), delayedCuts.rend());
    
    // cut starting from highest abscissa
    for ( std::vector<real>::iterator s = delayedCuts.begin(); s < delayedCuts.end(); ++s )
    {
        Fiber * fib = severNow(*s);
        
        if ( fib )
        {
            objset()->add(fib);
#ifdef LOGGING
            std::cout<<"severed "<<reference()<<" *s "<<std::setprecision(2)<<abs;
            std::cout<<" creating "<<part->reference();
            std::cout<<" position "<<part->posEnd(MINUS_END)<<std::endl;
#endif
        }
    }
    
    delayedCuts.clear();
}


/**
 This function tests the angle between consecutive segments on the Fiber.
 The Fiber is severed at points where the angle is such that ( cosine < 0 ),
 ie. if the angle 90 degrees or less.
 Multiple cuts can occur. Fiber parts are added to \a set.
 */
void Fiber::severKinks()
{
    ObjectSet * set = objset();
    // we sweep down, because severPoint() removes the distal part
    for ( unsigned int p = lastPoint()-1; p > 1 ; --p )
    {
        if ( diffPoints(p-1) * diffPoints(p) < 0 )
            set->add(severPoint(p));
    }
}

//------------------------------------------------------------------------------

void Fiber::cutAlongPlane(FiberSet * set, Vector const& n, const real a)
{
    Array<real> cuts;
    // check intersection in order of decreasing abscissa
    for ( int s = lastSegment(); s >=0 ; --s )
    {
        real abs;
        FiberLocus const& seg = segment(s);
        if ( seg.intersectPlane(n, a, abs) )
            cuts.push_back(abs);
    }
    // process the cuts in order of decreasing abscissa
    for ( real * s = cuts.begin(); s < cuts.end(); ++s )
    {
        Fiber * fib = severNow(*s);
        if ( fib ) set->add(fib);
    }
}

//------------------------------------------------------------------------------
void Fiber::join(Fiber * fib)
{
    assert_true( fib );
    // make sure that the two fibers are of the same kind:
    assert_true( prop == fib->prop );
    
    // join the points
    FiberNaked::join(fib);
    
    // transfer all FiberBinder
    real shift = abscissaM() - fib->abscissaP();
    Node * nd = fib->frBinders.first();
    while( nd )
    {
        FiberBinder* ha = static_cast<FiberBinder*>(nd);
        nd = nd->next();
        fib->removeBinder(ha);
        ha->relocate(this, ha->abscissa()+shift);
        addBinder(ha);
    }
}


//------------------------------------------------------------------------------
#pragma mark -

/** 
 Fiber::setDragCoefficientVolume() calculates the mobility for the entire fiber, 
 considering that the cylinder is straight and moving in a infinite fluid.
 fiber:hydrodynamic_radius[1] is a hydrodynamic cutoff that makes the
 drag coefficient proportional to length for length longer than the cutoff.
 
 The formula for a cylinder is taken from:\n
 <em>
 Tirado and de la Torre. J. Chem. Phys 71(6) 1979 \n
 http://link.aip.org/link/doi/10.1063/1.438613 \n
 </em>

 We calculate the translational drag coefficient averaged over all possible configurations:
 @code
 cylinder_drag = 3*PI*length*viscosity / ( log(length/diameter) + 0.312 )
 @endcode
 
 If the length is shorter than the diameter, the formula above fails and may even give negative result.
 Hence we also calculate the drag of a sphere with the same radius as the cylinder:
 @code
 sphere_drag = 6*PI*visc*R
 @endcode
 We use the maximum of these two coefficients.
 */
real Fiber::dragCoefficientVolume()
{
    real len = length();
    assert_true( len > 0 );
    
    // hydrodynamic cut-off on length:
    real lenc = len;
    assert_true( prop->hydrodynamic_radius[1] > 0 );
    if ( lenc > prop->hydrodynamic_radius[1] )
        lenc = prop->hydrodynamic_radius[1];
    
    //Stoke's for a sphere:
    assert_true( prop->hydrodynamic_radius[0] > 0 );
    real drag_sphere = 6 * prop->hydrodynamic_radius[0];
    
    const real pref = 3;

#if ( 0 )
    /*
     For an ellipsoid,  
     drag_transverse = 2*drag_parallel = 4*PI*L*visc / log(length/radius)
     We should average the mobility coefficients:  speed = mu * f
     mu_X = mu_parallel   = 2 * mu
     mu_Y = mu_transverse = mu
     mu_Z = mu_transverse = mu
     Hence:
     mu_averaged = ( mu + mu + 2*mu ) / 3 = 4/3 * mu.
     drag_averaged = 3*PI*length*viscosity / log(length/radius)
     See for example "Random Walks in Biology" by HC. Berg, Princeton University Press.
     */
    
    real drag_cylinder = pref * len / log( lenc / prop->hydrodynamic_radius[0] );
#else
    /*
     Tirado and de la Torre. J. Chem. Phys 71(6) 1979
     give the averaged translational friction coefficient for a cylinder:
     3*PI*length*viscosity / ( log( length/diameter ) + 0.312 )
     */
    real drag_cylinder = pref * len / ( log( 0.5 * lenc / prop->hydrodynamic_radius[0] ) + 0.312 );
#endif

    // use largest drag coefficient
    real drag = M_PI * prop->viscosity * ( drag_cylinder > drag_sphere ? drag_cylinder : drag_sphere );

    //Cytosim::MSG("Drag coefficient of Fiber in infinite fluid = %.1e\n", drag);
    //std::clog << "Fiber " << reference() << " has drag " << drag << "\n";

    return drag;
}



/**
 Fiber::setDragCoefficientSurface() uses a formula calculated by F. Gittes in:\n
 <em>
 Hunt et al. Biophysical Journal (1994) v 67 pp 766-781 \n
 http://dx.doi.org/10.1016/S0006-3495(94)80537-5 \n
 </em>
 
 It applies to a cylinder moving parallel to its axis and near an immobile surface:
 @code
 drag-per-unit-length = 2 &pi &eta / acosh(h/r)
 @endcode
 
 With:
 - r = cylinder radius,
 - h = distance between cylinder bottom and surface,
 - &eta = viscosity of the fluid.
 
 If the cylinder is exactly touching the surface, `h=0` and the drag coefficient is infinite.
 
 The drag coefficient for motion perpendicular to the cylinder axis would be twice higher,
 but for gliding assays, the parallel drag coefficient is the appropriate choice.  
 
 Note that this is usually equivalent to the approximate formula:
 @code
 drag-per-unit-length = 2 &pi &eta / log(2*h/r)
 @endcode
 because `acosh(x) = ln[ x + sqrt(x^2-1)) ] ~ ln[2x] if x >> 1`

 Hunt et al. also credit this reference for the formula:\n
 <em>
 The slow motion of a cylinder next to a plane wall.
 Jeffrey, D.J. & Onishi, Y. (1981) Quant. J. Mech. Appl. Math. 34, 129-137.
 </em>
*/
real Fiber::dragCoefficientSurface()
{
    real len = length();    
    
    if ( prop->cylinder_height <= 0 )
        throw InvalidParameter("fiber:surface_effect[1] (height above surface) must set and > 0!");
    
    // use the higher drag: perpendicular to the cylinder (factor 2)
    real drag = 2 * M_PI * prop->viscosity * len / acosh( 1 + prop->cylinder_height/prop->hydrodynamic_radius[0] );
    
    //Cytosim::MSG("Drag coefficient of Fiber near a planar surface = %.1e\n", drag);
    //std::cerr << "Drag coefficient of Fiber near a planar surface = " << drag << std::endl;

    return drag;
}



/**
 Calculate drag coefficient from two possible formulas
@code
 if ( fiber:surface_effect )
    setDragCoefficientSurface();
 else
    setDragCoefficientVolume();
@endcode
 */
void Fiber::setDragCoefficient()
{
    real drag;
    
    if ( prop->surface_effect )
    {
        drag = dragCoefficientSurface();
        if ( 0 )
        {
            real d = dragCoefficientVolume();
            std::cerr << "Drag coefficient of Fiber near a planar surface amplified by " << drag/d << std::endl;
        }
    }
    else
        drag = dragCoefficientVolume();

    //the forces are distributed equally on all points, hence we multiply by nbPoints()
    assert_true( nbPoints() > 0 );
    rfMobility = nbPoints() / drag;
}


void Fiber::prepareMecable()
{
    setDragCoefficient();
    storeDifferences();
    makeProjection();

    assert_true( rfMobility > REAL_EPSILON );

    // the scaling of the bending elasticity depends on the length of the segments
    rfRigidity  = prop->rigidity / segmentationCub();
}

//------------------------------------------------------------------------------

void Fiber::setInteractions(Meca & meca) const
{
    switch ( prop->confine )
    {
        case CONFINE_NOT:
            break;
            
        case CONFINE_INSIDE:
        {
            Space const* spc = prop->confine_space_ptr;
            
            for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
            {
                Vector pos = posPoint(pp);
                if ( spc->outside(pos) )
                    spc->setInteraction(pos, PointExact(this, pp), meca, prop->confine_stiff);
            }
        } break;
            
        case CONFINE_OUTSIDE:
        {
            Space const* spc = prop->confine_space_ptr;
            
            for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
            {
                Vector pos = posPoint(pp);
                if ( spc->inside(pos) )
                    spc->setInteraction(pos, PointExact(this, pp), meca, prop->confine_stiff);
            }
        } break;
            
        case CONFINE_SURFACE:
        {
            Space const* spc = prop->confine_space_ptr;
            
            for ( unsigned int pp = 0; pp < nbPoints(); ++pp )
                spc->setInteraction(posPoint(pp), PointExact(this, pp), meca, prop->confine_stiff);
        } break;
            
        case CONFINE_MINUS_END:
        {
            Space const* spc = prop->confine_space_ptr;
            
            unsigned int pp = 0;
            spc->setInteraction(posPoint(pp), PointExact(this, pp), meca, prop->confine_stiff);
        } break;
            
        case CONFINE_PLUS_END:
        {
            Space const* spc = prop->confine_space_ptr;
            
            unsigned int pp = lastPoint();
            spc->setInteraction(posPoint(pp), PointExact(this, pp), meca, prop->confine_stiff);
        } break;
            
        default:
            throw InvalidParameter("Invalid bead::confine");
    }
}


void Fiber::step()
{
#if SEVER_KINKED_FIBERS
    assert_true(linked());
    // sever fiber at joints that make an angle above 90 degrees:
    severKinks();
#endif
    
    // perform the cuts that were registered by sever()
    delayedSevering();
    
    //add single that act like glue
    if ( prop->glue )
    {
        assert_true(prop->confine_space_ptr);
        setGlue(frGlue, PLUS_END, prop->confine_space_ptr, prop->glue);
    }    
}

//------------------------------------------------------------------------------
#pragma mark -

void Fiber::addBinder(FiberBinder * fb)
{
    frBinders.push_back(fb);
}


void Fiber::removeBinder(FiberBinder * fb)
{
    frBinders.pop(fb);
}


FiberBinder * Fiber::firstBinder() const
{
    return static_cast<FiberBinder*>(frBinders.first());
}


void Fiber::updateBinders()
{
    //we iterate one step forward, because updating might lead to detachment:
    //the loop can be unrolled, or parallelized
    Node * hi = frBinders.first();
    while ( hi )
    {
        FiberBinder * ha = static_cast<FiberBinder*>(hi);
        hi = hi->next();
        ha->updateBinder();
        ha->checkFiberRange();
    }
}


void Fiber::detachBinders()
{
    //we iterate one step forward, because updating might lead to detachment:
    Node * hi = frBinders.first();
    while ( hi )
    {
        FiberBinder * ha = static_cast<FiberBinder*>(hi);
        hi = hi->next();
        ha->detach();
    }
}


int Fiber::nbBinders(unsigned int (*count)(const FiberBinder &)) const
{
    unsigned int result = 0;
    
    Node * hi = frBinders.first();
    while ( hi ) {
        FiberBinder * ha = static_cast<FiberBinder*>(hi);
        hi = hi->next();
        result += count(*ha);
    }
    
    //printf("countBinders(%p) = %i\n", count, result);
    return result;
}


int Fiber::nbBindersInRange(const real aMin, const real aMax, const FiberEnd from) const
{
    int result = 0;
    
    ///\todo: Convert aMin and aMax instead of converting each Binder abscissa
    Node * hi = frBinders.first();
    while ( hi )
    {
        FiberBinder * ha = static_cast<FiberBinder*>(hi);
        hi = hi->next();
        real abs = ha->abscissaFrom(from);
        if ( aMin <= abs  &&  abs <= aMax )
            ++result;
    }
    
    //printf("nbBinderssInRange(%8.2f, %8.2f)=%i\n", aMin, aMax, result);
    return result;
}


int Fiber::nbBindersNearEnd(const real len, const FiberEnd from) const
{
    int result = 0;
    
    Node * hi = frBinders.first();
    while ( hi ) {
        FiberBinder * ha = static_cast<FiberBinder*>(hi);
        hi = hi->next();
        result += ( ha->abscissaFrom(from) < len );
    }
    
    //printf("nbBindersNearEnd(%8.2f)=%i\n", len, result);
    return result;
}


//------------------------------------------------------------------------------
#pragma mark -

///setGlue1 creates a pushing force from the cortex
void Fiber::setGlue1(Single* glue, const FiberEnd which, const Space * spc)
{
    assert_true(spc);
    if ( spc->inside(posEnd(which)) )
    {
        //detach immediately if the tip is inside the box
        if ( glue->attached() )
            glue->detach();
    }
    else
    {
        if ( glue->attached() )
        {
            //always keep tracking the tip:
            glue->hand()->moveToEnd(which);
        }
        else {
            //reposition the grafted base if the MT is freshly outside:
            Vector P;
            spc->project(posEnd(which), P);
            glue->setPosition(P);
            //attach to the MT-tip:
            glue->attachToEnd(this, which);
        }
    }
}

///setGlue2 creates a pulling force from the cortex
void Fiber::setGlue2(Single* glue, const FiberEnd which, const Space * spc)
{    
    assert_true(spc);
    //reposition the grafted base if the MT is freshly outside:
    if ( ! glue->attached()  &&  spc->outside(posEnd(which)) )
    {
        /*
         Attach to the MT position that crosses the edge of the Space:
         */
        //find indx such that P[indx] is inside and P_{indx+1} is outside:
        int indx = lastPoint();
        do {
            --indx;
        } while ( indx >= 0  &&  spc->outside(posPoint(indx)) );
        
        if ( indx >= 0 )
        {
            Vector P1 = posPoint(indx);     //this point is inside
            Vector P2 = posPoint(indx+1);   //this point is outside
            
            //we interpolate using the distances of P1 and P2 to the edge
            real d1 = spc->distanceToEdge(P1);
            real d2 = spc->distanceToEdge(P2);
            if ( d1 + d2 > REAL_EPSILON )
            {
                // we find the intersection, assuming the edge is straight 
                FiberBinder fs(this, abscissaP(indx+d1/(d2+d1)));
                glue->attach(fs);
                glue->setPosition(fs.pos());
            }
        }
    }
}


///setGlue3 creates pulling and pushing force from the cortex
void Fiber::setGlue3(Single* glue, const Space * spc)
{
    assert_true(spc);
    /*
     If the glue is not already attached, we first check if the fiber intersects
     the edge of the Space:
     */
    if ( ! glue->attached() )
    {
        bool in = spc->inside(posEnd(MINUS_END));
        
        if ( in == spc->inside(posEnd(PLUS_END)) )
            return;
        
        // find a model point that is on the other side of the Space edge:
        for ( int pp = 1; pp < nbPoints(); ++pp )
        {
            if ( spc->inside(posPoint(pp)) != in )
            {
                // the abscissa is interpolated using the distances of P1 and P2 to the edge
                real d1 = spc->distanceToEdge(posPoint(pp-1));
                real d2 = spc->distanceToEdge(posPoint(pp));
                if ( d1 + d2 > REAL_EPSILON )
                {
                    /* we find the abscissa corresponding to the intersection,
                     assuming that the edge is locally straight */
                    FiberBinder fs(this, abscissaP(pp-1+d1/(d2+d1)));
                    glue->attach(fs);
                    glue->setPosition(fs.pos());
                    break;
                }
            }
        }
    }
}


void Fiber::setGlue(Single*& glue, const FiberEnd which, const Space * space, int glue_type)
{
    if ( glue == 0 )
    {
        /*
         search for a glue in the list of bound HandSingle
         this is useful when a simulation is restarted from file
         */
        for ( Single * gh = prop->glue_set->firstA(); gh; gh=gh->next() )
        {
            if ( gh->hand()->fiber() == this )
            {
                glue = gh;
                //std::cerr << "found Fiber:glue for " << reference() << "\n";
                break;
            }
        }
        
        // create the Single if needed
        if ( glue == 0 )
            glue = prop->glue_prop->newSingle();
    }
    
    // creates Single when MT interact with the cortex:
    switch( glue_type )
    {
        case 1:  setGlue1(glue, which, space);  break;
        case 2:  setGlue2(glue, which, space);  break;
        case 3:  setGlue3(glue, space);  break;
        default: throw InvalidParameter("invalid value of fiber:glue");
    }
    
#if ( 1 )
    // we keep the Single linked only if it is attached:
    if ( glue->attached() )
    {
        if ( !glue->linked() )
            prop->glue_set->add(glue);
    }
    else
    {
        if ( glue->linked() )
            prop->glue_set->remove(glue);
    }
#endif
}

//------------------------------------------------------------------------------
#pragma mark -

real Fiber::projectPoint(Vector const& w, real & dist) const
{
    // initialize with the minus-end:
    dist = w.distanceSqr(posPoint(0));
    real abs = 0, len = segmentation();
    
    // try all segments
    for ( unsigned int ii = 0; ii < nbSegments(); ++ii )
    {
        //check the segment:
        FiberLocus s(this, ii);
        real a, d;
        s.projectPoint0(w, a, d);
        if ( len < a )
        {
            // test exact point
            real e = w.distanceSqr(posPoint(ii+1));
            if ( e < dist ) {
                abs  = abscissaP(ii+1);
                dist = e;
            }
        }
        else if ( 0 <= a  &&  d < dist )
        {
            //the projection is the best found so far
            abs  = abscissaP(ii) + a;
            dist = d;
        }
    }

    return abs;
}

//------------------------------------------------------------------------------
#pragma mark -

void Fiber::write(OutputWrapper& out) const
{
    FiberNaked::write(out);
}


void Fiber::read(InputWrapper & in, Simul& sim)
{
    try {
#ifdef BACKWARD_COMPATIBILITY
        
        if ( in.formatID() == 33 )
            mark(in.readUInt32());

        if ( tag()=='m'  &&  in.formatID()==31 )
        {
            unsigned int p = in.readUInt16();
            prop = static_cast<FiberProp*>(sim.properties.find("fiber",p));
        }

        if ( in.formatID() < 31 )
        {
            setDynamicState(MINUS_END, in.readUInt8());
            setDynamicState(PLUS_END, in.readUInt8());
        }
#endif
        
        FiberNaked::read(in, sim);
        
    }
    catch( Exception & e ) {
        //std::cerr << "prop="<<prop<<"\n";
        e << ", while importing " << reference();
        throw;
    }
}

