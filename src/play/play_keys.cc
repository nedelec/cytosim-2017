// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "fiber_prop.h"
#include "fiber_disp.h"


/**
 It is possible to save images or many images by pressing a key..
 disabling this capacity in public distribution might be safer:
 It can be done by disabling ALLOW_WRITING below:
 */
#define ALLOW_WRITING

//------------------------------------------------------------------------------
/**
 Change size (add inc) for all PointDisp.
 Also increase PointDisp:width, while keeping the same ratio width/size
*/
void changePointDispSize(const PropertyList& plist, DisplayProp& DP, real inc)
{
    for ( unsigned int n = 0; n < plist.size(); ++n )
    {
        PointDisp * disp = static_cast<PointDisp*>(plist[n]);
        real s = inc * ( 1 + round(disp->size/inc) );
        if ( s > 0 )
            disp->size = s;
    }
    
    // also change the global value:
    real s = inc * ( 1 + round(DP.point_size/inc) );
    if ( s > 0 )
        DP.point_size = s;
}

/**
 Change size (add inc) for all PointDisp.
 Also increase PointDisp:width, while keeping the same ratio width/size
 */
void changePointDispWidth(const PropertyList& plist, DisplayProp& DP, real inc)
{
    for ( unsigned int n = 0; n < plist.size(); ++n )
    {
        PointDisp * disp = static_cast<PointDisp*>(plist[n]);
        real s = disp->width + 0.5 * inc;
        if ( s > 0 )
               disp->width = s;
    }
    
    // also change the global value:
    real s = DP.line_width + 0.5 * inc;
    if ( s > 0 )
       DP.line_width = s;
}


void changePointDispStyle(PointDisp* pdisp)
{
    if ( pdisp == 0 )
        return;

    int & style = pdisp->style;
    style = ( style + 1 ) % 8;
    switch( style )
    {
        case 0: flashText("Beads/Sphere:style=0: invisible"); break;
        case 1: flashText("Beads/Sphere:style=1: points");    break;
        case 2: flashText("Beads/Sphere:style=2: surface");   break;
        case 4: flashText("Beads/Sphere:style=4: ring");      break;
    }
}

void changePointDispStyle(const PropertyList& plist)
{
    for ( unsigned int n = 0; n < plist.size(); ++n )
        changePointDispStyle(static_cast<PointDisp*>(plist[n]));
}


#pragma mark -

void changeExclude(FiberDisp* fd, bool mod)
{
    if ( mod )
        fd->exclude >>= 2;
    fd->exclude = ( fd->exclude + 1 ) % 4;
    if ( mod )
        fd->exclude <<= 2;
    
    switch( fd->exclude )
    {
        case 0: flashText("All fibers");                break;
        case 1: flashText("Right-pointing fibers");     break;
        case 2: flashText("Left-pointing fibers");      break;
        case 3: flashText("No fibers");                 break;
        case 4: flashText("Counter-clockwise fibers");  break;
        case 8: flashText("Clockwise fibers");          break;
        case 12: flashText("No fibers");                break;
    }
}


void changeExplode(FiberDisp* fd)
{
    fd->explode = ( fd->explode + 1 ) % 3;
    flashText("Fiber:explode = %i", fd->explode);
}


void changeColoring(FiberDisp* fd)
{
    fd->coloring = ( fd->coloring + 1 ) % 5;
    switch( fd->coloring )
    {
        case FiberDisp::COLORING_NONE:      flashText("Fibers: no coloring");           break;
        case FiberDisp::COLORING_NUMBER:    flashText("Fibers: coloring by number");    break;
        case FiberDisp::COLORING_DIRECTION: flashText("Fibers: coloring by direction"); break;
        case FiberDisp::COLORING_MARK:      flashText("Fibers: coloring by mark");      break;
        case FiberDisp::COLORING_FLECK:     flashText("Fibers: coloring by cluster");   break;
    }
}


