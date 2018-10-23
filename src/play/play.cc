// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
//main file for the graphical part of Cytosim

#include "play.h"
#include "glossary.h"
#include "exceptions.h"
#include "offscreen.h"
#include "messages.h"
#include "gle.h"
using namespace gle;

#include "fiber_disp.h"
#include "line_disp.h"
#include "point_disp.h"

using glApp::flashText;

#include "play_disp.cc"
#include "play_keys.cc"
#include "play_sim.cc"
#include "play_mouse.cc"
#include "play_menus.cc"
#include "assert_macro.h"
#include <fstream>
#include "tictoc.h"

extern bool functionKey[];

//------------------------------------------------------------------------------
using std::endl;

void splash(std::ostream & os = std::cout)
{
    os << "  ------------------------------------------------------------- " << endl;
    os << " |  CytoSIM  -  www.cytosim.org  -  version PI  -  May  2017   |" << endl;
    os << "  ------------------------------------------------------------- " << endl;
}


void info(std::ostream & os = std::cout)
{
    os << "www.cytosim.org - play" << endl;
    os << " https://github.com/nedelec/cytosim" << endl;
    os << " Compiled at "<<__TIME__<< " on " <<__DATE__<< endl;
    os << " Precision: " << sizeof(real) << " bytes,  epsilon = " << REAL_EPSILON << endl;
    
#ifdef NDEBUG
    os << " (no assertions)" << endl;
#else
    os << " with assertions" << endl;
#endif

    os << " PNG : " << SaveImage::supported("png") << endl;
    os << " DIM = " << DIM << endl;
}


void help(std::ostream & os = std::cout)
{
    os << " Options can be specified on the command line while invoking cytosim.\n"
          "\n Global options:\n"
          "         cd                         change working directory to match that of input file\n"
          "         live                       enter live simulation mode directly\n"
          "         FILE.cym                   specify input config file\n"
          "         FILE.cmo                   specify trajectory file\n"
          "         FILE.cms                   specify display configuration file\n"
          "         PARAMETER=value            set parameter value (example size=512)\n"
          "\n Rendering:\n"
          "         image frame=INT            render specified frame\n"
          "         image frame=INT,INT,...    render several frames\n"
          "         image magnification=INT    render sub-pixel resolution image\n"
          "         movie                      render all frames\n"
          "         movie period=INT           render one every INT frames\n"
          "\n Help:\n"
          "         keys                       print brief list of keyboard commands\n"
          "         help                       print this help\n"
          "         parameters                 print a list of parameters\n"
          "                                    (there should be no space around the equal sign)\n";
}


//------------------------------------------------------------------------------
/// kill the slave thread and free memory
void exit_handler()
{
    simThread.cancel();
    dproperties.erase();
    gle::release();
    //fprintf(stderr, "Goodbye...\n");
}


enum { OFFSCREEN_IMAGE=1, OFFSCREEN_MOVIE=7 };


