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


void splash(std::ostream & os = std::cout)
{
    os << "  ------------------------------------------------------------- \n";
    os << " |  CytoSIM  -  www.cytosim.org  -  version PI  -  May  2017   |\n";
    os << "  ------------------------------------------------------------- \n";
}

void help(std::ostream& os)
{
    os << "Synopsis:\n";
    os << "       Generates reports/statistics about Cytosim's objects\n";
    os << "       for DIM = " << DIM << "\n";
    os << "Syntax:\n";
    os << "       report [time] WHAT [OPTIONS]\n";
    os << "Options:\n";
    os << "       precision=INTEGER\n";
    os << "       column=INTEGER\n";
    os << "       verbose=0\n";
    os << "       frame=INTEGER[,INTEGER[,INTEGER[,INTEGER]]]\n";
    os << "       period=INTEGER\n";
    os << "       input=FILE_NAME\n";
    os << "       output=FILE_NAME\n";
    os << "\n";
    os << "  This tool must be invoked in a directory containing the simulation output,\n";
    os << "  and it will generate reports by calling Simul::report(). The only required\n";
    os << "  argument `WHAT` determines what data will be generated. Many options are\n";
    os << "  available, but are not listed here. Please check the HTML documentation.\n";
    os << "  By default, all frames in the file are processed in order, but a frame index,\n";
    os << "  or multiple indices can be specified (the first frame has index 0).\n";
    os << "  The input trajectory file is `objects.cmo` unless otherwise specified.\n";
    os << "  The result is sent to standard output unless a file is specified as `output`\n";
    os << "  Attention: there should be no whitespace in any of the option.\n";
    os << "\n";
    os << "Examples:\n";
    os << "       report fiber:points\n";
    os << "       report fiber:points frame=10 > fibers.txt\n";
    os << "       report fiber:points frame=10,20 > fibers.txt\n";
}

//------------------------------------------------------------------------------

void report_raw(std::ostream& os, std::string const& what, int frm, Glossary& opt)
{
    if ( verbose > 0 )
    {
        os << "% frame   " << frm << '\n';
        simul.report(os, what, opt);
    }
    else
    {
        std::stringstream ss;
        simul.report(ss, what, opt);
        StreamFunc::skip_lines(os, ss, '%');
    }
}


void report_prefix(std::ostream& os, std::string const& what, int frm, Glossary& opt)
{
    char prefix[256] = { 0 };
    snprintf(prefix, sizeof(prefix), "%9.3f ", simul.simTime());
    
    std::stringstream ss;
    
    if ( verbose )
    {
        os << "% frame   " << frm << '\n';
        simul.report(ss, what, opt);
        StreamFunc::prefix_lines(os, ss, prefix, '%', 0);
    }
    else
    {
        simul.report(ss, what, opt);
        StreamFunc::prefix_lines(os, ss, prefix, 0, '%');
    }
}


void report(std::ostream& os, std::string const& what, int frm, Glossary& opt)
{
    try
    {
        report_raw(os, what, frm, opt);
    }
    catch( Exception & e )
    {
        std::cerr << "Aborted: " << e.what() << '\n';
        exit(EXIT_FAILURE);
    }
}


//------------------------------------------------------------------------------


int main(int argc, char* argv[])
{
    if ( argc < 2 || strstr(argv[1], "help") )
    {
        help(std::cout);
        return EXIT_SUCCESS;
    }
    
    if ( strstr(argv[1], "info") || strstr(argv[1], "--version")  )
    {
        splash(std::cout);
        std::cout << " DIM = " << DIM << '\n';
        return EXIT_SUCCESS;
    }
    
    Glossary arg;
    
    std::string input = simul.prop->trajectory_file;
    std::string str, what = argv[1];
    std::ostream * osp = &std::cout;
    std::ofstream ofs;
    
    arg.readStrings(argc-1, argv+1);
    
    unsigned frame = 0;
    unsigned period = 1;
    
    arg.set(input, ".cmo") || arg.set(input, "input");;
    arg.set(verbose, "verbose");
    arg.set(period, "period");
    
    FrameReader reader;
    RNG.seedTimer();
    
    try
    {
        Parser(simul, 1, 1, 0, 0, 0).readProperties();
        reader.openFile(input);
    }
    catch( Exception & e )
    {
        std::clog << "Aborted: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
    
    if ( arg.set(str, "output") )
    {
        try {
            ofs.open(str.c_str());
        }
        catch( ... )
        {
            std::clog << "Cannot open output file\n";
            return EXIT_FAILURE;
        }
        osp = &ofs;
    }
    
    Cytosim::silent();
    
    
    if ( arg.has_key("frame") )
    {
        // multiple frame indices can be specified:
        unsigned s = 0;
        while ( arg.set(frame, "frame", s) )
        {
            // try to load the specified frame:
            if ( 0 == reader.readFrame(simul, frame) )
                report(*osp, what, frame, arg);
            else
            {
                std::cerr << "Error: missing frame " << frame << '\n';
                return EXIT_FAILURE;
            }
            ++s;
        };
    }
    else
    {
        // process every 'period' frame in the file:
        while ( 0 == reader.readNextFrame(simul) )
        {
            if ( 0 == frame % period )
                report(*osp, what, frame, arg);
            ++frame;
        }
    }
    
    if ( ofs.is_open() )
        ofs.close();
    
    /// check that all specified parameters have been used:
    std::stringstream ss;
    if ( arg.warnings(ss) > 1 )
        std::cerr << ss.str() << '\n';
    
    return EXIT_SUCCESS;
}