void changeMask(FiberDisp* fd)
{
    if ( fd->mask < 2 )
        fd->mask = 2;
    else
        fd->mask = ( fd->mask * 2 ) % 1024;
    if ( fd->mask )
        fd->phase &= fd->mask;
    flashText("fiber:mask=%i, %i", fd->mask, fd->phase);
}


void changePhase(FiberDisp* fd)
{
    fd->phase = ( fd->phase + 1 );
    if ( fd->mask )
        fd->phase %= fd->mask;
    flashText("fiber:mask=%i, %i", fd->mask, fd->phase);
}


void changePointStyle(FiberDisp* fd)
{
    fd->point_style = ( fd->point_style + 1 ) % 3;
    switch ( fd->point_style )
    {
        case 0:  flashText("Fiber points: invisible");   break;
        case 1:  flashText("Fibers: model points");      break;
        case 2:  flashText("Fibers: arrowheads");        break;
        case 3:  flashText("Fibers: abscissa");          break;
    }
}


void changeLineStyle(FiberDisp* fd)
{
    fd->line_style = ( fd->line_style + 1 ) % 3;
    switch ( fd->line_style )
    {
        case 0:  flashText("Fibers lines: invisible");   break;
        case 1:  flashText("Fibers: lines");             break;
        case 2:  flashText("Fibers: axial tensions");    break;
        case 3:  flashText("Fibers: color by angle");    break;
        case 4:  flashText("Fibers: polarity ratchets"); break;
    }
}


void changeSpeckleStyle(FiberDisp* fd)
{
    fd->speckle_style = ( fd->speckle_style + 1 ) % 3;
    switch ( fd->speckle_style )
    {
        case 0:  flashText("Fibers: no speckles");       break;
        case 1:  flashText("Fibers: random speckles");   break;
        case 2:  flashText("Fibers: regular speckles");  break;
    }
}


void changePointSize(FiberDisp* fd, real inc)
{
    int s = inc * ( 1 + round( fd->point_size / inc ) );
    
    if ( s > 0 )
        fd->point_size = s;
    
    flashText("%s:point_size=%0.2f", fd->name().c_str(), fd->point_size);
}


void changeLineWidth(FiberDisp* fd, real inc)
{
    real s = fd->line_width + 0.5 * inc;
    
    if ( s > 0 )
    {
        if ( fd->line_width > 0 )
        {
            real scale = s / fd->line_width;
            fd->point_size  *= scale;
            fd->end_size[0] *= scale;
            fd->end_size[1] *= scale;
        }
        fd->line_width = s;
        flashText("Fibers: line_width %0.2f", s);
    }
}

//------------------------- Fibers ends ------------------------------

void changeTipStyle(FiberDisp* fd)
{
    int * style = fd->end_style;
    // showing the plus ends -> the minus ends -> both -> none
    switch( (style[1]?1:0) + (style[0]?2:0) )
    {
        case 0:
            style[0] = 2;
            style[1] = 0;
            break;
        case 1:
            style[0] = 0;
            style[1] = 0;
            break;
        case 2:
            style[0] = 2;
            style[1] = 4;
            break;
        case 3:
        default:
            style[0] = 0;
            style[1] = 4;
            break;
    }

    switch( (style[0]?1:0) + (style[1]?2:0) )
    {
        case 0: flashText("Fibers: no ends");    break;
        case 1: flashText("Fibers: plus-ends");  break;
        case 2: flashText("Fibers: minus-ends"); break;
        case 3: flashText("Fibers: both ends");  break;
    }
}


void changeTipSize(FiberDisp* fd, real inc)
{
    real * size = fd->end_size;
    if ( size[0] + 2*inc > 0 ) size[0] += 2*inc;
    if ( size[1] + inc   > 0 ) size[1] += inc;
    flashText("Fibers: end_size %.1f %.1f", size[0], size[1]);
}


//------------------------ Select ------------------------------
#pragma mark -


