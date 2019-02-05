// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "sim.h"
#include "parser.h"
#include "glossary.h"
#include "tokenizer.h"
#include "stream_func.h"
#include "messages.h"
#include <fstream>


#define VERBOSE_PARSER 0

//------------------------------------------------------------------------------
/**
 The permission of the parser are:
 - allow_change: existing Property or Object can be modified
 - allow_set: new Properties can be created
 - allow_new: new Object can be created
 - allow_write: can write to disc
 .
 */
Parser::Parser(Simul& s, bool ds, bool dc, bool dn, bool dr, bool dw)
: Interface(s), do_set(ds), do_change(dc), do_new(dn), do_run(dr), do_write(dw)
{
    spos = 0;
}

//------------------------------------------------------------------------------
#pragma mark -


/**
 Read class and optional field

 Syntax:
 @code
 CLASS
 @endcode

 also accepts:
 @code
 CLASS:FIELD
 @endcode

 The return value is true if the second syntax is found.
 */

bool Parser::read_class_name(std::istream & is, std::string & kd, std::string & fd) const
{
    bool res = false;
    kd = Tokenizer::get_identifier(is);

    if ( ! simul.isProperty(kd) )
        throw InvalidSyntax("unknown class `"+kd+"'");

    if ( ':' == is.peek() )
    {
        is.get();
        fd = Tokenizer::get_identifier(is);

        if ( fd.empty() )
            throw InvalidSyntax("missing field in syntax `set class:field name'");
        if ( !isalpha(fd[0]) )
            throw InvalidSyntax("invalid field in syntax `set class:field name'");

        res = true;
    }
    return res;
}


/**
 Read Property name preceded by an optional property-index

 Syntax:
 @code
 [INDEX] NAME
 @endcode

 also accepts:
 @code
 [INDEX] *
 @endcode

 */

void Parser::read_property_name(std::istream & is, std::string& nm, const std::string& kd) const
{
    int ix = -1;
    nm = Tokenizer::get_token(is);

    if ( nm.empty() )
        throw InvalidSyntax("missing property name");

    if ( isdigit(nm[0]) )
    {
        std::istringstream iss(nm);
        iss >> ix;

        //check for leftover:
        iss.get();
        if ( iss.gcount() > 0 )
            throw InvalidSyntax("leftover characters in index specification `"+nm+"'");

        nm = Tokenizer::get_token(is);
    }

    // we accept '*' instead of the name
    if ( nm == "*" )
    {
        // try to substitute the correct name
        Property * p = simul.findProperty(kd, ix);

        if ( p )
            nm = p->name();
    }
    else
    {
        if ( !isalpha(nm[0]) )
            throw InvalidSyntax("missing or invalid property name");

        Property * p = simul.findProperty(kd, nm);

        if ( p  &&  ix > 0  &&  p->index() != ix )
        {
            std::cerr << "Property " << kd << "`" << nm << "' should have index " << p->index() << std::endl;
            throw InvalidSyntax("Could not find property index");
        }
    }
}


//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Parse
//------------------------------------------------------------------------------

/**
 Create a new Property (a set of parameters associated with a class).

 @code
 set CLASS NAME
 {
   PARAMETER =  VALUE
   ...
 }
 @endcode

 Short syntax:
 @code
 set CLASS:PARAMETER NAME VALUE
 @endcode

 CLASS should be one of the predefined object (see @ref ObjectGroup).\n
 NAME can be any string, but it should start with a letter.\n
 The command will define the given NAME for this class.
 */

void Parser::parse_set(std::istream & is)
{
    std::string kind, field, name;
    bool has_field = read_class_name(is, kind, field);
    read_property_name(is, name, kind);

    std::string blok = Tokenizer::get_token(is, true);

    if ( blok.empty() )
        throw InvalidSyntax("missing/empty value block");

    Glossary opt;

    if ( has_field )
    {
        if ( blok[0] == '{' || field == "display" )
            opt.set_values(field, blok);
        else
            opt.set_values(field, Tokenizer::strip_block(blok));
    }
    else
        opt.read(Tokenizer::strip_block(blok));

    Property * p = 0;
    if ( has_field )
    {
        // if the field is specified, 'set' behaves like 'change'
        // we always allow 'display' to be modified
        if ( do_change )
            p = execute_change(kind, name, opt);
        else if ( field == "display" )
            change_display(kind, name, opt);
    }
    else
    {
        if ( do_set )
            p = execute_set(kind, name, opt);
        else if ( do_change )
        {
            p = simul.findProperty(kind, name);
            if ( p ) execute_change(kind, name, opt);
        }
        else if ( opt.has_key("display") )
            change_display(kind, name, opt);
    }

    if ( p && opt.warnings(std::cerr) )
        StreamFunc::show_lines(std::cerr, is, spos, is.tellg());
}

