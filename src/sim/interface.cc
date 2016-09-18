// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "interface.h"
#include "glossary.h"
#include "filepath.h"
#include "tictoc.h"
#include <fstream>

extern Random RNG;

#define VERBOSE_INTERFACE 0

//------------------------------------------------------------------------------

Interface::Interface(Simul& s)
: simul(s)
{
}

//------------------------------------------------------------------------------
#pragma mark -


/**
 Property::complete() is called after a property is set.
 This ensures that inconsistencies are detected as early as possible.
 
 The drawback is that we cannot support cross-dependencies (A needs B and vice-versa).
 If that is necessary, we could:
 - call complete() for all Properties, after the parsing process is complete.
 - remove any check for the existence of invoked properties, in which case 
 error would be detected only when objects are created later.
 */
Property* Interface::execute_set(std::string const& kind, std::string const& name, Glossary& def)
{
#if ( VERBOSE_INTERFACE > 0 )
    std::cerr << "-SET " << kind << " `" << name << "'" << std::endl;
#endif
    
    Property* p = simul.newProperty(kind, name, def);
    
    if ( p == 0 )
        throw InvalidSyntax("failed to create property of class `"+kind+"'");
    
    p->read(def);
    p->complete(simul.prop, &simul.properties);
    
    return p;
}

//------------------------------------------------------------------------------
void Interface::execute_change(Property * p, Glossary& def)
{
    p->read(def);
    p->complete(simul.prop, &simul.properties);
    
    if ( p->kind() == "space" )
    {
        /*
         Update the dimensions of any Space using this property.
         This is necessary to make 'change space:dimension' work.
         */
        for ( Space * spc = simul.spaces.first(); spc; spc=spc->next() )
        {
            SpaceProp * sp = static_cast<SpaceProp*>(p);
            if ( spc->prop == p )
                spc->readLengths(sp->dimensions);
        }
    }
}


Property* Interface::execute_change(std::string const& kind, std::string const& name, Glossary& def)
{
#if ( VERBOSE_INTERFACE > 0 )
    std::cerr << "-CHANGE " << kind << " `" << name << "'" << std::endl;
#endif
    
    if ( name == "*" )
    {
        PropertyList list = simul.findProperties(kind);
        
        if ( list.size() == 0 )
            throw InvalidSyntax("there is no "+kind);

        for ( PropertyList::iterator n = list.begin(); n != list.end(); ++n )
            execute_change(*n, def);
            
        return 0;
    }
    else
    {
        Property* p = simul.findProperty(kind, name);
    
        if ( p == 0 )
            throw InvalidSyntax("unknown "+kind+" `"+name+"'");

        execute_change(p, def);
        
        return p;
    }
}

/**
 This may change the 'display' field in the Properties corresponding to `kind` and `name`.
 The `display` value should be specified in `def`.
 */
void Interface::change_display(std::string const& kind, std::string const& name, Glossary& def)
{
    Glossary opt = def.extract("display");
    
    if ( name == "*" )
    {
        PropertyList list = simul.findProperties(kind);
        for ( PropertyList::iterator n = list.begin(); n != list.end(); ++n )
            (*n)->read(opt);
    }
    else
    {
        Property * p = simul.findProperty(kind, name);
        if ( p )
            p->read(opt);
    }
}


//------------------------------------------------------------------------------
#pragma mark -


/**
 Creates one object of class 'kind' and type 'name'.
 */
ObjectList Interface::execute_new(std::string const& kind, std::string const& name, Glossary& opt)
{
    if ( kind == "simul" )
        throw InvalidSyntax("you cannot call `new "+kind+"'");
        
    ObjectSet * set = simul.findSet(kind);

    if ( set == 0 )
        throw InvalidSyntax("unknown class `"+kind+"'");
        
    ObjectList res = set->newPlacedObjects(kind, name, opt);
    
    if ( res.size() == 0 )
        return res;
    
    // optionally mark the objects:
    int mk = 0;
    if ( opt.set(mk, "mark") )
    {
        for ( Object ** oi = res.begin(); oi < res.end(); ++oi )
            (*oi)->mark(mk);
    }
    
    /* 
     Because the objects in ObjectList are not necessarily all of the same class,
     we call simul.add() rather than directly set->add()
     */
    simul.add(res);
    
    //hold();

#if ( VERBOSE_INTERFACE > 2 )
    std::cerr << " Simul::add("<<res.size()<<" "<<kind<<" "<<name<<")"<<std::endl;
#endif
    
    return res;
}


//------------------------------------------------------------------------------
/**
 Creates \a cnt objects of class 'kind' and type 'name'.
 The objects are placed at random position in a random orientation within the current Space.
 
 This is meant to be faster than calling execute_new(set, kind, name, opt) 
 \a cnt times.
 */
