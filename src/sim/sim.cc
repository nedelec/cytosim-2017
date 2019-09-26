// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#include "simul.h"
#include "parser.h"
#include "messages.h"
#include "glossary.h"
#include "exceptions.h"
#include "tictoc.h"
#include <csignal>

using std::endl;


void splash(std::ostream & os = std::cout)
{
    os << "  ------------------------------------------------------------- " << endl;
    os << " |  CytoSIM  -  www.cytosim.org  -  version PI  -  May  2017   |" << endl;
    os << "  ------------------------------------------------------------- " << endl;
}


void help(std::ostream & os = std::cout)
{
    os << " Command line options:" << endl;
    os << "    FILENAME   set config file if FILENAME ends by `.cym'" << endl;
    os << "    *          send messages to terminal instead of `messages.cmo'" << endl;
    os << "    info       print build options" << endl;
    os << "    help       print this message" << endl;
    os << "    -          do not splash standard output" << endl;
}


void info(std::ostream & os = std::cout)
{
    os << "www.cytosim.org - sim" << endl;
    os << " https://github.com/nedelec/cytosim" << endl;
    os << " Compiled at "<<__TIME__<< " on " <<__DATE__<< endl;
    os << " Precision: " << sizeof(real) << " bytes,  epsilon = " << REAL_EPSILON << endl;
    
#ifdef NDEBUG
    os << " (no assertions)" << endl;
#else
    os << " with assertions" << endl;
#endif
    
    os << " DIM = " << DIM << endl;
}


void killed_handler(int sig)
{
    Cytosim::MSG("killed\n");
    exit(sig);
}

//------------------------------------------------------------------------------
//=================================  MAIN  =====================================
//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    
    // Register a function to be called for Floating point exceptions:
    if ( signal(SIGINT, killed_handler) == SIG_ERR )
        std::cerr << "Could not register SIGINT handler\n";
    
    if ( signal(SIGTERM, killed_handler) == SIG_ERR )
        std::cerr << "Could not register SIGTERM handler\n";

    Simul simul;

    //parse the command line:
    Glossary glos;
    glos.readStrings(argc, argv);
    
    if ( glos.use_key("help") )
    {
        help();
        return EXIT_SUCCESS;
    }
    
    if ( glos.use_key("info") )
    {
        info();
        return EXIT_SUCCESS;
    }
    
    if ( ! glos.use_key("*") )
    {
        Cytosim::open("messages.cmo");
    }        
    
    if ( !glos.use_key("-") )
        splash();
    
    char date[26];
    TicToc::date(date, sizeof(date));
    Cytosim::MSG("CYTOSIM started %s\n", date);
    
    
    try {
        simul.initialize(glos);
    }
    catch( Exception & e ) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch(...) {
        std::cerr << "Error: an unknown exception occured during initialization" << std::endl;
        return EXIT_FAILURE;
    }
    
    glos.warnings(std::cerr);

    Cytosim::MSG("============================== RUNNING ================================\n");

    try {
        Parser(simul, 1, 1, 1, 1, 1).readConfig(simul.prop->config);
    }
    catch( Exception & e ) {
        std::cerr << std::endl << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch(...) {
        std::cerr << std::endl << "Error: an unknown exception occured" << std::endl;
        return EXIT_FAILURE;
    }
    
    TicToc::date(date, sizeof(date));
    Cytosim::MSG("%s\n", date);
    Cytosim::MSG("end\n");
    Cytosim::close();
    return EXIT_SUCCESS;
}