//------------------------------------------------------------------------------
/**
 Change the value of one (or more) parameters for property \a NAME of class \a CLASS.

 @code
 change CLASS NAME
 {
   PARAMETER = VALUE
   ...
 }
 @endcode

 Short syntax:

 @code
 change CLASS:PARAMETER NAME VALUE
 @endcode

 The possible values of CLASS are in the @ref ObjectGroup.\n
 The NAME should have been defined previously in the same class with `set`,
 but it is possible to use a star (*) to change all properties of the class.
 */

void Parser::parse_change(std::istream & is)
{
    std::string field, kind, name;
    bool has_field = read_class_name(is, kind, field);
    read_property_name(is, name, kind);

    std::string blok = Tokenizer::get_token(is, true);

    if ( blok.empty() )
        throw InvalidSyntax("missing/empty value block");

    Glossary opt;

    if ( has_field )
    {
        if ( blok[0] == '{' )
            opt.set_values(field, blok);
        else
            opt.set_values(field, Tokenizer::strip_block(blok));
    }
    else
        opt.read(Tokenizer::strip_block(blok));

    if ( do_change )
    {
        execute_change(kind, name, opt);
        if ( opt.warnings(std::cerr) )
            StreamFunc::show_lines(std::cerr, is, spos, is.tellg());
    }
}

//------------------------------------------------------------------------------
/**
 The command `new` creates one or more objects with given specifications:

 @code
 new [MULTIPLICITY] CLASS NAME
 {
   position         = POSITION
   orientation      = ROTATION
   post_translation = VECTOR
   post_rotation    = ROTATION
   mark             = INTEGER
   required         = INTEGER
 }
 @endcode

 The possible values of CLASS are in the @ref ObjectGroup.\n
 The NAME should have been defined previously in the same class with `set`.\n

 The other parameters are:

 Parameter          | type      | Description
 -------------------|-----------|------------------------------------------------------------
 MULTIPLICITY       | INTEGER   | the number of objects, by default 1.
 `orientation`      | ROTATION  | a rotation applied before translation
 `position`         | POSITION  | a translation
 `post_translation` | VECTOR    | a translation applied every time after one object is created
 `post_rotation`    | ROTATION  | a rotation applied every time after one object is created
 `mark`             | INTEGER   | this mark is given to all objects created (default = 0).
 `required`         | INTEGER   | cytosim will stop if it cannot create as many objects as specified (default=0)


 Note that `position` only applies to movable objects, and `orientation` only applies to rotatable objects.
 In addition, `post_translation` and `post_rotation` are relevant only if `(MULTIPLICITY > 1)`,
 and do not apply to the first object.\n


 Short syntax:

 @code
 new [MULTIPLICITY] CLASS NAME ( POSITION )
 @endcode

 Shorter syntax:

 @code
 new [MULTIPLICITY] CLASS NAME
 @endcode

*/