//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    bool goLive = false;
    int  off_screen = 0;
    int  magnification = 1;
    
    Cytosim::quiet();
    Glossary glos;
    
    try {
        glos.readStrings(argc, argv);
    }
    catch( Exception & e ) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    // check for major options:
    
    if ( glos.use_key("help") )
    {
        splash();
        help();
        return EXIT_SUCCESS;
    }
    
    if ( glos.use_key("info") )
    {
        info();
        return EXIT_SUCCESS;
    }
    
    if ( glos.use_key("keys") )
    {
        Player::help(std::cout);
        return EXIT_SUCCESS;
    }

    if ( glos.use_key("live") )
        goLive = true;
    
    if ( glos.use_key("image") )
    {
        off_screen = OFFSCREEN_IMAGE;
        glos.set(magnification, "magnification");
    }

    if ( glos.use_key("poster") )
    {
        off_screen = OFFSCREEN_IMAGE;
        magnification = 3;
    }

    if ( glos.use_key("movie") )
        off_screen = OFFSCREEN_MOVIE;

    // change working directory if specified:
    std::string dir;
    if ( glos.set(dir, "directory") )
    {
        FilePath::change_dir(dir);
        //std::clog << "Cytosim working directory is " << FilePath::get_cwd() << '\n';
    }

    glApp::build();
    
    if ( glos.use_key("parameters") )
    {
        writePlayParameters(std::cout, false);
        return EXIT_SUCCESS;
    }

    try
    {
        simul.initialize(glos);
    }
    catch( Exception & e )
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    // secondary configuration file to adjust display parameters:
    std::string setup = goLive ? simul.prop->config : simul.prop->property_file;
    View& view = glApp::views[0];

    try
    {
        // obtain setup from the command line:
        glos.set(setup, "setup");
        glos.set(setup, ".cms");
        
        //std::cerr << "cytosim graphical setup file: " << setup << std::endl;
        
        // extract first specification of "simul:display" string from the setup file
        Parser(simul, 0, 0, 0, 0, 0).readConfig(setup);
        
        // settings in the file are read, but do not overwrite the command-line options:
        glos.read(simul.prop->display, 1);
        simul.prop->display_fresh = false;
        
        // read parameters:
        GP.read(glos);
        view.read(glos);
        DP.read(glos);
        PP.read(glos);
    }
    catch( Exception & e )
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    /*
     At exit, exit_handler() will kill the slave thread, 
     before any object is destroyed to prevent the slave thread
     from accessing corrupted data.
     */
    atexit(exit_handler);
    
    //---------start live simulation or open trajectory file
    
    if ( goLive )
    {
        try
        {
            Parser(simul, 0, 0, 0, 0, 0).readConfig(setup);
            
            //set PP.live to drive the timer in play:
            PP.live = 1;
            simThread.period(PP.period);
            
            if ( simThread.start() )
            {
                std::cerr << "Error: could not start live thread" << std::endl;
                return EXIT_FAILURE;
            }
        }
        catch( Exception & e ) {
            std::cerr << "\nError: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }        
    }
    else
    {
        try
        {
            Parser(simul, 1, 1, 0, 0, 0).readConfig(simul.prop->property_file);
            
            if ( simul.prop->property_file != setup )
                Parser(simul, 0, 0, 0, 0, 0).readConfig(setup);
            
            simThread.openFile();
            
            // load requested frame from trajectory file:
            if ( simThread.readFrame(PP.frame) )
            {
                // if eof happened, load last frame in file:
                readFrame(-1);
                std::cerr << "Warning: found only frame " << PP.frame << std::endl;                
            }
        }
        catch( Exception & e ) {
            std::cerr << "\nError: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }
    
#ifndef __APPLE__
    // it is necessary under Linux/Windows to initialize GLUT to display fonts
    glutInit(&argc, argv);
#endif

    //-------- off-screen non interactive rendering -------
    
    if ( off_screen ) 
    {
        if ( OffScreen::open(view.window_size[0], view.window_size[1]) )
        {
            std::cerr << "Failed to create off-screen pixels\n";
            return EXIT_FAILURE;
        }
       
        gle::initialize();
        Player::initStyle(PP.style);

        unsigned s = 0;
        if ( off_screen == OFFSCREEN_IMAGE )
        {
            // it is possible to specify multiple frame indices:
            do {
                if ( PP.frame != simThread.frame() )
                    simThread.readFrame(PP.frame);
                if ( magnification > 1 )
                {
                    Player::saveMagnifiedImage(magnification, PP.frame);
                }
                else
                {
                    Player::displayScene();
                    Player::saveImage(PP.frame);
                }
                ++s;
            } while ( glos.set(PP.frame, "frame", s) );
        }
        else if ( off_screen == OFFSCREEN_MOVIE )
        {
            // save every PP.period
            s = PP.period;
            do {
                if ( ++s >= PP.period )
                {
                    Player::displayScene();
                    Player::saveImage(simThread.frame());
                    s = 0;
                }
            } while ( 0 == simThread.nextFrame() );
        }
        
        OffScreen::close();
        glos.warnings(std::cerr);
        return EXIT_SUCCESS;
    }
    
    glos.warnings(std::cerr);

    //--------- initialize and open window

#ifdef __APPLE__
    glutInit(&argc, argv);
#endif
    
    glApp::init(Player::displayLive, DIM);
    
    //-------- call GLUT to open a window & start playing -------

    try
    {
        //link the Function-Keys to the user controls
        glApp::bindFunctionKeys(functionKey);
        glApp::actionFunc(processMouseAction);
        glApp::actionFunc(processMouseActionMotion);

        gle::initialize();
        Player::initStyle(PP.style);
        Player::buildMenus();
        
        //register all the GLUT callback functions:
        glutKeyboardFunc(Player::processNormalKey);
        glutEntryFunc(Player::processMouseEntry);
        
        //register a first timer callback:
        glutTimerFunc(100, Player::timer, 0);
    }
    catch ( Exception & e )
    {
        printf("Initialization error: %s\n", e.what());
        return EXIT_FAILURE;
    }
    
    //start the GLUT event handler:
    glutMainLoop();

    return EXIT_SUCCESS;
}
