// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
//------------------------------------------------------------------------------
//                                 reader.cc
//
//     this is mostly a test for the class defined in frame_reader.h
//     but it can be used to navigate from frame to frame in a object-file
//------------------------------------------------------------------------------

#include <cstring>
#include <cctype>
#include <cstdlib>

#include "glossary.h"
#include "messages.h"
#include "iowrapper.h"
#include "frame_reader.h"
#include "simul.h"
#include "parser.h"

//------------------------------------------------------------------------------

void help(std::ostream & os = std::cout)
{
    os << "Read cytosim trajectory file\n";
    os << "\n";
    os << "Syntax:  reader [options] file_in file_out\n";
    os << "\n";
    os << "options:\n";
    os << "     help       display this message\n";
    os << "     binary=0   write text coordinates in `file_out'\n";
    os << "     binary=1   write binary coordinates in `file_out'\n";
    os << "     verbose=?  set the verbose level\n";
    os << "\n";
}

void instructions(std::ostream & os = std::cout)
{
    os << "Commands understood at prompt:\n";
    os << "  'q'      quit\n";
    os << "  'n'      read next frame\n";
    os << "  'w'      write frame\n";
    os << "  'c'      clear buffer without changing positions\n";
    os << "  'r'      rewind\n";
    os << "  'e'      erase state\n";
    os << " INTEGER   read specified frame if possible\n";
}


//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    Glossary glos;
    glos.readStrings(argc, argv);
    
    if ( glos.use_key("help") )
    {
        help();
        instructions();
        return EXIT_SUCCESS;
    }
    
    std::string output = "objects2.cmo";
    glos.set(output, "output");
    
    bool binary = true;
    glos.set(binary, "binary");
    
    int verbose = 0;
    glos.set(verbose, "verbose");
    Cytosim::setVerbose(verbose);

    Simul simul;
    FrameReader reader;
    try {
        Parser(simul, 1, 1, 0, 0, 0).readProperties();
        reader.openFile(simul.prop->trajectory_file);
    }
    catch( Exception & e )
    {
        std::cerr << "Aborted: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    if ( !reader.good() )
    {
        printf("File could not be oppened\n");
        return EXIT_FAILURE;
    }
    
    int frame;
    
    char user[1024] = "\0";
    
    printf("TestReader: read/write frame for cytosim. Enter (h) for help\n");
    while ( true )
    {
        if ( reader.frame() < 0 )
            printf("No frame in buffer\n");
        else
            printf("Frame %i in buffer: %9.6f s, %i Fibers, %i Couples\n", 
                   reader.frame(), simul.simTime(), simul.fibers.size(), simul.couples.size());
        
        printf(" ? ");
        fgets(user, sizeof(user), stdin);
        
        if ( isdigit( user[0] ))
        {
            if ( 1 == sscanf(user, "%i", &frame ) )
            {
                try {
                    if ( 0 != reader.readFrame(simul, frame) )
                        printf("frame not found: ");
                }
                catch( Exception & e ) {
                    printf("Error in frame %i: %s\n", frame, e.what());
                }
            }
        }
        else
        {
            switch( user[0] )
            {
                case '\n':
                case 'n':
                    try {
                        if ( 0 == reader.readNextFrame(simul) )
                            printf("next: ");
                    }
                    catch( Exception & e ) {
                        printf("IO Error: %s\n", e.what());
                    }
                    break;
                    
                case 'w':
                    simul.writeObjects(output, binary, true);
                    break;
                    
                case 'e':
                    simul.erase();
                    break;
                    
                case 'b':
                    binary = !binary;
                    printf("binary = %i\n", binary);
                    break;
                    
                case 'c':
                    reader.clearPositions();
                    break;

                case 'r':
                    reader.rewind();
                    break;
                    
                case 'q': case 'Q': case 27:
                    return 0;
                    
                default:
                    instructions();
                    break;
            }
        }
    }
}