void Parser::parse_new(std::istream & is)
{
    unsigned cnt = 1;
    Tokenizer::get_integer(is, cnt);
    std::string kind = Tokenizer::get_identifier(is);
    std::string name = Tokenizer::get_identifier(is);

    // handle the case where the class name was not specified:
    if ( name.empty() )
    {
        Property * pp = simul.properties.find(kind);
        if ( pp )
        {
            name = kind;
            kind = pp->kind();
            //std::cerr << "Assuming you meant `"<< kind << " " << name << "'\n";
            //StreamFunc::show_line(std::cerr, is, is.tellg());
        }
    }

    Glossary opt;

    // Syntax sugar: () specify only position
    std::string blok = Tokenizer::get_block(is, '(');
    if ( blok.empty() )
    {
        blok = Tokenizer::get_block(is, '{');
        opt.read(blok);
    }
    else {
        opt.set_values("position", blok);
    }


    if ( do_new  &&  cnt > 0 )
    {
#if ( VERBOSE_PARSER > 0 )
        std::cerr << "-NEW " << cnt << " " << kind << " `" << name << "'\n";
#endif

        if ( opt.nb_keys() == 0 )
        {
            execute_new(kind, name, cnt);
        }
        else
        {
            std::string str;
            int created = 0;
            Vector trans(0,0,0);
            bool has_trans = opt.set(trans, "post_translation");
            bool has_rot = opt.set(str, "post_rotation");

            if ( has_rot )
            {
                Rotation rot, cum;
                std::istringstream iss(str);
                rot = Movable::readRotation(iss, trans, 0);
                cum.makeIdentity();
                for ( unsigned int n = 0; n < cnt; ++n )
                {
                    ObjectList objs = execute_new(kind, name, opt);
                    ObjectSet::translateObjects(objs, n*trans);
                    ObjectSet::rotateObjects(objs, cum);
                    created += objs.size();
                    cum = cum * rot;
                }
            }
            else
            {
                for ( unsigned int n = 0; n < cnt; ++n )
                {
                    ObjectList objs = execute_new(kind, name, opt);
                    if ( has_trans )
                        ObjectSet::translateObjects(objs, n*trans);
                    created += objs.size();
                }
            }

            int required = 0;
            if ( opt.set(required, "required")  &&  created < required )
            {
                std::cerr << "created  = " << created << "\n";
                std::cerr << "required = " << required << "\n";
                throw InvalidSyntax("could not create enough "+kind+" `"+name+"'");
            }

            if ( opt.warnings(std::cerr, -1) )
                StreamFunc::show_lines(std::cerr, is, spos, is.tellg());
        }
    }
}

//------------------------------------------------------------------------------
/**
 Delete objects:

 @code
 delete [MULTIPLICITY] CLASS NAME
 {
   mark       = INTEGER
   position   = POSITION
 }
 @endcode

 NAME can be '*', and the parameters \a mark and \a position are optional.

 To delete all objects of the class:
 @code
 delete CLASS *
 @endcode

 To delete all objects of specified NAME:
 @code
 delete CLASS NAME
 @endcode

 To delete at most CNT objects of class NAME:
 @code
 delete CNT CLASS NAME
 @endcode

 To delete all objects with a specified mark:
 @code
 delete CLASS *
 {
   mark = INTEGER
 }
 @endcode

 To delete all objects within a Space:
 @code
 delete CLASS *
 {
   position = inside, SPACE
 }
 @endcode

 The SPACE must be the name of an existing Space.
 Only 'inside' and 'outside' are valid specifications.
*/

void Parser::parse_delete(std::istream & is)
{
    int cnt = -1;
    Tokenizer::get_integer(is, cnt);

    std::string kind = Tokenizer::get_identifier(is);

    if ( simul.findSet(kind) == 0 )
        throw InvalidSyntax("unknown class `"+kind+"'");

    std::string name;
    read_property_name(is, name, kind);

    std::string blok = Tokenizer::get_block(is, '{');

    if ( do_change )
    {
        Glossary opt(blok);
        execute_delete(kind, name, opt, cnt);

        if ( opt.warnings(std::cerr) )
            StreamFunc::show_lines(std::cerr, is, spos, is.tellg());
    }
}


/**
 Mark objects:

 @code
 mark [MULTIPLICITY] CLASS NAME
 {
   mark       = INTEGER
   position   = POSITION
 }
 @endcode

 NAME can be '*', and the parameter position is optional.
 The syntax is the same as for command `delete`.
 */

void Parser::parse_mark(std::istream & is)
{
    int cnt = -1;
    Tokenizer::get_integer(is, cnt);

    std::string kind = Tokenizer::get_identifier(is);

    if ( simul.findSet(kind) == 0 )
        throw InvalidSyntax("unknown class `"+kind+"'");

    std::string name;
    read_property_name(is, name, kind);

    std::string blok = Tokenizer::get_block(is, '{');

    if ( do_change )
    {
        Glossary opt(blok);
        execute_mark(kind, name, opt, cnt);

        if ( opt.warnings(std::cerr) )
            StreamFunc::show_lines(std::cerr, is, spos, is.tellg());
    }
}

//------------------------------------------------------------------------------
/**
 Cut all fibers that intersect a given plane.

 @code
 cut fiber NAME
 {
    plane = VECTOR, REAL
 }
 @endcode

 NAME can be '*' to cut all fibers.
 The plane is specified by a normal vector \a n (VECTOR) and a scalar \a (REAL).
 The plane is defined by <em> n.x + a = 0 </em>
 */

void Parser::parse_cut(std::istream & is)
{
    std::string kind = Tokenizer::get_identifier(is);
    std::string name = Tokenizer::get_token(is);
    std::string blok = Tokenizer::get_block(is, '{');

    if ( blok.empty() )
        throw InvalidSyntax("missing block after `cut'");

    if ( do_change )
    {
        Glossary opt(blok);
        execute_cut(kind, name, opt);
    }
}


