// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "display.h"
#include "hand_prop.h"
#include "sphere_prop.h"
#include "fiber_prop.h"
#include "point_disp.h"
#include "fiber_disp.h"
#include "line_disp.h"
#include "opengl.h"
#include "modulo.h"
#include "simul.h"
#include "gle.h"
#include "gle_color.h"
#include "gle_color_list.h"
#include "glapp.h"
using namespace gle;


extern Modulo * modulo;



Display::Display(DisplayProp const* dp)
: prop(dp), mPixelSize(1), uFactor(1), sFactor(1)
{
    assert_true(dp);
}

void Display::setPixelFactors(real ps, real u)
{
    mPixelSize = ps;
    uFactor    = u;
    //the 0.5 comes from the fact that glPointSize uses diameter instead of radius
    sFactor    = 0.5 * u * mPixelSize;
}


//------------------------------------------------------------------------------
#pragma mark -

void Display::display(Simul const& sim)
{
    displayBSpaces(sim.spaces);
    displayFields(sim.fields);
    
    if ( prop->couple_select & 1 )
        displayFCouples(sim.couples);
    
    if ( prop->single_select & 1 )
        displayFSingles(sim.singles);
    
    displayFibers(sim.fibers);
    displaySolids(sim.solids);
    displayBeads(sim.beads);
    displaySpheres(sim.spheres);
    displayOrganizers(sim.organizers);
    
    if ( prop->single_select & 2 )
        displayASingles(sim.singles);
    
    if ( prop->couple_select & 2 )
        displayACouples(sim.couples);
    
    if ( prop->couple_select & 4 )
        displayBCouples(sim.couples);
    
    displayMisc(sim);
    displayTransparentObjects(sim);
    displayFSpaces(sim.spaces);
}

//------------------------------------------------------------------------------
void Display::displayTiled(Simul const& sim, int nine)
{
    display(sim);
    
    if ( modulo )
    {    
        int l[3] = { 0 };
        int u[3] = { 0 };
        
        for ( int d = 0; d < DIM; ++d )
        {
            if ( modulo->isPeriodic(d) )
            {
                l[d] = (nine>1) ? -1 : 0;
                u[d] = +1;
            }
        }
        
        glMatrixMode(GL_MODELVIEW);
        
        for ( int dx = l[0]; dx <= u[0]; ++dx )
            for ( int dy = l[1]; dy <= u[1]; ++dy )
                for ( int dz = l[2]; dz <= u[2]; ++dz )
                    if ( dx | dy | dz )
                    {
                        Vector T = dx * modulo->period(0) + dy * modulo->period(1) + dz * modulo->period(2);
                        gleTranslate( T);
                        display(sim);
                        gleTranslate(-T);
                    }
    }
}

//------------------------------------------------------------------------------                         
#pragma mark -


/**
 Create a PointDisp for this Property if necessary
 */
void Display::prepareFiberDisp(FiberProp * p, PropertyList& alldisp, gle_color col, char stamp)
{
    assert_true(p);
    
    FiberDisp *& disp = p->disp;
    if ( disp == 0 )
    {
        disp = new FiberDisp(p->name());
        alldisp.push_back(disp);
        // set default:
        disp->color      = col;
        disp->point_size = prop->point_size;
        disp->line_width = prop->line_width;
    }
    
    try {
        disp->readString(p->display, stamp);
    } catch(Exception & e) {
        std::cerr << "fiber:display: " << e.what() << std::endl;
    }
    
    // in 3D, we set visible=-1 for transparent objects
    if ( disp->visible  &&  disp->color.transparent() )
        disp->visible = -1;
}


/**
 set LineDisp for given Fiber
 */
