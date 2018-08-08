// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
//------------------just read and write coordinate file----------------------
//    mostly for testing, but can also make some modifications on state
//    F. Nedelec, EMBL 2002

#include "simul.h"
#include "parser.h"
#include "glossary.h"
#include "iowrapper.h"
#include "exceptions.h"

void help()
{
    printf("Synopsis: Copy a trajectory file.\n");
    printf(" The file is written in the latest format, either binary or text-based.\n");
    printf(" A category of objects can be removed with option skip=WHAT\n\n");
    
    printf("Usage:\n");
    printf("    sieve input output [options]\n\n");
    printf("possible options are:\n");
    printf("    binary=0   generate a text file\n");
    printf("    binary=1   generate a binary file\n");
    printf("    verbose=?  set the verbose level\n");
    printf("    skip=WHAT  remove all objects of class WHAT\n");
}


void inventory(std::ostream& os, Simul& sim, int cnt)
{
    os << "Frame " << cnt << " written with:" << std::endl;
    if ( sim.spaces.size() )     os << std::setw(8) << sim.spaces.size()  << " spaces\n";
    if ( sim.fibers.size() )     os << std::setw(8) << sim.fibers.size()  << " fibers\n";
    if ( sim.beads.size() )      os << std::setw(8) << sim.beads.size()   << " beads\n";
    if ( sim.solids.size() )     os << std::setw(8) << sim.solids.size()  << " solids\n";
    if ( sim.spheres.size() )    os << std::setw(8) << sim.spheres.size() << " spheres\n";
    if ( sim.singles.size() )    os << std::setw(8) << sim.singles.size() << " singles\n";
    if ( sim.couples.size() )    os << std::setw(8) << sim.couples.size() << " couples\n";
    if ( sim.organizers.size() ) os << std::setw(8) << sim.organizers.size() << " organizers\n";
}

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    if ( argc < 3 )
    {
        help();
        return EXIT_SUCCESS;
    }
    
    std::string input  = argv[1];
    std::string output = argv[2];

    Simul simul;
    Glossary glos;
    
    glos.readStrings(argc-2, argv+2);
    
    int verbose = 0;
    glos.set(verbose, "verbose");
    
    std::string skip;
    glos.set(skip, "skip");
    
    bool binary = true;
    glos.set(binary, "binary");

    
    InputWrapper in;
    try {
        Parser(simul, 1, 1, 0, 0, 0).readProperties();
        in.open(input.c_str(), "rb");
    }
    catch( Exception & e ) {
        std::cerr<<"Error opening input file `" << input << "' :" << std::endl;
        std::cerr<< e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    std::cerr << ">>>>>> Copying `" << input << "' -> `" << output << "'" << std::endl;

    int cnt = 0;

    while ( in.good() )
    {
        try {
            if ( 0 == simul.reloadObjects(in) )
            {
                if ( skip.size() )
                {
                    ObjectSet * set = simul.findSet(skip);
                    if ( set )
                        set->erase();
                }

                //inventory(std::cout, simul, cnt);
                ++cnt;
                
                try {
                    simul.writeObjects(output, binary, true);
                }
                catch( Exception & e ) {
                    std::cerr<<"could not write to `" << output << "' :" << e.what() << std::endl;
                    return EXIT_FAILURE;
                }
            }
        }
        catch( Exception & e ) {
            std::cerr << "Error in frame: " << e.what() << std::endl;
        }
    }
    
    std::cerr << ">>>>>> " << cnt << " frames written to `" << output << "'" << std::endl;
    return 0;
}
