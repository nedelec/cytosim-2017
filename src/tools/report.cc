// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/**
 This is a program to analyse simulation results:
 it reads a trajectory-file, and print some data from it.
*/

#include <fstream>
#include <sstream>

#include "stream_func.h"
#include "frame_reader.h"
#include "iowrapper.h"
#include "glossary.h"
#include "messages.h"
#include "parser.h"
#include "simul.h"

Simul simul;
int verbose = 1;

//------------------------------------------------------------------------------

void report_raw(std::ostream& os, std::string const& what, int frm, Glossary& opt)
{
    if ( verbose )
    {
        os << "% frame " << frm << std::endl;
        os << "% time " << simul.simTime() << std::endl;
    }
    simul.report(os, what, opt);
}


void report_prefix(std::ostream& os, std::string const& what, int frm, Glossary& opt)
{
    if ( verbose )
    {
        os << "% frame " << frm << std::endl;
        os << "% time " << simul.simTime() << std::endl;
    }
    
    std::stringstream pss, ss;
    pss << std::left << std::setw(8) << std::setprecision(3) << simul.simTime() << " ";
    const std::string pre = pss.str();
    simul.report(ss, what, opt);

    if ( verbose )
        StreamFunc::prefix_lines(os, ss, pre, 0, '%');
    else
        StreamFunc::prefix_lines(os, ss, pre, '%', 0);
}


void report(std::ostream& os, std::string const& what, int frm, Glossary& opt)
{
    std::string prefix;
    opt.set(prefix, "prefix");

    if ( prefix == "time" )
        report_prefix(os, what, frm, opt);
    else
        report_raw(os, what, frm, opt);
}

//------------------------------------------------------------------------------

void help(std::ostream& os)
{
    os << "Synopsis: generate reports/statistics about cytosim's objects\n";
    os << "\n";
    os << "Syntax:\n";
    os << "       report WHAT [prefix=time] [verbose=0] [frame=INTEGER] [output=FILE_NAME]\n";
    os << "\n";
    os << "This will generate reports using Simul::report()\n";
    os << "See the documentation of Simul::report() for a list of possible values for WHAT\n";
    os << "\n";
    os << " Compiled at "<<__TIME__<< " on " <<__DATE__<< "\n";
}

//------------------------------------------------------------------------------


int main(int argc, char* argv[])
{
    MSG.silent();
    
    if ( argc < 2 || strstr(argv[1], "help") )
    {
        help(std::cout);
        return EXIT_SUCCESS;
    }
    
    if ( strstr(argv[1], "info") )
    {
        std::cout << "www.cytosim.org" << std::endl;
        std::cout << "   Compiled at "<<__TIME__<< " on " <<__DATE__<< std::endl;
        std::cout << "   DIM = " << DIM << std::endl;
        return EXIT_SUCCESS;
    }

    std::string input = simul.prop->trajectory_file;
    std::string str, what = argv[1];
    
    std::ostream * osp = &std::cout;
    std::ofstream ofs;
    
    Glossary opt;
    opt.readStrings(argc-1, argv+1);

    opt.set(input,   ".cmo") || opt.set(input, "input");;
    opt.set(verbose, "verbose");

    if ( opt.set(str, "output") )
    {
        try {
            ofs.open(str.c_str());
        }
        catch( ... )
        {
            std::cerr << "Cannot open output file" << std::endl;
            return EXIT_FAILURE;
        }
        osp = &ofs;
    }
    
    FrameReader reader;
    try
    {
        Parser(simul, 1, 1, 0, 0, 0).readProperties();
        reader.openFile(input);
    }
    catch( Exception & e )
    {
        std::cerr << "Aborted: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    int frame = 0;
    try
    {
        if ( opt.set(frame, "frame") )
        {
            // try to load the specified frame:
            if ( reader.readFrame(simul, frame) )
            {
                if ( 0 == reader.readFrame(simul, -1) )
                {
                    frame = reader.frame();
                    std::cerr << "Warning: only found frame " << frame << std::endl;
                }
                else
                {
                    std::cerr << "Error: missing frame " << frame << std::endl;
                    return EXIT_FAILURE;
                }
            }
            report(*osp, what, frame, opt);
        }
        else
        {
            // load all frames in the file:
            while ( 0 == reader.readNextFrame(simul) )
            {
                report(*osp, what, frame, opt);
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