void Display::prepareLineDisp(const Fiber * fib)
{
    assert_true(fib->prop);
    const FiberDisp * disp = fib->prop->disp;
    LineDisp *& self = fib->disp;
    
    if ( self == 0 )
        self = new LineDisp();

    // visibility:
    self->visible = disp->visible;
    
    // hide right or left-pointing fibers:
    if ( ( disp->exclude & 1 )  &&  fib->diffPoints(0)*disp->right < 0 )
        self->visible = 0;
    if ( ( disp->exclude & 2 )  &&  fib->diffPoints(0)*disp->right > 0 )
        self->visible = 0;
    
#if ( DIM == 2 )
    // hide clockwise or counter-clockwise orientated fibers:
    if ( ( disp->exclude & 4 )  &&  vecProd(fib->posPoint(0), fib->diffPoints(0)) < 0 )
        self->visible = 0;
    if ( ( disp->exclude & 8 )  &&  vecProd(fib->posPoint(0), fib->diffPoints(0)) > 0 )
        self->visible = 0;
#endif
    
    if ( disp->mask )
    {
#if ( 0 )
        if ( ( fib->number() - disp->phase ) % disp->mask )
#elif ( 1 )
        if ( ( fib->signature() - disp->phase ) % disp->mask )
#else
        if ( ( fib->mark() - disp->phase ) % disp->mask )
#endif
            self->visible = 0;
    }

    
    // default body color:
    self->color = disp->color;    
    
    // change body color depending on coloring mode:
    switch ( disp->coloring )
    {
        case FiberDisp::COLORING_NUMBER:
            self->color = gle::bright_color(fib->number()+disp->phase);
            break;
        case FiberDisp::COLORING_DIRECTION:
            // use the scalar product with direction defined by FiberDisp:right
            if ( fib->diffPoints(0) * disp->right > 0 )
                self->color = 0x00BB00FF; //green
            else
                self->color = 0xFFFFFFFF; //white
            break;
        case FiberDisp::COLORING_MARK:
            self->color = gle::nice_color(fib->mark());
            break;
        case FiberDisp::COLORING_FLECK:
            self->color = gle::std_color(fib->fleck());
            break;
    }
    
    // by default, tip color is the same as body color:
    self->end_color[0] = self->color;
    self->end_color[1] = self->color;

    
    // For dynamic Fibers, set colors of tips according to state:
    if ( fib->dynamicState(PLUS_END) > 0 )
        self->end_color[0] = disp->end_color[fib->dynamicState(PLUS_END)%5];
    
    if ( fib->dynamicState(MINUS_END) > 0 )
        self->end_color[1] = disp->end_color[fib->dynamicState(MINUS_END)%5];
    
    
    // set disp->explode_shift
    if ( disp->explode )
    {
        if ( disp->explode == 2 )
        {
            // shift is radial
            uint32_t m = lcrng3(fib->mark(), 8);
            real s = ( m * 0x1p-32 ) * disp->explode_range;
            self->explode_shift = fib->posEnd(PLUS_END).normalized(s);
        }
        else
        {
            real s = (fib->signature() * 0x1p-32) * disp->explode_range;
            self->explode_shift.set(0, s, 0);
        }
    }
    else
    {
        self->explode_shift.set(0, 0, 0);
    }
}


/**
 Create a PointDisp for this Property if necessary
 */
template < typename T >
void Display::preparePointDisp(T * p, PropertyList& alldisp, const std::string& kind, gle_color col, char stamp)
{
    assert_true(p);
        
    PointDisp *& disp = p->disp;
    if ( disp == 0 )
    {
        disp = new PointDisp(kind, p->name());
        alldisp.push_back(disp);
        // set default:
        disp->color = col;
        disp->size  = prop->point_size;
        disp->width = prop->line_width;
    }
    
    // parse display string once:
    try {
        disp->readString(p->display, stamp);
    } catch(Exception & e) {
        std::cerr << kind << ": " << e.what() << std::endl;
    }
        
    // in 3D, we set visible=-1 for transparent objects
    if ( DIM==3  &&  disp->visible  &&  disp->color.transparent() )
        disp->visible = -1;
    
    disp->prepare(uFactor);
}

/**
 Perform the operations that are necessary to display the simulation:
 - create FiberDisp, HandDisp, SphereDisp, etc. (one per Property)
 - create LineDisp (one per Fiber)
 - set default values,
 - parse display strings
 .
 
 In 3D, this set disp->visible = -1, for transparent objects.
 */