int Interface::execute_new(std::string const& kind, std::string const& name, unsigned int cnt)
{
    if ( kind == "simul" )
        throw InvalidSyntax("you cannot call `new "+kind+"'");

    ObjectSet * set = simul.findSet(kind);
    
    if ( set == 0 )
        throw InvalidSyntax("unknown class `"+kind+"'");

    Glossary opt;
    unsigned int created = 0;

    for ( unsigned int n = 0; n < cnt; ++n )
    {
        ObjectList objs = set->newObjects(kind, name, opt);
        
        if ( simul.space() )
        {
            ObjectSet::rotateObjects(objs, Rotation::randomRotation(RNG));
            ObjectSet::translateObjects(objs, simul.space()->randomPlace());
        }
    
        /* 
         Because the objects in ObjectList are not necessarily all of the same class,
         we call simul.add() rather than directly set->add()
         */
#if ( VERBOSE_INTERFACE > 4 )
        std::cerr << " Simul::add("<<objs.size()<<" "<<kind<<" "<<name<<")"<<std::endl;
#endif
        created += simul.add(objs);
    }
    
    //hold();

    return created;
}

//------------------------------------------------------------------------------
#pragma mark -

/// holds a set of criteria to select Objects
class Criteria
{
public:

    int     mrk;
    void  * prp;
    Space * ins;
    Space * ous;

    /// initialize
    Criteria()
    {
        mrk = -1;
        prp = 0;
        ins = 0;
        ous = 0;
    }
    
    void set(Simul& simul, std::string const& kind, std::string const& name, Glossary& opt)
    {
        if ( name != "*" )
        {
            prp = simul.properties.find(kind, name);
            if ( prp == 0 )
                throw InvalidSyntax("unknown object `"+name+"' of class `"+kind+"'");
        }
        
        std::string str;
        if ( opt.set(str, "position") )
        {
            Space * spc = 0;
            std::string spn;
            if ( opt.set(spn, "position", 1) )
                spc = simul.firstSpace(spn);
            else
                spc = simul.space();
            if ( spc == 0 )
                throw InvalidSyntax("unknown Space `"+spn+"'");
            
            if ( str == "inside" )
                ins = spc;
            else if ( str == "outside" )
                ous = spc;
            else
                throw InvalidSyntax("unknown specification `"+str+"'");
        }
        
        opt.set(mrk, "mark");
    }
    
    /// check criteria for given object
    bool check(Object const * obj)
    {
        bool res = true;
        if ( mrk >= 0 )
            res &= ( obj->mark() == mrk );
        if ( ins )
            res &= ins->inside(obj->position());
        if ( ous )
            res &= ous->outside(obj->position());
        if ( prp )
            res &= ( obj->property() == prp );
        return res;
    }
};


bool select_object(Object const* obj, void* val)
{
    return static_cast<Criteria*>(val)->check(obj);
}


/**
 If \a cnt is specified, this will delete at most 'cnt' objects, chosen randomly
 */
void Interface::execute_delete(std::string const& kind, std::string const& name, Glossary& opt, int cnt)
{
    ObjectSet * set = simul.findSet(kind);
    if ( set == 0 )
        throw InvalidSyntax("unknown object class `"+kind+"' after `delete'");
    
    Criteria cri;
    cri.set(simul, kind, name, opt);

    ObjectList objs = set->collect(select_object, &cri);

    // limit to 'cnt' objects:
    if ( 0 <= cnt  &&  (unsigned)cnt < objs.size() )
    {
        objs.mix(RNG);
        objs.truncate(cnt);
    }
    
    simul.erase(objs);
}



/**
 If \a cnt is specified, this will mark at most 'cnt' objects, chosen randomly
 */
void Interface::execute_mark(std::string const& kind, std::string const& name, Glossary& opt, int cnt)
{
    ObjectSet * set = simul.findSet(kind);
    if ( set == 0 )
        throw InvalidSyntax("unknown object class `"+kind+"' after `mark'");

    int mrk;
    if ( ! opt.set(mrk, "mark") )
        throw InvalidParameter("mark must be specified for command `mark'");
    opt.erase("mark");
    
    Criteria cri;
    cri.set(simul, kind, name, opt);
    
    ObjectList objs = set->collect(select_object, &cri);
    
    // limit to 'cnt' objects:
    if ( 0 <= cnt  &&  (unsigned)cnt < objs.size() )
    {
        objs.mix(RNG);
        objs.truncate(cnt);
    }
    
    simul.mark(objs, mrk);
}



void Interface::execute_cut(std::string const& kind, std::string const& name, Glossary& opt)
{
    if ( kind != "fiber" )
        throw InvalidSyntax("only `cut fiber *' is supported");
    
    Criteria cri;
    cri.set(simul, kind, name, opt);
    
    Vector n(1,0,0);
    real a = 0;
    opt.set(n, "plane");
    opt.set(a, "plane", 1);
    
#if ( VERBOSE_INTERFACE > 0 )
    std::cerr << "-CUT PLANE (" << n << ").x = " << -a << std::endl;
#endif
    
    simul.fibers.cutAlongPlane(n, a, select_object, &cri);
}

