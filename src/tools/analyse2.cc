// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/**
 This is a modern template for a program to analyse simulation results:
 
 It reads trajectory-file, calculates some characteristics and print them out
 */

#include "frame_reader.h"
#include "iowrapper.h"
#include "glossary.h"
#include "messages.h"
#include "parser.h"
#include "simul.h"

Simul simul;

//------------------------------------------------------------------------------

void analyse(std::ostream& os, std::string const& what, Glossary& opt, int frm)
{
    os << "% frame " << frm << std::endl;
    os << "% time " << simul.simTime() << std::endl;
    simul.report(os, what, opt);
}


//------------------------------------------------------------------------------

void help(std::ostream& os)
{
    os << "Generate reports/statistics on the simulation objects\n";
    os << "\n";
    os << "Syntax:\n";
    os << "       analyse WHAT [prefix='time'] [frame=INTEGER]\n";
    os << "\n";
    os << "Analyse will generate the same reports as Simul::report()\n";
    os << "The documentation of Simul::report() has a list of possible values for WHAT\n";
    os << "\n";
    os << "The report is send to the standard output";
    
    os << std::endl;
}

//------------------------------------------------------------------------------


int main(int argc, char* argv[])
{
    Cytosim::silent();
    
    if ( argc < 2 || strstr(argv[1], "help") )
    {
        help(std::cout);
        return EXIT_SUCCESS;
    }
    
    std::string what = argv[1];
    
    Glossary opt;
    opt.readStrings(argc-1, argv+1);
    
    FrameReader reader;
    
    try
    {
        Parser(simul, 1, 1, 0, 0, 0).readProperties();
        reader.openFile(simul.prop->trajectory_file);
        
        int frame = 0;
        if ( opt.set(frame, "frame") )
        {
            if ( 0 == reader.readFrame(simul, frame) )
                analyse(std::cout, what, opt, frame);
            else
                std::cerr << "Error: missing frame " << frame << std::endl;
        }
        else
        {
            while ( 0 == reader.readNextFrame(simul) )
            {
                analyse(std::cout, what, opt, frame);
                ++frame;
            }
        }
    }
    catch( Exception & e )
    {
        std::cerr << "Aborted: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
