// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "sim.h"
#include "simul.h"
#include "meca.h"
#include "exceptions.h"
#include "hand_prop.h"
#include "simul_prop.h"
#include "backtrace.h"
#include "modulo.h"

extern Modulo * modulo;

#include "simul_step.cc"
#include "simul_file.cc"
#include "simul_custom.cc"
#include "simul_report.cc"
#include "simul_solve.cc"

#include "nucleus.h"
#include "aster.h"
#include "bundle.h"
#include "fake.h"
#include "wrist.h"
#include "space_strip.h"
#include "space_periodic.h"
#include "space_cylinderP.h"
#include "fiber.h"

#include <csignal>

//---------------------------  global variables/functions ---------------------

bool functionKey[17] = { 0 };


void out_of_memory_handler()
{
    fprintf(stderr, "* * * * *\n");
    fprintf(stderr, "Cytosim: memory allocation failed\n");
    fprintf(stderr, "* * * * *\n");
    print_backtrace(stderr);
    exit(1);
}

void termination_handler()
{
    fprintf(stderr, "* * * * *\n");
    fprintf(stderr, "Cytosim: uncaught exception\n");
    fprintf(stderr, "* * * * *\n");
    print_backtrace(stderr);
    abort();
}

void fpe_handler(int sig)
{
    fprintf(stderr, "* * * * *\n");
    fprintf(stderr, "Cytosim: floating point exception\n");
    fprintf(stderr, "* * * * *\n");
    print_backtrace(stderr);
    exit(sig);
}

//------------------------------------------------------------------------------
#pragma mark -

Simul::Simul()
: prop(0), spaces(*this), fields(*this),
fibers(*this), spheres(*this), beads(*this), solids(*this),
singles(*this), couples(*this), organizers(*this)
{
    sTime         = 0;
    sReady        = 0;
    sSpace        = 0;
    prop          = new SimulProp("undefined", this);
    prop->index(0);
}