void changeSingleSelect(DisplayProp& DP)
{
    unsigned int & select = DP.single_select;
    switch( select )
    {
        case 3:   select = 0;    flashText("Singles:select=0: invisible");   break;
        case 0:   select = 2;    flashText("Singles:select=2: only bound");  break;
//        case 2:   select = 1;    flashText("Singles:select=1: only free");   break;
        default:  select = 3;    flashText("Singles:select=3: all");         break;
    }
}

void changeCoupleSelect(DisplayProp& DP)
{
    unsigned int & select = DP.couple_select;
    switch( select )
    {
        case 7:   select = 0;    flashText("Couples:select=0: invisible");           break;
        case 0:   select = 2;    flashText("Couples:select=2: only bound");          break;
        case 2:   select = 4;    flashText("Couples:select=4: only bridge");         break;
        case 4:   select = 12;   flashText("Couples:select=12: only anti-parallel"); break;
//        case 12:  select = 1;    flashText("Couples:select=1: only free");           break;
        default:  select = 7;    flashText("Couples:select=7: all");                 break;
    }
}


//------------------------------------------------------------------------------
//---------------------------- keyboard commands -------------------------------
//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Keyboard Commands

void Player::help(std::ostream & os)
{
    os << "--------------------------Keyboard Commands-----------------------\n";
    os << "Fibers:\n";
    os << "         `        Select another type of fibers for modifications\n";
    os << "         1        Change display: line / color-coded forces / hide\n";
    os << "         3 4      Decrease; increase line width (ALT: point size)\n";
    os << "         !        Change tip display: none / plus / both / minus\n";
    os << "         # $      Decrease; increase fiber tip display size\n";
    os << "         2        Change speckle display: random / regular / off)\n";
    os << "         c d w    Coloring, Right/left-pointing, Fractional masking\n";
    os << "         t T      Auto-tracking nematic, Auto-tracking polar\n";
    os << "Beads / Solids / Spheres:\n";
    os << "         5        Switch between different bead/sphere display style\n";
    os << "         %        Change first bead/solid display style\n";
    os << "         * (      Decrease; increase point size\n";
    os << "Singles / Couples:\n";
    os << "         8 *      Change Couple; Single selection mode\n";
    os << "         9 0      Decrease; Increase point size\n";
    os << "         ( )      Decrease; Increase line width\n";
    os << "----------------------------Animation-----------------------------\n";
    os << "         < >      Show previous / next frame ( , . also works)\n";
    os << "         u i o p  Play reverse; stop; slower; play/faster\n";
    os << "         z Z      Reset simulation; Rewind movie to first frame\n";
    os << "         space F  Reset view; adjust window to screen width\n";
    os << "         escape   Quit\n";
    os << "----------------------------Simulation----------------------------\n";
    os << "         a A      Start live mode; double nb-steps/display\n";
    os << "         s S      Step simulation engine; set nb-steps/display = 1\n";
    os << "         z Z      Reset simulation; create new initial state\n";
    os << "         g G      Delete mouse-hands; Creat new Hand for mouse\n";
#ifdef ALLOW_WRITING
    os << "---------------------------Input/Output---------------------------\n";
    os << "         r        Read parameter file and update simulation\n";
    os << "         R        Write display parameters to terminal\n";
    os << "         y Y      Save displayed image; Play and save all images\n";
#endif
}


