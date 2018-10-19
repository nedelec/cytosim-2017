// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "messages.h"
#include "interface.h"
#include "iowrapper.h"
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

/// holds a set of criteria used to select Objects
class SelectionCriteria
{
public:
    
    int          mrk;
    int          st;
    int          st1;
    int          st2;
    void  const* prp;
    Space const* ins;
    Space const* ous;
    
    /// initialize
    SelectionCriteria()
    {
        mrk = 0;
        st  = -1;
        st1 = -1;
        st2 = -1;
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
            Space const* spc = 0;
            std::string spn;
            if ( opt.set(spn, "position", 1) )
                spc = simul.findSpace(spn);
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
        opt.set(st1, "state")    || opt.set(st1, "state1") || opt.set(st1, "stateP");
        opt.set(st2, "state", 1) || opt.set(st2, "state2") || opt.set(st2, "stateM");
    }
    
    /// return `true` if given object fulfull all the criteria
    bool check(Object const * obj)
    {
        if ( mrk > 0 && obj->mark() != mrk )
            return false;
        if ( ins && ins->outside(obj->position()) )
            return false;
        if ( ous && ous->inside(obj->position()) )
            return false;
        if ( prp && obj->property() != prp )
            return false;
        if ( st1 >= 0 )
        {
            if ( obj->tag()==Single::TAG && static_cast<Single const*>(obj)->attached() != st1 )
                return false;
            if ( obj->tag()==Couple::TAG && static_cast<Couple const*>(obj)->attached1() != st1 )
                return false;
            if ( obj->tag()==Fiber::TAG && static_cast<Fiber const*>(obj)->dynamicState(PLUS_END) != st1 )
                return false;
        }
        if ( st2 >= 0 )
        {
            if ( obj->tag()==Single::TAG )
                throw InvalidParameter("to select Single, 'state[1]' is irrelevant");
            if ( obj->tag()==Couple::TAG && static_cast<Couple const*>(obj)->attached2() != st2 )
                return false;
            if ( obj->tag()==Fiber::TAG && static_cast<Fiber const*>(obj)->dynamicState(MINUS_END) != st2 )
                return false;
        }
        return true;
    }
};


bool select_object(Object const* obj, void* val)
{
    return static_cast<SelectionCriteria*>(val)->check(obj);
}


/**
 If \a cnt is specified, this will delete at most 'cnt' objects, chosen randomly
 */
void Interface::execute_delete(std::string const& kind, std::string const& name, Glossary& opt, int cnt)
{
    ObjectSet * set = simul.findSet(kind);
    if ( set == 0 )
        throw InvalidSyntax("unknown object class `"+kind+"' after `delete'");
    
    SelectionCriteria cri;
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
    
    SelectionCriteria cri;
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
    
    SelectionCriteria cri;
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
    static int hour = -1;
    int h = TicToc::hours_today();
    if ( hour != h )
    {
        hour = h;
        char date[26];
        TicToc::date(date, sizeof(date));
        Cytosim::MSG("%s\n", date);
    }
    
    static double clk = 0;
    double cpu = double(clock()) / CLOCKS_PER_SEC;
    Cytosim::MSG("F%-6i  %7.2fs   CPU %10.3fs  %10.0fs\n", frame, stime, cpu-clk, cpu);
    clk = cpu;

    Cytosim::flush();
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
    opt.set(solve,      "solve", KeyList<int>("off", 0, "on", 1, "horizontal", 2, "flux", 3));
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
                simul.relax();
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
            simul.relax();
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
 Import a simulation snapshot from a trajectory file
 
 The frame to be imported can be specified as an option: `frame=INTEGER`:
 @code
 import sim_objects.cmo { frame = 10 }
 @endcode
 
 By default, this will replace the simulation state by the one loaded from file.
 To add the file objects to the simulation without deleting the current world,
 you should specify `keep=1`:
 
 @code
 import sim_objects.cmo { keep = 1 }
 @endcode
 */
void Interface::execute_import(std::string const& file, Glossary& opt)
{
    InputWrapper in(file.c_str(), "rb");
    if ( ! in.good() )
        throw InvalidIO("Could not open file `"+file+"'");
    
    bool kip = false;
    unsigned cnt = 0, frm = 0;
    
    opt.set(frm, "frame");
    opt.set(kip, "keep");
    
#if ( VERBOSE_INTERFACE > 0 )
    std::clog << "-IMPORT frame " << indx << " from " << file << std::endl;
#endif
    
    while ( in.good() )
    {
        if ( kip )
        {
            real t = simul.simTime();
            simul.loadObjects(in);
            simul.setTime(t);
        }
        else
            simul.reloadObjects(in);
        if ( cnt >= frm )
            break;
        ++cnt;
    }
    
    if ( cnt < frm )
        throw InvalidIO("Could not import requested frame");
    
#if ( 0 )
    //unfinished code to mark imported objects
    int mrk;
    if ( opt.set(mrk, "mark") )
    {
        simul.mark(objs, mrk);
    }
#endif
    
    // set time
    real t;
    if ( opt.set(t, "time") )
        simul.setTime(t);
}


/**
 see Parser::parse_export
 */
void Interface::execute_export(std::string& file, std::string const& what, Glossary& opt)
{
    bool append = true;
    bool binary = true;
    
    opt.set(append, "append");
    opt.set(binary, "binary");
    
#if ( VERBOSE_INTERFACE > 0 )
    std::clog << "-EXPORT " << what << " to " << file << std::endl;
#endif
    
    if ( what == "objects" || what == "all" )
    {
        // a '*' designates the usual file name for output:
        if ( file == "*" )
            file = simul.prop->trajectory_file;
        
        simul.writeObjects(file, binary, append);
    }
    else if ( what == "properties" )
    {
        // a '*' designates the usual file name for output:
        if ( file == "*" )
            file = simul.prop->property_file;
        
        simul.writeProperties(file, false);
    }
    else
        throw InvalidIO("only `objects' or `properties' can be exported");
}


/**
 see Parser::parse_report
 */
void Interface::execute_report(std::string& file, std::string const& what, Glossary& opt)
{
    std::string str;
#if ( VERBOSE_INTERFACE > 0 )
    std::clog << "-REPORT " << what << " to " << file << std::endl;
#endif
    
    // a '*' designates the C-standard output:
    if ( file == "*" )
    {
        simul.report(std::cout, what, opt);
    }
    else
    {
        bool append = true;
        opt.set(append, "append");
        std::ofstream out(file.c_str(), append ? std::ios_base::app : std::ios_base::out);
        simul.report(out, what, opt);
        out.close();
    }
}