void Display::prepareForDisplay(Simul const& sim, PropertyList& alldisp)
{
    const char stamp = '%';
    
    // modulo display:
    if ( prop->fold )
        sim.foldPosition();
    
    // counter to give different colors to the objects
    unsigned int idx = 0;
    
    // create a FiberDisp for each FiberProp:
    PropertyList plist = sim.properties.find_all("fiber");
    
    bool analyze_clusters = false;
    for ( unsigned int n = 0; n < plist.size(); ++n, ++idx )
    {
        prepareFiberDisp(static_cast<FiberProp*>(plist[n]), alldisp, gle::nice_color(idx), stamp);
        
        if ( static_cast<FiberProp*>(plist[n])->disp->coloring == FiberDisp::COLORING_FLECK )
            analyze_clusters = true;
    }
    
    if ( analyze_clusters )
        sim.analyzeClusters();
    
    // attribute LineDisp, and set individual display values for all fibers
    for ( Fiber * fib = sim.fibers.first(); fib; fib = fib->next() )
        prepareLineDisp(fib);
    
    //create a PointDisp for each HandProp:
    plist = sim.properties.find_all("hand");
    for ( unsigned int n = 0; n < plist.size(); ++n, ++idx )
        preparePointDisp(static_cast<HandProp*>(plist[n]), alldisp, "hand:display", gle::nice_color(idx), stamp);
    
    //create a PointDisp for each SphereProp:
    plist = sim.properties.find_all("sphere");
    for ( unsigned int n = 0; n < plist.size(); ++n, ++idx )
        preparePointDisp(static_cast<SphereProp*>(plist[n]), alldisp, "sphere:display", gle::bright_color(idx), stamp);
    
    //create a PointDisp for each BeadProp:
    plist = sim.properties.find_all("bead");
    for ( unsigned int n = 0; n < plist.size(); ++n, ++idx )
        preparePointDisp(static_cast<BeadProp*>(plist[n]), alldisp, "bead:display", gle::bright_color(idx), stamp);
    
    //create a PointDisp for each SpaceProp:
    gle_color col(DIM==2?0xAAAAAAFF:0xFFFFFF22);
    plist = sim.properties.find_all("space");
    for ( unsigned int n = 0; n < plist.size(); ++n, ++idx )
        preparePointDisp(static_cast<SpaceProp*>(plist[n]), alldisp, "space:display", col, stamp);
}


/**
 if `coloring` is enabled, this returns the N-th bright color,
 with an alpha value of 1/2 if `visible<0`.
 
 otherwise it returns the object' display color
 */
gle_color Display::bodyColor(PointDisp const* disp, Number nb) const
{
    if ( disp->coloring )
    {
        if ( disp->visible < 0 )
            return bright_color(nb).set_alpha(128);
        else
            return bright_color(nb);
    }
    else
        return disp->color;
}

//------------------------------------------------------------------------------
#pragma mark -


/**
 Display the MINUS_END of a Fiber, according to \a style:
 - 1: draw a sphere
 - 2: draw a cone
 - 3: draw a flat cylinder
 - 4: draw an arrow-head
 - 5: arrow-head in reverse direction
 .
 */
void Display::displayMinusEnd(const int style, Fiber const& fib, real width) const
{
    switch(style)
    {
        case 1:
            gleObject(fib.posPoint(0), width, gleSphere2B);
            break;
        case 2:
            gleObject(fib.posPoint(0), -fib.dirPoint(0), width, gleCone1B);
            break;
        case 3:
            gleObject(fib.posPoint(0), -fib.dirPoint(0), width, gleCylinderHB);
            break;
        case 4:
            gleObject(fib.posPoint(0),  fib.dirPoint(0), width, gleArrowTail2B);
            break;
        case 5:
            gleObject(fib.posPoint(0), -fib.dirPoint(0), width, gleArrowTail2B);
            break;
    }
}

/**
 Display the PLUS_END of a Fiber, according to \a style:
 - 1: draw a sphere
 - 2: draw a cone
 - 3: draw a flat cylinder
 - 4: draw an arrow-head
 - 5: arrow-head in reverse direction
 .
 */
