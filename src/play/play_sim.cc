// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "opengl.h"
#include "iowrapper.h"


//------------------------------------------------------------------------------
#pragma mark I/O

void Player::readFrame(int f)
{
    simThread.readFrame(f);
    PP.frame = simThread.frame();
}


void Player::previousFrame()
{
    if ( PP.frame > 0 )
        simThread.readFrame(PP.frame-1);
    else {
        if ( PP.loop )
            simThread.readFrame(-1);
        else
            PP.play = PLAY_STOP;
    }
    PP.frame = simThread.frame();
}

/**
 Reads the next frame from the current file position.
 This will jump over missing frames.
 */
void Player::nextFrame()
{    
    try {
        
        if ( simThread.nextFrame() )
        {
            if ( PP.loop )
                simThread.readFrame(0);
            else
            {
                flashText("end-of-file\n");
                PP.play = PLAY_STOP;
            }
        }
        
        PP.frame = simThread.frame();
    }
    catch( Exception & e )
    {
        flashText("Error:\n %s", e.what());
        if ( simThread.eof() )
            PP.play = PLAY_STOP;
    }
}

//------------------------------------------------------------------------------
#pragma mark Commands

void Player::rewind()
{
    if ( simThread.goodFile() )
    {
        PP.play = PLAY_STOP;
        simThread.readFrame(0);
        glApp::postRedisplay();
    }
}


void Player::startForward()
{
    if ( PP.play!=PLAY_FORWARD  &&  PP.play!=PLAY_FORWARD_WRITE  &&  !PP.live )
    {
        //rewind if the end of the file was reached:
        if ( simThread.eof() )
            simThread.readFrame(0);
        PP.play = PLAY_FORWARD;
    }
    else
    {
        PP.delay /= 2;
        //the delay should be compatible with graphic refresh rates:
        const unsigned int min_delay = 1;
        if ( PP.delay < min_delay )
        {
            PP.delay = min_delay;
            if ( PP.live )
                flashText("Delay is %i ms! use 'A' to jump frames", PP.delay);
            else
                flashText("Delay is %i ms!", PP.delay);
        }
        else {
            flashText("Delay %i ms", PP.delay);
        }
    }
}


void Player::startBackward()
{
    if ( PP.play != PLAY_REVERSE )
    {
        if ( simThread.frame() == 0 )
            readFrame(-1);
        else
            flashText("Play reverse");
        PP.play = PLAY_REVERSE;
    }
    else
    {
        PP.delay /= 2;
        if ( PP.delay < 8 )
            PP.delay = 8;
    }
}


void Player::step()
{
    simThread.release();
    PP.live = 0;
    PP.play = PLAY_STOP;
}


void Player::stop()
{
    glApp::displayFunc(displayLive);
    PP.play = PLAY_STOP;
    PP.live = 0;
}


void Player::restart()
{
    try {
        simThread.stop();
        simThread.clear();
        dproperties.erase();
        FDisp = 0;

        simThread.start();
        PP.live = 1;
    }
    catch( Exception & e ) {
        flashText("Error: %s", e.what());
    }
}


void Player::startstop()
{
    if ( PP.live )
        PP.live = 0;
    else if ( simThread.goodFile() )
    {
        if ( PP.play == PLAY_STOP )
            startForward();
        else
            stop();
    }
    else
        PP.live = 1;
}



//------------------------------------------------------------------------------
#pragma mark -


/**
 try to initialize current FDisp and PDisp
 */
void Player::setPointers(bool next)
{
    Property * val = 0;
    if ( FDisp == 0 )
        val = dproperties.find_next("fiber:display", 0);
    
    if ( next )
    {
        // change FD, allowing access to different FiberDisp
        val = dproperties.find_next("fiber:display", FDisp);
    }
    
    if ( val  &&  val != FDisp )
    {
        FDisp = static_cast<FiberDisp*>(val);
        flashText("Focussing on fibers `%s'", FDisp->name().c_str());
    }
}

/**
 Adjust window size to occupy the full screen width
 */
void Player::widenDisplay()
{
    Vector dim = simThread.extension();
    if ( DIM > 1  &&  dim.XX > 0 )
    {
        const int sw = glutGet(GLUT_SCREEN_WIDTH) - 4;
        const int sh = glutGet(GLUT_SCREEN_HEIGHT) - 50;
        dim /= dim.XX;
        dim.XX = 0;
        int h = sw * dim.norm_inf() + 128;
        if ( h > sh-50 ) h = sw-50;
        glutReshapeWindow(sw, h);
        glutPositionWindow(1, 50);
    }
}


/**
 Write global parameters that control the display:
 - GlappProp
 - DisplayProp
 - PlayProp
 .
 */
void Player::writePlayParameters(std::ostream & os, bool prune)
{
    os << "set simul:display *" << std::endl;
    os << "{" << std::endl;
    View& view = glApp::currentView();
    view.write_diff(os, prune);
    DP.write_diff(os, prune);
    GP.write_diff(os, prune);
    //output parameters for the main view:
    PP.write_diff(os, prune);
    os << "}" << std::endl;
}

/**
 Write all the parameters that control the display:
 - GlappProp
 - DisplayProp
 - PlayProp
 - ObjectDisp
 .
 */
void Player::writeDisplayParameters(std::ostream & os, bool prune)
{
    dproperties.write(os, prune);
}


//------------------------------------------------------------------------------
#pragma mark -

void Player::timer(const int value)
{
    //std::cerr << "Player::timer " << value << std::endl;
    //glApp::postRedisplay();

    if ( PP.live )
    {
        simThread.release();
        glutTimerFunc(PP.delay, timer, 2);
    }
    else
    {        
        switch( PP.play )
        {
            default:
            case PLAY_STOP:
                break;
                
            case PLAY_REVERSE:
                previousFrame();
                glApp::postRedisplay();
                break;
                
            case PLAY_FORWARD:
                nextFrame();
                glApp::postRedisplay();
                break;
                
            case PLAY_FORWARD_WRITE:
                nextFrame();
                displayScene();
                saveImage(simThread.frame());
                glutSwapBuffers();
                break;
        }
        /*
         Register the next timer callback
         in idle mode, we use a long time-interval
         */
        if ( PP.play == PLAY_STOP )
            glutTimerFunc(100, timer, 1);
        else
            glutTimerFunc(PP.delay, timer, 2);
    }
}