Simul::~Simul()
{
    erase();
    
    if ( prop )
    {
        delete(prop);
        prop = 0;
    }
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 set current Space to \a spc. (zero is a valid argument).
 
 This also set the space of all objects to \a spc.
 */
void Simul::space(Space * spc)
{
    //std::cerr << "Simul::space(" << spc << ")\n";
    sSpace = spc;

    if ( modulo )
    {
        // modulo is a copy of a Space, and deletion is handled elsewhere
        modulo = 0;
    }
    
    if ( fiberGrid.hasGrid() )
        fiberGrid.clear();
    
    if ( stericGrid.hasGrid() )
        stericGrid.clear();
    
    if ( spc )
    {
        if ( spc->prop->shape == "periodic" ) 
            modulo = static_cast<SpacePeriodic*>(spc);
        else if ( spc->prop->shape == "strip" )
            modulo = static_cast<SpaceStrip*>(spc);
        else if ( spc->prop->shape == "cylinderP" )
            modulo = static_cast<SpaceCylinderP*>(spc);
    }
}


Space* Simul::findSpace(std::string const& str) const
{
    if ( str == "first" )
        return static_cast<Space*>(spaces.inventory.first());
    
    if ( str == "last" )
        return static_cast<Space*>(spaces.inventory.last());
    
    Property * sp = properties.find("space", str);
    
    if ( sp )
        return spaces.first(static_cast<SpaceProp*>(sp));
    else
        return 0;
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 This will initialize the simulation by registering callbacks.
 You should still call Simul::prepare() before calling step()
 */
void Simul::initialize(Glossary & glos)
{
    // This is to avoid having a half-initialized state:
    sReady = 0;
    
    // Register a function to be called if operator new fails:
    std::set_new_handler(out_of_memory_handler);
    
    // Register a function to be called upon abortion:
    std::set_terminate(termination_handler);
    
    // Register a function to be called for Floating point exceptions:
    if ( signal(SIGFPE, fpe_handler) == SIG_ERR )
        std::cerr << "Could not register SIGFPE handler\n";
    
    // read parameters, and complete
    prop->read(glos);
    prop->complete(0, 0);
}


/**
 Will pepare the simulation engine to make it ready to make a step():
 - set FiberGrid used for attachment of Hands,
 - set StericGrid
 - call complete() for all registered Property
 .
 The simulated objects should not be changed.
 
 */
void Simul::prepare(Space * spc)
{
    if ( spc == 0 )
        throw InvalidSyntax("A space must be defined first!");
     
    setFiberGrid(spc);
    
    if ( prop->steric )
        setStericGrid(spc);
    
    // this will fix the current Space of some objects:
    prop->complete(prop, &properties);
    
    // this is necessary for diffusion in Field:
    fields.prepare();
    
    couples.prepare(properties);

    sReady = true;
}

//------------------------------------------------------------------------------

void Simul::erase()
{
    sReady    = 0;
    sTime     = 0;
    
    organizers.erase();
    fibers.erase();
    spheres.erase();
    beads.erase();
    solids.erase();
    singles.erase();
    couples.erase();
    fields.erase();
    spaces.erase();
    
    // destroy all properties, except the SimulProp:
    properties.erase();
}



void Simul::foldPosition() const
{
    if ( modulo )
    {
        fibers.foldPosition(modulo);
        beads.foldPosition(modulo);
        solids.foldPosition(modulo);
        spheres.foldPosition(modulo);
        singles.foldPosition(modulo);
        couples.foldPosition(modulo);
        organizers.foldPosition(modulo);
    }
}


//------------------------------------------------------------------------------
#pragma mark -



Object * Simul::findObject(const Tag tag, const long num)
{
    //printf("Find  %c%lu\n", tag, num);
    ObjectSet const* set = findSet(tag);
    if ( set == 0 )
        throw InvalidIO("unknown object tag in Simul::find()");
    return set->findObject(num);
}


Object * Simul::findObject(const std::string& nam, const long num) const
{
    ObjectSet const* set = findSet(nam);
    if ( set == 0 )
        throw InvalidIO("unknown object class `"+nam+"'");
    return set->findObject(num);
}


//------------------------------------------------------------------------------
#pragma mark -

void Simul::add(Object * w)
{
    assert_true(w);
    ObjectSet * set = findSet(w->tag());
    if ( set == 0 )
        throw InvalidIO("invalid object->tag() in Simul::add()");
    set->add(w);
    //std::cerr << " Simul::add(" << w->reference() << ")" << std::endl;
}


int Simul::add(ObjectList const& objs)
{
    int cnt = 0;
    //std::cerr << " Simul::add("<< objs.size() <<" objects):" << std::endl;
    for ( Object ** oi = objs.begin(); oi < objs.end(); ++oi )
    {
        Object * obj = *oi;
        if ( obj )
        {
            add(obj);
            ++cnt;
        }
    }
    return cnt;
}


void Simul::remove(Object * w)
{
    ObjectSet * set = findSet(w->tag());
    if ( set == 0 )
        throw InvalidIO("invalid object->tag() in Simul::remove()");
    set->remove(w);
}


void Simul::remove(ObjectList const& objs)
{
    //std::cerr << " Simul::remove("<< objs.size() <<" objects):" << std::endl;
    for ( Object ** oi = objs.begin(); oi < objs.end(); ++oi )
    {
        Object * obj = *oi;
        if ( obj )
            remove(obj);
    }
}


void Simul::erase(Object * w)
{
    //std::cerr << "Simul::erase " << w->reference() << std::endl;
    remove(w);
    delete(w);
}


void Simul::erase(ObjectList const& objs)
{
    //std::cerr << " Simul::erase("<< objs.size() <<" objects):" << std::endl;
    for ( Object ** oi = objs.begin(); oi < objs.end(); ++oi )
    {
        Object * obj = *oi;
        if ( obj )
        {
            remove(obj);
            delete(obj);
        }
    }
}


void Simul::mark(ObjectList const& objs, int mrk)
{
    //std::cerr << " Simul::erase("<< objs.size() <<" objects):" << std::endl;
    for ( Object ** oi = objs.begin(); oi < objs.end(); ++oi )
        (*oi)->mark(mrk);
}

//------------------------------------------------------------------------------
#pragma mark -

ObjectSet * Simul::findSet(const std::string& kind)
{
    //std::cerr << "findSet("<<kind<<")"<<std::endl;
    if ( kind == "space" )        return &spaces;
    if ( kind == "field" )        return &fields;
    if ( kind == "fiber" )        return &fibers;
    if ( kind == "bead" )         return &beads;
    if ( kind == "solid" )        return &solids;
    if ( kind == "sphere" )       return &spheres;
    if ( kind == "single" )       return &singles;
    if ( kind == "couple" )       return &couples;
    if ( kind == "aster" )        return &organizers;
    if ( kind == "bundle" )       return &organizers;
    if ( kind == "nucleus" )      return &organizers;
    if ( kind == "fake" )         return &organizers;
    return 0;
}



ObjectSet * Simul::findSet(const Tag tag)
{
    switch( tag )
    {
#ifdef BACKWARD_COMPATIBILITY
        case 'm':                  return &fibers;
#endif
        case       Couple::TAG:    return &couples;
        case       Single::TAG:    return &singles;
        case        Wrist::TAG:    return &singles;
        case        Fiber::TAG:    return &fibers;
        case         Bead::TAG:    return &beads;
        case        Solid::TAG:    return &solids;
        case       Sphere::TAG:    return &spheres;
        case      Nucleus::TAG:    return &organizers;
        case       Bundle::TAG:    return &organizers;
        case        Aster::TAG:    return &organizers;
        case         Fake::TAG:    return &organizers;
        case        Field::TAG:    return &fields;
        case        Space::TAG:    return &spaces;
    }
    return 0;
}

//------------------------------------------------------------------------------
#pragma mark -

bool Simul::isProperty(const std::string& kind) const
{
    if ( kind == "simul" )
        return true;
    
    if ( kind == "hand" )
        return true;
    
    return ( 0 != findSet(kind) );
}


Property* Simul::findProperty(const std::string& kd, const std::string& nm) const
{
    if ( kd == "simul" )
        return prop;

    return properties.find(kd, nm);
}


Property* Simul::findProperty(const std::string& kd, const int ix) const
{
    if ( kd == "simul" )
        return prop;
    
    return properties.find(kd, ix);
}


SingleProp* Simul::findSingleProp(const std::string& nm) const
{
    Property * p = properties.find_or_die("single", nm);
    return static_cast<SingleProp*>(p);
}


PropertyList Simul::findProperties(const std::string& kd) const
{
    if ( kd == "simul" )
    {
        PropertyList list;
        list.push_back(prop);
        return list;
    }
    
    return properties.find_all(kd);
}

/**
 @defgroup ObjectGroup List of objects
 
 @ The command `set simul` will define the global paramters.
 The `simul` is automatically created, and you cannot use 'new simul'.

 Objects       | Base Class    | Parameters     
 --------------|---------------|----------------
 `simul`       |  Simul        | @ref SimulPar  
 
 
 Other classes below can be used with `set` and `new`:
 
 Class Name    | Base Class    | Parameters       |  Specialization
 --------------|---------------|------------------|-------------------
 `space`       |  Space        | @ref SpacePar    | @ref SpaceGroup
 `field`       |  FieldBase    | @ref FieldPar    | -                 
 `fiber`       |  Fiber        | @ref FiberPar    | @ref FiberGroup
 `bead`        |  Bead         | @ref BeadPar     | -
 `solid`       |  Solid        | @ref BeadPar     | -
 `sphere`      |  Sphere       | @ref SpherePar   | -

 
 Activities that can bind to fiber, make crosslinks, etc:
 
 Class Name    | Base Class    | Parameters       |  Specialization
 --------------|---------------|------------------|-------------------
 `hand`      @ |  Hand         | @ref HandPar     | @ref HandGroup
 `single`      |  Single       | @ref SinglePar   | @ref SingleGroup
 `couple`      |  Couple       | @ref CouplePar   | @ref CoupleGroup

 @  Class `hand` and derived classes can only be used with command `set`,
 because they only exist as part of `single` or `couple`.
 
 
 Organizers    | Base Class    | Parameters       
 --------------|---------------|------------------
 `aster`       |  Aster        | @ref AsterPar    
 `bundle`      |  Bundle       | @ref BundlePar   
 `nucleus`     |  Nucleus      | @ref NucleusPar  
 `fake`        |  Fake         | @ref FakePar     
 .
 
 */
Property* Simul::newProperty(const std::string& kd, const std::string& nm, Glossary& glos)
{
    if ( kd == "simul" )
    {
        assert_true(prop);
        prop->name(nm);
        return prop;
    }
    
    Property * p = findProperty(kd, nm);
    
    if ( p )
        throw InvalidSyntax("duplicate "+kd+" property `"+nm+"'");
    
    if ( kd == "hand" )
    {
        p = HandProp::newProperty(nm, glos);
        properties.deposit(p);
    }
    else
    {
        ObjectSet * set = findSet(kd);
        
        if ( set == 0 )
            throw InvalidSyntax("unknown object class `"+kd+"'");
        
        p = set->newProperty(kd, nm, glos);
        properties.deposit(p);
    }
    
    return p;
}