void Display::displayPlusEnd(const int style, Fiber const& fib, real width) const
{
    switch(style)
    {
        case 1:
            gleObject(fib.posEnd(PLUS_END), width, gleSphere2B);
            break;
        case 2:
            gleObject(fib.posEnd(PLUS_END), fib.dirEnd(PLUS_END), width, gleCone1B);
            break;
        case 3:
            gleObject(fib.posEnd(PLUS_END), fib.dirEnd(PLUS_END), width, gleCylinderHB);
            break;
        case 4:
            gleObject(fib.posEnd(PLUS_END), fib.dirEnd(PLUS_END), width, gleArrowTail2B);
            break;
        case 5:
            gleObject(fib.posEnd(PLUS_END), -fib.dirEnd(PLUS_END), width, gleArrowTail2B);
            break;
    }
}


//------------------------------------------------------------------------------
#pragma mark -

void Display::displayAverageFiber(FiberSet const& fibers,
                                  bool (*func)(Fiber const*, void*),
                                  void* arg)
{
    Vector g, d, nm;
    fibers.infoDirection(g, d, nm, func, arg);
    
    real n = 0;
    Vector m(0,0,0), p(0,0,0);
    
    for ( Fiber const* fib=fibers.first(); fib; fib=fib->next() )
    {
        if ( func==0  ||  func(fib, arg) )
        {
            const real weigth = fib->length();
            n += weigth;
            m += weigth * fib->posEnd(MINUS_END);
            p += weigth * fib->posEnd(PLUS_END);
        }
    }
    
    if ( n > REAL_EPSILON )
    {
        p /= n;
        m /= n;
        
        Vector v = ( p - m ).normalized();
        gleCylinder(m, v, 10*mPixelSize);
        gleCone(p, v, 10*mPixelSize);
        gleObject(g, 10*mPixelSize, gleSphere1B);
    }
}


bool selectRL(Fiber const* fib, void* arg)
{
    return ( fib->prop == arg );
}

bool selectR(Fiber const* fib, void* arg)
{
    return ( fib->prop == arg ) && ( fib->diffPoints(0)*fib->prop->disp->right > 0 );
}

bool selectL(Fiber const* fib, void* arg)
{
    return ( fib->prop == arg ) && ( fib->diffPoints(0)*fib->prop->disp->right < 0 );
}

void Display::displayAverageFiber1(FiberSet const& fibers, void* arg)
{
#if ( 1 )
    // highlight with a black outline
    glLineWidth(3);
    glColor3f(0,0,0);
    glDepthMask(GL_FALSE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    displayAverageFiber(fibers, selectRL, arg);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);        
    glDepthMask(GL_TRUE);
#endif
    
    glColor3f(1,1,1);
    displayAverageFiber(fibers, selectRL, arg);
}


void Display::displayAverageFiber2(FiberSet const& fibers, void* arg)
{
#if ( 1 )
    // highlight with a black outline
    glLineWidth(3);
    glColor3f(0,0,0);
    glDepthMask(GL_FALSE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    displayAverageFiber(fibers, selectR, arg);
    displayAverageFiber(fibers, selectL, arg);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);        
    glDepthMask(GL_TRUE);
#endif
    
    // display right-pointing fibers in Red
    glColor3f(1,0,0);
    displayAverageFiber(fibers, selectR, arg);
    
    // display left-pointing fibers in Green
    glColor3f(0,1,0);
    displayAverageFiber(fibers, selectL, arg);
}    


void Display::displayMisc(Simul const& sim)
{
    //sim.stericGrid.display();
    
    PropertyList plist = sim.properties.find_all("fiber");
    
    for ( unsigned int k = 0; k < plist.size(); ++k )
    {
        FiberProp* fp = static_cast<FiberProp*>(plist[k]);
        if ( fp->disp->show_average == 1 )
            displayAverageFiber1(sim.fibers, fp);
        else if ( fp->disp->show_average == 2 )
            displayAverageFiber2(sim.fibers, fp);
    }
}