//------------------------------------------------------------------------------
#pragma mark -

void reportCPUtime(int frame, real stime)
{
    static clock_t clock;
    static double cum = 0;
    
    static int hour = -1;
    if ( hour != TicToc::hours_today() )
    {
        char date[26];
        TicToc::date(date, sizeof(date));
        MSG("%s\n", date);
        hour = TicToc::hours_today();
    }
    
    char cpu[64];
    clock = TicToc::processor_time(cpu, sizeof(cpu), clock, cum);
    
    MSG("F%-6i  %7.2fs  -  %s\n", frame, stime, cpu);
    MSG.flush();
}


void Interface::execute_run(Glossary& opt, unsigned nb_steps, bool do_write)
{
    unsigned int nb_frames  = 0;
    int          solve      = 1;
    bool         prune      = true;
    bool         binary     = true;
    real         event_rate = 0;
    std::string  event_code;
    
    opt.set(nb_frames,  "nb_frames");
    opt.set(event_rate, "event");
    opt.set(event_code, "event", 1);
    opt.set(solve,      "solve", KeyList<int>("none",0, "full", 1, "x_only", 2, "flux", 3));
    opt.set(prune,      "prune");
    opt.set(binary,     "binary");
    
    int           frame = 1;
    real          delta = nb_steps;
    unsigned long stop  = nb_steps;
    
#if ( VERBOSE_INTERFACE > 0 )
    std::cerr << "-RUN START " << nb_steps << std::endl;
#endif

    if ( do_write )
        simul.writeProperties(simul.prop->property_file, prune);
    
    if ( nb_frames > 0 )
    {
        delta = real(nb_steps) / real(nb_frames);
        stop  = (int)delta;
    }
    
    simul.prop->strict = 1;
    simul.prepare();
    
    // Gillespie time at which next event will occur:
    real etime = RNG.exponential();
    // decrement of Gillespie time for one time-step
    real event_rate_dt = event_rate * simul.prop->time_step;
    
    unsigned int n = 0;
    while ( 1 )
    {
        if ( n >= stop )
        {
            if ( do_write  &&  nb_frames > 0 )
            {
                simul.writeObjects(simul.prop->trajectory_file, binary, simul.prop->append_file);
                simul.prop->append_file = true;
                reportCPUtime(frame, simul.simTime());
            }
            if ( n >= nb_steps )
                break;
            stop = (int)( ++frame * delta );
        }
        
        simul.step();
        
        if ( solve )
            simul.solve();

        hold();
        
        etime -= event_rate_dt;
        while ( etime < 0 )
        {
#if ( VERBOSE_INTERFACE > 0 )
            std::cerr << "-EVENT" << std::endl;
#endif
            std::istringstream iss(event_code);
            parse(iss, "while executing event code");
            etime += RNG.exponential();
        }
        
        ++n;
    }
    
    simul.relax();
    
#if ( VERBOSE_INTERFACE > 1 )
    std::cerr << "-RUN COMPLETED"<<std::endl;
#endif
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 Imports a simulation snapshot from a trajectory file
 
 By default, the first frame in the file is read, but
 an argument 'frame=INTEGER' can be given in 'opt'.
 */
void Interface::execute_read(std::string const& file, unsigned frame)
{
#if ( VERBOSE_INTERFACE > 0 )
    std::cerr << "-READ frame " << frame << " from " << file << std::endl;
#endif
    
    InputWrapper in(file.c_str(), "rb");
    if ( ! in.good() )
        throw InvalidIO("Could not open trajectory file `"+file+"'");
    
    unsigned indx = 0;
    while ( in.good() )
    {
        simul.readObjects(in);
        if ( indx >= frame )
            break;
        ++indx;
    }
    
    if ( frame > 0  &&  indx != frame )
        throw InvalidIO("Could not find requested frame in trajectory file");
}


/**
 see Parser::parse_write
 */
void Interface::execute_write(std::string& file, std::string const& what, Glossary& opt)
{
    bool append = true;
    bool binary = true;
    
    opt.set(append, "append");
    opt.set(binary, "binary");

#if ( VERBOSE_INTERFACE > 0 )
    std::cerr << "-WRITE " << what << " to " << file << std::endl;
#endif
    
    // write `state` uses a C-File instead of a C++ stream
    if ( what == "state" )
    {
        if ( file == "*" )
            file = simul.prop->trajectory_file;

        simul.writeObjects(file, binary, append);
        return;
    }
    
    if ( what == "dump" )
    {
        simul.dump();
        return;
    }
    
    // other cases are handled with simul.report()
    if ( file == "*" )
    {
        std::cout << "% time " << simul.simTime() << std::endl;
        simul.report(std::cout, what, opt);
    }
    else
    {
        std::ofstream out(file.c_str(), append ? std::ios_base::app : std::ios_base::out);
        out << "% time " << simul.simTime() << std::endl;
        simul.report(out, what, opt);
        out.close();
    }
}