//------------------------------------------------------------------------------

/**
 Perform simulation steps, and write frames to files.
 Currently, only 'run simul *' is supported.

 @code
 run [NB_STEPS] simul *
 {
   nb_steps  = INTEGER
   solve     = SOLVE_MODE
   event     = RATE, ( CODE )
   nb_frames = INTEGER
   prune     = BOOL
 }
 @endcode

 The optional specification [NB_STEPS] enables the short syntax:
 @code
 run NB_STEPS simul *
 @endcode

 Option        |  Default  |   Action
 --------------|-----------|---------------------------------------------------------
 `nb_steps`    |  1        | number of simulation steps
 `solve`       |  1        | Enable/disable motion of the objects
 `event`       |  none     | custom code executed stochastically with prescribed rate
 `nb_frames`   |  0        | number of states written to trajectory file
 `prune`       |  true     | Print only parameters that are different from default
 \n

 If set, `event` defines an event occuring at a rate specified by the positive real \c RATE.
 The action is defined by CODE, a string enclosed with parenthesis containing cytosim commands.
 This code will be executed at stochastic times with the specified rate.

 Example:
 @code
 event = 10, ( new fiber actin { position=(rectangle 1 6); length=0.1; } )
 @endcode

 Calling `run` will not output the initial state, but this can be done with `write`:
 @code
 write state objects.cmo { append = 0 }

 run 1000 simul *
 {
   nb_frames = 10
 }
 @endcode

 @todo: add code executed at each time-step
 */

void Parser::parse_run(std::istream & is)
{
    unsigned cnt = 1;
    bool has_cnt = Tokenizer::get_integer(is, cnt);

    std::string kind = Tokenizer::get_identifier(is);

    if ( kind != "simul" )
        throw InvalidSyntax("only `run simul *' is supported");

    std::string name = Tokenizer::get_token(is);
    std::string blok = Tokenizer::get_block(is, '{');

    if ( blok.empty() && !has_cnt )
    {
#ifdef BACKWARD_COMPATIBILITY
        has_cnt = Tokenizer::get_integer(is, cnt);
        if ( has_cnt )
            std::cerr << "deprecated syntax 'run simul * CNT' accepted\n";
        else
#endif
        throw InvalidSyntax("the number of simulation steps must be specified");
    }

    if ( do_run )
    {
        if ( name != simul.prop->name()  &&  name != "*" )
            throw InvalidSyntax("please, use `run CNT simul *'");

        Glossary opt(blok);

        if ( opt.set(cnt,"nb_steps") && has_cnt )
            throw InvalidSyntax("the number of simulation steps was specified twice");;

        execute_run(opt, cnt, do_write);

        if ( opt.warnings(std::cerr) )
            StreamFunc::show_lines(std::cerr, is, spos, is.tellg());
    }
}

//------------------------------------------------------------------------------
/**
 Include another config file, and executes it.

 @code
 include FILE_NAME
 {
   required = BOOL
 }
 @endcode

 By default, required = 1, and execution will terminate if the file is not found.
 If required is set to 0, the file will be read,
 but execution will continue even if the file is not found.

 \todo: able to specify do_set and do_new for command 'include'
*/

void Parser::parse_read(std::istream & is)
{
    bool required = true;
    std::string file = Tokenizer::get_token(is);

    if ( file.empty() )
        throw InvalidSyntax("missing/invalid file name after 'include'");

    std::string blok = Tokenizer::get_block(is, '{');
    if ( ! blok.empty() )
    {
        Glossary opt(blok);
        opt.set(required, "required");
        if ( opt.warnings(std::cerr) )
            StreamFunc::show_lines(std::cerr, is, spos, is.tellg());
    }

    std::ifstream fis(file.c_str());
    if ( ! fis.fail() )
    {
#if ( VERBOSE_PARSER > 0 )
        std::cerr << "-INCLUDE "<< file << std::endl;
#endif
        parse(fis, "while reading `"+file+"'");
    }
    else
    {
        if ( required )
            throw InvalidSyntax("could not open file `"+file+"'");
        else
            Cytosim::warning("could not open file `%s'\n", file.c_str());
    }
}

//------------------------------------------------------------------------------
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

 Note that the simulation time will be changed to the one specified in the file,
 but it can be reset with:
 @code
 change simul * { time=0 }
 @endcode
 */