//------------------------------------------------------------------------------
void Player::processNormalKey(const unsigned char key, const int x, const int y)
{
    //std::cerr << "processing key `" << key << "'\n";
    
    setPointers(0);
    // the view associated with the current window
    View & view = glApp::currentView();

    // execute the custom piece of code (magic_key / magic_code)
    for ( int k = 0; k < PlayProp::NB_MAGIC_KEYS; ++k )
    {
        if ( key == PP.magic_key[k] )
        {
            std::istringstream iss(PP.magic_code[k]);
            simThread.execute(iss);
            glApp::postRedisplay();
            return;
        }
    }
    
    /* 
     In the switch below:
     - use break if the display need to be refreshed,
     - otherwise, use return.
    */
    switch (key)
    {
        
        case 'h':
        {
            GP.show_message = ( GP.show_message + 1 ) % 6;
            GP.message = buildMessage(GP.show_message);
        } break;
            
        
        //------------------------- live simulation mode:
        
        case 'r': {
            try {
                simThread.reloadConfig();
                flashText("Parameters reloaded");
            }
            catch( Exception & e ) {
                flashText("Error in config: %s", e.what());
                PP.live = 0;
            }
        } break;

        case 'R':
        {
            if ( glutGetModifiers() & GLUT_ACTIVE_ALT )
                simThread.writeProperties(std::cout, true);
            else
            {
                writePlayParameters(std::cout, true);
                writeDisplayParameters(std::cout, true);
            }
        } break;
            
#ifdef ALLOW_WRITING

        case 'y':
            // save image
            displayScene();
            saveImage(PP.image_index++);
            return;
            
        case 'Y':
            //will save all frames (cf timer function).
            PP.play = PLAY_FORWARD_WRITE;
            break;
        
#endif
        case 'F':
            widenDisplay();
            //view.reset();
            break;
            
        case ' ':
            if ( glutGetModifiers() & GLUT_ACTIVE_ALT )
            {
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                view.reset();
                flashText("");
                break;
            }
            startstop();
            break;
            
        case 'z':
            if ( simThread.goodFile() )
                rewind();
            else
                restart();
            break;
            
        case 'Z':
            simThread.cancel();
            PP.live = 0;
            break;
            
        case 'a':
            if ( glutGetModifiers() & GLUT_ACTIVE_ALT )
            {
                initStyle(1);
                flashText("Style 1");
                break;
            }
            if ( 0 == simThread.persist() )
                flashText("Extend simulation...");
            PP.live = 1;
            break;
            
        case 'A':
            PP.period = 2 * PP.period;
            simThread.period(PP.period);
            break;
            
        case 's':
            if ( glutGetModifiers() & GLUT_ACTIVE_ALT )
            {
                initStyle(2);
                flashText("Style 2");
                break;
            }
            step();
            break;
                        
        case 'S':
            PP.period = 1;
            simThread.period(PP.period);
            break;
            
        case 'G':
            simThread.releaseHandle();
            break;
            
        case 'g':
            simThread.deleteHandles();
            flashText("Deleted mouse-controled handles");
            break;
            
        //------------------------- play / stop / reverse:
             
        case '<':
        case ',':
            if ( PP.play == PLAY_FORWARD )
                PP.play = PLAY_STOP;
            else
                previousFrame();
            glApp::postRedisplay();
            return;
            
        case '>':
        case '.':
            if ( PP.play == PLAY_REVERSE )
                PP.play = PLAY_STOP;
            else
                nextFrame();
            glApp::postRedisplay();
            return;
            
        case 'u':
            startBackward();
            return;
             
        case 'i':
            PP.toggleReport(glutGetModifiers() & GLUT_ACTIVE_ALT);
            break;
            
        case 'o':
            if ( PP.delay < 1 << 13 )
                PP.delay *= 2;
            flashText("Delay %i ms", PP.delay);
            return;
            
        case 'p':
            startForward();
            return;
                        
        //------------------------------ Fibers ------------------------------
            
        case '`': {
            setPointers(1);
        } break;
            
        case 't':
            view.track_fibers = ( view.track_fibers ? 0 : 3 );
            flashText("view.track_fibers = %i (nematic)", view.track_fibers);
            break;
            
        case 'T':
            view.track_fibers = ( view.track_fibers ? 0 : 2 );
            flashText("view.track_fibers = %i (vectorial)", view.track_fibers);
            break;
            
        case 'd':
            if ( glutGetModifiers() & GLUT_ACTIVE_ALT )
            {
                initStyle(3);
                flashText("Style 3");
                break;
            }
            if ( FDisp )
                changeExclude(FDisp, 0);
            break;
            
        case 'e':
            if ( FDisp )
                changeExplode(FDisp);
            break;
                        
        case 'w':
            if ( FDisp )
                changeMask(FDisp);
            break;
            
        case 'c':
            if ( FDisp )
                changeColoring(FDisp);
            break;
            
        case 'W':
        case 'C':
            if ( FDisp )
                changePhase(FDisp);
            break;
            
        case '1':
            if ( FDisp )
            {
                if ( glutGetModifiers() & GLUT_ACTIVE_ALT ) 
                    changePointStyle(FDisp);
                else 
                    changeLineStyle(FDisp);
            }
            break;
            
        case '2':
            if ( FDisp )
                changeSpeckleStyle(FDisp);
            break;
            
        case '3':
            if ( FDisp ) 
            {
                if ( glutGetModifiers() & GLUT_ACTIVE_ALT )
                    changePointSize(FDisp, -1);
                else
                    changeLineWidth(FDisp, -1);
            }
            break;
            
        case '4':
            if ( FDisp )
            {
                if ( glutGetModifiers() & GLUT_ACTIVE_ALT )
                    changePointSize(FDisp, +1);
                else
                    changeLineWidth(FDisp, +1);
            }
            break;
            
        //------------------------- Fibers tips ------------------------------

        case '!':
            if ( FDisp )
                changeTipStyle(FDisp);
            break;
            
        case '#':
            if ( FDisp )
                changeTipSize(FDisp, -1);
            break;
                
        case '$':
            if ( FDisp )
                changeTipSize(FDisp, +1);
            break;
            
        //------------------------ Solid / Sphere ------------------------------
            
        case '5':
            changePointDispStyle(dproperties.find_all("bead:display", "sphere:display"));
            break;
        
        case '%':
        {
            PointDisp * pd = static_cast<PointDisp*>(dproperties.find_next("bead:display", 0));
            changePointDispStyle(pd);
        }  break;
            
        case '6':
            changePointDispSize(dproperties.find_all("bead:display", "sphere:display"), DP, -1);
            flashText("Point size %.1f", DP.point_size);
            break;
            
        case '7':
            changePointDispSize(dproperties.find_all("bead:display", "sphere:display"), DP, +1);
            flashText("Point size %.1f", DP.point_size);
            break;
            
        case '^':
            changePointDispWidth(dproperties.find_all("bead:display", "sphere:display"), DP, -1);
            flashText("Line width %.1f", DP.line_width);
            break;
            
        case '&':
            changePointDispWidth(dproperties.find_all("bead:display", "sphere:display"), DP, +1);
            flashText("Line width %.1f", DP.line_width);
            break;
            
            
        //------------------------ Select ------------------------------
            
        case '*':
            changeSingleSelect(DP);
            break;
        
        case '8':
            changeCoupleSelect(DP);
            break;
        
        //------------------------ Point-Size ------------------------------

        case '9':
            changePointDispSize(dproperties.find_all("hand:display"), DP, -1);
            flashText("Point size %.1f", DP.point_size);
            break;
            
        case '0':
            changePointDispSize(dproperties.find_all("hand:display"), DP, +1);
            flashText("Point size %.1f", DP.point_size);
            break;
            
        case '(':
            changePointDispWidth(dproperties.find_all("hand:display"), DP, -1);
            flashText("Line width %.1f", DP.line_width);
            break;
            
        case ')':
            changePointDispWidth(dproperties.find_all("hand:display"), DP, +1);
            flashText("Line width %.1f", DP.line_width);
            break;
            
        default:
            glApp::processNormalKey(key);
            return;
    }
    
    // if break was called, redraw the scene:
    glApp::postRedisplay();
    // rebuild the menus, that might have changed:
    buildMenus();
}