//------------------------------------------------------------------------------
#pragma mark -

void Display::displayBSpace(Space const& obj)
{
    const PointDisp * disp = obj.prop->disp;
    
    if ( disp->width > 0 )
        glLineWidth(disp->width*uFactor);

    if ( DIM == 2 )
    {
        disp->color.color();
        if ( !obj.display() )
            obj.displaySection( 2, 0, 0.01 );
    }
    else if ( DIM == 3 )
    {
        //disp->color.back();
        prop->inner_color.back();

        if ( disp->visible > 0 )
            glDepthMask(GL_TRUE);
        else
            glDepthMask(GL_FALSE);
        
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        bool drawn = obj.display();
        glDepthMask(GL_TRUE);
        glCullFace(GL_BACK);
        
        if ( ! drawn )
        {
            obj.displaySection( 2, 0, 0.01 );
            obj.displaySection( 0, 0, 0.01 );
            obj.displaySection( 1, 0, 0.01 );
        }
    }
}

void Display::displayFSpace(Space const& obj)
{
    const PointDisp * disp = obj.prop->disp;

    if ( DIM == 3 )
    {
        disp->color.color();

        if ( disp->visible > 0 )
            glDepthMask(GL_TRUE);
        else
            glDepthMask(GL_FALSE);

        glCullFace(GL_BACK);
        obj.display();
        glDepthMask(GL_TRUE);
    }
}

void Display::displayBSpaces(SpaceSet const& set)
{
    for ( Space * obj = set.first(); obj; obj=obj->next() )
        if ( obj->prop->disp->visible & 1 )
            displayBSpace(*obj);
}

/**
 If the color is not transparent, this will hide everything inside.
 */