void Parser::parse_import(std::istream & is)
{
    std::string file = Tokenizer::get_token(is);

    if ( file.empty() )
        throw InvalidSyntax("missing/invalid file name after 'import'");

    std::string blok = Tokenizer::get_block(is, '{');
    Glossary opt(blok);

    if ( do_new )
    {
        execute_import(file, opt);
        if ( opt.warnings(std::cerr) )
            StreamFunc::show_lines(std::cerr, is, spos, is.tellg());
    }
}



/**
 Export state to file. The general syntax is:

 @code
 export WHAT FILE_NAME
 {
 append = BOOL
 binary = BOOL
 }
 @endcode

 WHAT must be ``objects``, and by default, `binary` and `append` are both `true`.
 If `*` is specified instead of a file name, the current trajectory file will be used.


 Short syntax:
 @code
 export objects FILE_NAME
 @endcode


 Examples:

 @code
 export objects sim_objects.cmo { append=0 }
 @code
 @endcode
 export objects sim_objects.txt { binary=0 }
 @endcode

 Attention: this command is disabled for `play`.
 */

void Parser::parse_export(std::istream & is)
{
    std::string what = Tokenizer::get_token(is);
    std::string file = Tokenizer::get_token(is);

    if ( file.empty() )
        throw InvalidSyntax("missing/invalid file name after 'export'");

    std::string blok = Tokenizer::get_block(is, '{');
    Glossary opt(blok);

    if ( do_write )
    {
        //what = Tokenizer::strip_block(what);
        execute_export(file, what, opt);
        if ( opt.warnings(std::cerr) )
            StreamFunc::show_lines(std::cerr, is, spos, is.tellg());
    }
}

/**
 Export formatted data to file. The general syntax is:

 @code
 report WHAT FILE_NAME
 {
 append = BOOL
 }
 @endcode

 Short syntax:
 @code
 report WHAT FILE_NAME
 @endcode


 WHAT should be a valid argument to `report`:
 @copydetails Simul::report

 If `*` is specified instead of a file name, the report is sent to the standard output.

 Examples:

 @code
 report parameters parameters.cmo { append=0 }
 report fiber:length fibers.txt
 @endcode

 Note that this command is disabled for `play`.
 */

void Parser::parse_report(std::istream & is)
{
    std::string what = Tokenizer::get_token(is);
    std::string file = Tokenizer::get_token(is);

    if ( file.empty() )
        throw InvalidSyntax("missing/invalid file name after 'report'");

    std::string blok = Tokenizer::get_block(is, '{');
    Glossary opt(blok);

    if ( do_write || file == "*" )
    {
        execute_report(file, what, opt);

        if ( opt.warnings(std::cerr) )
            StreamFunc::show_lines(std::cerr, is, spos, is.tellg());
    }
}

//------------------------------------------------------------------------------

/**
 Call custom function

 @code
 call FUNCTION_NAME
 @endcode

 FUNCTION_NAME should be custom?, where ? is a digit.

 \todo: propagate the arguments (do_set, do_new) to the custom commands
 */
void Parser::parse_call(std::istream & is)
{
    std::string str = Tokenizer::get_token(is);

    if ( str.empty() )
        throw InvalidSyntax("missing command name after 'call'");

    if ( str.size() == 7  &&  str.compare(0, 6, "custom") == 0 )
    {
        int nb = str[6] - '0';
        switch( nb )
        {
            case 0: simul.custom0(); return;
            case 1: simul.custom1(); return;
            case 2: simul.custom2(); return;
            case 3: simul.custom3(); return;
            case 4: simul.custom4(); return;
            case 5: simul.custom5(); return;
            case 6: simul.custom6(); return;
            case 7: simul.custom7(); return;
            case 8: simul.custom8(); return;
            case 9: simul.custom9(); return;
            default:
                throw InvalidSyntax("unknown command after 'call'");
        }
    }
}

//------------------------------------------------------------------------------
/**
 Repeat specified code.

 @code
 repeat INTEGER { CODE }
 @endcode

 */

void Parser::parse_repeat(std::istream & is)
{
    unsigned cnt = 1;

    if ( ! Tokenizer::get_integer(is, cnt) )
        throw InvalidSyntax("missing number after 'repeat'");

    std::string code = Tokenizer::get_block(is, '{');

    for ( unsigned int c = 0; c < cnt; ++c )
    {
        //it is best to use a fresh stream for each instance:
        std::istringstream iss(code);
        parse(iss, "while executing `repeat'");
        hold();
    }
}

//------------------------------------------------------------------------------

/**
 Terminates execution

 @code
 stop
 @endcode
 */
void Parser::parse_stop(std::istream & is)
{
    std::string str = Tokenizer::get_token(is);

    if ( str.empty() )
        throw Exception("halting program at command 'stop'");

    else if ( str == "if" )
    {
        str = Tokenizer::get_token(is);
        ABORT_NOW("unfinished code");
    }
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 The configuration file should contain a succession of commands.
 Most command have the following syntax:
 @code
 COMMAND CLASS NAME
 {
    PARAMETERS
 }
 @endcode

 Essential commands:

 Command        |   Action
 ---------------|---------------------------------------------------------
 `set`          | Create a new Property, and set parameter values
 `change`       | Change parameter values in an existing Property
 `new`          | Create one or many new objects
 `delete`       | Delete objects
 `run`          | Perform simulation steps
 `read`         | Read another config file
 `write`        | export formatted data with selected object properties
 `import`       | Import Objects from trajectory file
 `export`       | Export all Objects to file, with their coordinates

 Other commands:

 Command        |   Action
 ---------------|---------------------------------------------------------
 `mark`         | Mark objects
 `repeat`       | Execute code a number of times
 `stop`         | Stop program
 `cut`          | Cut Fibers
 `call`         | Call a custom function

 */
void Parser::parse(std::istream & is, std::string const& msg)
{
    std::streampos fpos;
    std::string tok;

    try {
        while ( is.good() )
        {
            do {
                spos = is.tellg();
                fpos = spos;
                tok = Tokenizer::get_token(is);
                if ( is.fail() ) return;
            } while ( tok[0] == '\n' );


            // skip matlab-style comments
            if ( tok[0] == '%' )
            {
                if ( '{' == is.peek() )
                    Tokenizer::get_until(is, "%}");
                else
                    Tokenizer::get_line(is);
                continue;
            }

#ifdef BACKWARD_COMPATIBILITY
            // skip C-style comments
            if ( tok[0] == '/' )
            {
                if ( '*' == is.peek() )
                {
                    // multi-line comments
                    Tokenizer::get_until(is, "*/");
                    continue;
                }
                else if ( '/' == is.peek() )
                {
                    // single-line comment
                    Tokenizer::get_line(is);
                    continue;
                }
            }
#endif

#if ( VERBOSE_PARSER > 8 )
            std::cerr << "COMMAND |" << tok << "| " << std::endl;
            StreamFunc::show_line(std::cout, is, is.tellg());
#endif

            if ( tok == "set" )
                parse_set(is);
            else if ( tok == "change" )
                parse_change(is);
            else if ( tok == "new" )
                parse_new(is);
            else if ( tok == "delete" )
                parse_delete(is);
            else if ( tok == "mark" )
                parse_mark(is);
            else if ( tok == "cut" )
                parse_cut(is);
            else if ( tok == "run" )
                parse_run(is);
            else if ( tok == "include" )
                parse_read(is);
            else if ( tok == "read" )
                parse_read(is);
            else if ( tok == "write" )
                parse_report(is);
            else if ( tok == "report" )
                parse_report(is);
            else if ( tok == "import" )
                parse_import(is);
            else if ( tok == "export" )
                parse_export(is);
            else if ( tok == "call" )
                parse_call(is);
            else if ( tok == "repeat" )
                parse_repeat(is);
            else if ( tok == "stop" )
                parse_stop(is);
            else {
                throw InvalidSyntax("unexpected token `"+tok+"'");
            }

            hold();
        }
    }
    catch( Exception & e )
    {
        if ( msg.size() )
            e << "\n " + msg + "\n";
        e << StreamFunc::get_lines(is, fpos, is.tellg());
        throw;
    }
}



void Parser::readConfig(std::string const& file)
{
    std::ifstream is(file.c_str(), std::ifstream::in);

    if ( ! is.good() )
    {
        std::ostringstream oss;
        if ( file == "config.cym" )
            oss << "You must specify a config file" << std::endl;
        else
            oss << "Could not open config file `" << file << "'" << std::endl;
        throw InvalidIO(oss.str());
    }

#if ( VERBOSE_PARSER > 0 )
    std::cerr << "-------  Cytosim reads " << file << std::endl;
    std::cerr << "  set=" << do_set << "  change=" << do_change << "  new=" << do_new;
    std::cerr << "  run=" << do_run << "  write=" << do_write << std::endl;
#endif

    parse(is, "while reading `"+file+"'");
}