void Display::displayFSpaces(SpaceSet const& set)
{
    for ( Space * obj = set.first(); obj; obj=obj->next() )
        if ( obj->prop->disp->visible & 2 )
            displayFSpace(*obj);
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 This displays only one Field,
 by default, this displays the Fiber current field
 
 GL_CULL_FACE should be disabled
 */
void Display::displayFields(FieldSet const& set)
{
    Field * f = set.first();
    
    if ( 0 < prop->field_number )
        f = set.find(prop->field_number);

    if ( f && f->hasField() )
        f->display(prop->field_max, false);
}

//------------------------------------------------------------------------------
void Display::displayFibers(FiberSet const& set)
{
    for ( Fiber * obj = set.first(); obj ; obj = obj->next() )
#if ( DIM == 3 )
        if ( obj->prop->disp->visible > 0 && obj->disp->visible  )
#else
        if ( obj->prop->disp->visible && obj->disp->visible )
#endif
        {
            displayFiber(*obj);
            displayFiberMinusEnd(*obj);
            displayFiberPlusEnd(*obj);
        }
}

//------------------------------------------------------------------------------
void Display::displaySolids(SolidSet const& set)
{
    for ( Solid * obj = set.first(); obj; obj = obj->next() )
    {
#if ( DIM == 3 )
        if ( obj->prop->disp->visible > 0 )
#else
        if ( obj->prop->disp->visible )
#endif
        {
            displaySolid(*obj);
            for ( unsigned int ii = 0; ii < obj->nbPoints(); ++ii )
                displayTSolid(*obj, ii);
        }
    }
}

//------------------------------------------------------------------------------
void Display::displayBeads(BeadSet const& set)
{
    for ( Bead * obj = set.first(); obj; obj = obj->next() )
    {
#if ( DIM == 3 )
        if ( obj->prop->disp->visible > 0 )
#else
        if ( obj->prop->disp->visible )
#endif
        {
            displayBead(*obj);
            displayTBead(*obj);
        }
    }
}

//------------------------------------------------------------------------------
void Display::displaySpheres(SphereSet const& set)
{
    for ( Sphere * obj=set.first(); obj ; obj=obj->next() )
    {
#if ( DIM == 3 )
        if ( obj->prop->disp->visible > 0 )
#else
        if ( obj->prop->disp->visible )
#endif
        {
            displaySphere(*obj);
            displayTSphere(*obj);
        }
    }
}        

//------------------------------------------------------------------------------
void Display::displayOrganizers(OrganizerSet const& set)
{
    for ( Organizer * obj=set.first(); obj ; obj=obj->next() )
        displayOrganizer(*obj);
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 @brief A display element with a depth coordinate
 
 zObject is used to depth-sort and display transparent objects
 */
class zObject
{
    GLfloat mPos;
    
    unsigned int mIdx;
    
    const Object * mObj;
    
public:
  
    zObject()
    {
        mObj = 0;
        mPos = 0;
        mIdx = 0;
    }
    
    zObject(Object * o, GLfloat z, unsigned int i = 0)
    {
        mObj = o;
        mPos = z;
        mIdx = i;
    }
    
    /// the Z-coordinate
    inline GLfloat depth() const { return mPos; }
    
    /// display element
    void display(Display * dis) const
    {
        switch( mObj->tag() )
        {
            case Fiber::TAG:
            {
                const Fiber * obj = static_cast<const Fiber*>(mObj);
                dis->displayFiber(*obj);
                dis->displayFiberMinusEnd(*obj);
                dis->displayFiberPlusEnd(*obj);
            } break;
            
            case Solid::TAG:
            {
                const Solid * obj = static_cast<const Solid*>(mObj);
                if ( mIdx == 0 )
                    dis->displaySolid(*obj);
                dis->displayTSolid(*obj, mIdx);
            } break;
            
            case Bead::TAG:
            {
                const Bead * obj = static_cast<const Bead*>(mObj);
                dis->displayBead(*obj);
                dis->displayTBead(*obj);
            } break;
            
            case Sphere::TAG:
            {
                const Sphere * obj = static_cast<const Sphere*>(mObj);
                dis->displaySphere(*obj);
                dis->displayTSphere(*obj);
            } break;
        }
    }
};


/// function for qsort: compares the Z component of two zObjects
int zObject_comp(const void * ap, const void * bp)
{
    const zObject * a = static_cast<const zObject*>(ap);
    const zObject * b = static_cast<const zObject*>(bp);
    
    if ( a->depth() > b->depth() ) return  1;
    if ( a->depth() < b->depth() ) return -1;
    return 0;
}


/**
 This display all objects that have ( disp->visible < 0 )

 Depth-sorting is used in 3D to display objects from the 
 furthest to the nearest.
*/
void Display::displayTransparentObjects(Simul const& sim)
{
#if ( DIM == 3 )
    
    // get current modelview transformation:
    GLfloat mat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);
    
    // extract axis corresponding to vertical direction:
    Vector3 vertical(mat[2], mat[6], mat[10]);
    
    Array<zObject> zObjects;
    
    for ( Fiber * obj=sim.fibers.first(); obj ; obj=obj->next() )
        if ( obj->disp->visible < 0 )
            zObjects.push_back(zObject(obj, obj->position()*vertical));
 
    for ( Bead * obj=sim.beads.first(); obj ; obj=obj->next() )
        if ( obj->prop->disp->visible < 0 )
            zObjects.push_back(zObject(obj, obj->position()*vertical));
    
    for ( Solid * obj=sim.solids.first(); obj ; obj=obj->next() )
        if ( obj->prop->disp->visible < 0 )
            for ( unsigned int ii = 0; ii < obj->nbPoints(); ++ii )
                zObjects.push_back(zObject(obj, obj->posPoint(ii)*vertical, ii));

    for ( Sphere * obj=sim.spheres.first(); obj ; obj=obj->next() )
        if ( obj->prop->disp->visible < 0 )
            zObjects.push_back(zObject(obj, obj->position()*vertical));
    
    // depth-sort objects:
    qsort(zObjects.addr(), zObjects.size(), sizeof(zObject), &zObject_comp);
    
    // display object as transparent:
    glDepthMask(GL_FALSE);

    for ( unsigned int m = 0; m < zObjects.size(); ++m )
    {
        const zObject & obj = zObjects[m];
        obj.display(this);
    }
    
    glDepthMask(GL_TRUE);
    
#endif
}


