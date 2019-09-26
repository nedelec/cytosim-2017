// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

using namespace Player;
using glApp::clearMenu;

//------------------------------------------------------------------------------
void processMenuFiber(int item)
{
    if ( FDisp )
    {
        FiberDisp *& FD = FDisp;
        switch (item)
        {
            case 0:   break;
            case 1:   FD->line_style   = FD->line_style?0:1;      break;
            case 2:   FD->line_style   = FD->line_style==2?0:2;   break;
                
            case 3:   FD->point_style  = !FD->point_style;        break;
            case 5:   FD->point_style  = FD->point_style==2?0:2;  break;
                
            case 7:   FD->end_style[1] = 3*!FD->end_style[1];     break;
            case 8:   FD->end_style[0] = 2*!FD->end_style[0];     break;
                
            case 9:   FD->forces       = FD->forces?0:0.1;        break;
            case 10:  FD->visible      = !FD->visible;            break;
                
            case 20:  FD->coloring = FiberDisp::COLORING_NONE;      break;
            case 21:  FD->coloring = FiberDisp::COLORING_NUMBER;    break;
            case 22:  FD->coloring = FiberDisp::COLORING_MARK;      break;
            case 23:  FD->coloring = FiberDisp::COLORING_FLECK;     break;
            case 24:  FD->coloring = FiberDisp::COLORING_DIRECTION; break;
                
            case 30:  FD->show_average = 0;  break;
            case 31:  FD->show_average = 1;  break;
            case 32:  FD->show_average = 2;  break;
                
            default:
                std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
                return;
        }
        glApp::postRedisplay();
    }
    buildMenus();
}


int buildMenuFiber()
{
    static int menuID = 0;
    if ( menuID == 0 )
        menuID = glutCreateMenu(processMenuFiber);
    else
        clearMenu(menuID);
    
    if ( FDisp )
    {
        FiberDisp *& FD = FDisp;
        //check that FD is a valid pointer:
        assert_true( dproperties.find_index(FD) >= 0 );
        glutAddMenuEntry(FD->visible        ? "Hide"             :"Show",             10);
        glutAddMenuEntry(FD->line_style     ? "Hide Lines"       :"Show Lines",        1);
        glutAddMenuEntry(FD->line_style==2  ? "Hide Tensions"    :"Show Tensions",     2);
        glutAddMenuEntry(FD->point_style    ? "Hide Points"      :"Show Points",       3);
        glutAddMenuEntry(FD->point_style==2 ? "Hide Arrows"      :"Show Arrows",       5);
        glutAddMenuEntry(FD->end_style[1]   ? "Hide Minus-ends"  :"Show Minus-end",    7);
        glutAddMenuEntry(FD->end_style[0]   ? "Hide Plus-ends"   :"Show Plus-end",     8);
        glutAddMenuEntry(FD->forces!=0      ? "Hide Point-forces":"Show Point-Forces", 9);
        glutAddMenuEntry("No coloring",           20);
        glutAddMenuEntry("Coloring by number",    21);
        glutAddMenuEntry("Coloring by mark",      22);
        glutAddMenuEntry("Coloring by cluster",   23);
        glutAddMenuEntry("Coloring by direction", 24);
        glutAddMenuEntry("show_average=0", 30);
        glutAddMenuEntry("show_average=1", 31);
        glutAddMenuEntry("show_average=2", 32);
    }
    else
        glutAddMenuEntry("no fiber?", 0);

    return menuID;
}

//------------------------------------------------------------------------------
void processMenuCouple(int item)
{
    switch (item)
    {
        case 0:  return;
        case 1:  DP.couple_select = 0;  break;
        case 2:  DP.couple_select = 1;  break;
        case 3:  DP.couple_select = 2;  break;
        case 4:  DP.couple_select = 4;  break;
        default:
            std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
            return;
    }
    buildMenus();
    glApp::postRedisplay();
}

int buildMenuCouple()
{
    static int menuID = 0;
    if ( menuID == 0 )
        menuID = glutCreateMenu(processMenuCouple);
    else
        clearMenu(menuID);
    
    glutAddMenuEntry("Hide all",    1);
    glutAddMenuEntry("Show free",   2);
    glutAddMenuEntry("Show bound",  3);
    glutAddMenuEntry("Show bridge", 4);
    return menuID;
}

//------------------------------------------------------------------------------
void processMenuBead(int item)
{
    Property * pp = dproperties.find_next("bead:display", 0);
    if ( pp == 0 )
        pp = dproperties.find_next("sphere:display", 0);
    if ( pp )
    {
        int & style = static_cast<PointDisp*>(pp)->style;
        switch (item)
        {
            case 0:  break;
            case 1:  style ^= 1;  break;
            case 2:  style ^= 2;  break;
            case 4:  style ^= 4;  break;
            case 8:  style ^= 8;  break;
            case 9:  style  = 0;  break;
            default:
                std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
                return;
        }
        glApp::postRedisplay();
    }
    buildMenus();
}

int buildMenuBead()
{
    static int menuID = 0;
    if ( menuID == 0 )
        menuID = glutCreateMenu(processMenuBead);
    else
        clearMenu(menuID);
    
    Property * pp = dproperties.find_next("bead:display", 0);
    if ( pp == 0 )
        pp = dproperties.find_next("sphere:display", 0);
    if ( pp )
    {
        int & style = static_cast<PointDisp*>(pp)->style;
        glutAddMenuEntry(style&1?"Hide points":"Show points", 1);
        glutAddMenuEntry(style&2?"Hide spheres":"Show spheres", 2);
        glutAddMenuEntry(style&4?"Hide outlines":"Show outlines", 4);
        glutAddMenuEntry(style&8?"Hide decoration":"Add decorations", 8);
        glutAddMenuEntry("Hide All", 9);
    }
    else
        glutAddMenuEntry("no beads?", 0);
    return menuID;
}



//------------------------------------------------------------------------------
void processMenuDisplay(int item)
{
    View & view = glApp::currentView();
    switch (item)
    {
        case 0:   return;
        case 1:   view.reset();                            break;
        case 3:   DP.tiled = !DP.tiled;                    break;
        case 4:   glApp::switchFullScreen();               break;
        case 6:   view.track_fibers = !view.track_fibers;  break;
        case 7:   DP.point_value=(DP.point_value?0:0.01);  break;
        
        case 101: initStyle(1);  break;
        case 102: initStyle(2);  break;
        case 103: initStyle(3);  break;
            
        default:
            std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
            return;
    }
    buildMenus();
    glApp::postRedisplay();
}


int buildMenuStyle()
{
    static int menuID = 0;
    if ( menuID == 0 )
    {
        menuID = glutCreateMenu(processMenuDisplay);
        glutAddMenuEntry("Wireframe (style 1)", 101);
        glutAddMenuEntry("Wireframe (style 2)", 102);
        glutAddMenuEntry("Lighting  (style 3)", 103);
    }
    return menuID;
}


int buildMenuDisplay()
{
    static int menuID = 0;
    int m0 = buildMenuStyle();
    int m1 = buildMenuFiber();
    int m2 = buildMenuCouple();
    int m3 = buildMenuBead();
    
    if ( menuID == 0 )
        menuID = glutCreateMenu(processMenuDisplay);
    else
        clearMenu(menuID);
    
    glutAddMenuEntry("Reset View",  1);
    glutAddSubMenu("Style",   m0);
    glutAddSubMenu("Fibers",  m1);
    glutAddSubMenu("Couple",  m2);
    glutAddSubMenu("Beads",   m3);
    
    View & view = glApp::currentView();
    glutAddMenuEntry(GP.full_screen?"Exit Fullscreen (f)":"Fullscreen (f)", 4);
    glutAddMenuEntry(DP.point_value?"Set point_value = 1 pixel":"Set point_value = 10 nm", 7);
    glutAddMenuEntry(DP.tiled?"Non-tiled Display":"Tiled Display", 3);
    glutAddMenuEntry(view.track_fibers?"stop tracking":"Track Fibers", 6);
    
    return menuID;
}



//------------------------------------------------------------------------------
#pragma mark -

void processMenuFiberSelect(int item)
{
    if ( FDisp == 0 )
        return;
        
    switch (item)
    {
        case 0:  return;
        case 1:  FDisp->exclude  = 0;   break;
        case 2:  FDisp->exclude ^= 1;   break;
        case 3:  FDisp->exclude ^= 2;   break;
        default:
            std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
            return;
    }
    buildMenus();
    glApp::postRedisplay();
}

int buildMenuFiberSelect()
{
    static int menuID = 0;
    if ( menuID == 0 )
        menuID = glutCreateMenu(processMenuFiberSelect);
    else
        clearMenu(menuID);
    
    glutAddMenuEntry("Hide All", 1);
    if ( FDisp )
    {
        glutAddMenuEntry(FDisp->exclude&1?"Show right pointing":"Hide right pointing", 2);
        glutAddMenuEntry(FDisp->exclude&2?"Show left pointing":"Hide left pointing", 3);
    }
    return menuID;
}


//------------------------------------------------------------------------------
void processMenuCoupleSelect(int item)
{
    switch (item)
    {
        case 0:  return;
        case 1:  DP.couple_select  = 0;   break;
        case 2:  DP.couple_select ^= 1;   break;
        case 3:  DP.couple_select ^= 2;   break;
        case 4:  DP.couple_select ^= 4;   break;
        default:
            std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
            return;
    }
    buildMenus();
    glApp::postRedisplay();
}

int buildMenuCoupleSelect()
{
    static int menuID = 0;
    if ( menuID == 0 )
        menuID = glutCreateMenu(processMenuCoupleSelect);
    else
        clearMenu(menuID);
    
    glutAddMenuEntry("Hide All", 1);
    glutAddMenuEntry(DP.couple_select&1?"Hide Free":"Show Free",     2);
    glutAddMenuEntry(DP.couple_select&2?"Hide Bound":"Show Bound",   3);
    glutAddMenuEntry(DP.couple_select&4?"Hide Bridge":"Show Bridge", 4);
    return menuID;
}

//------------------------------------------------------------------------------
void processMenuSingleSelect(int item)
{
    switch (item)
    {
        case 0:  return;
        case 1:  DP.single_select  = 0;   break;
        case 2:  DP.single_select ^= 1;   break;
        case 3:  DP.single_select ^= 2;   break;
        
        default:
            std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
            return;
    }
    buildMenus();
    glApp::postRedisplay();
}

int buildMenuSingleSelect()
{
    static int menuID = 0;
    if ( menuID == 0 )
        menuID = glutCreateMenu(processMenuSingleSelect);
    else
        clearMenu(menuID);
    
    glutAddMenuEntry("Hide All",     1);
    glutAddMenuEntry(DP.single_select&1?"Hide Free":"Show Free",     2);
    glutAddMenuEntry(DP.single_select&2?"Hide Bridge":"Show Bridge", 3);
    return menuID;
}

int buildSubMenu8()
{
    static int menuID = 0;
    if ( menuID == 0 ) {
        menuID = glutCreateMenu(processMenuSingleSelect);
        glutAddMenuEntry("-", 0);
    }
    return menuID;
}


//------------------------------------------------------------------------------

int buildMenuSelect()
{
    static int menuID = 0;
    int m1 = buildMenuFiberSelect();
    int m2 = buildMenuCoupleSelect();
    int m3 = buildMenuSingleSelect();
    
    if ( menuID == 0 )
        menuID = glutCreateMenu(0);
    else
        clearMenu(menuID);

    glutAddSubMenu("Fibers",  m1);
    glutAddSubMenu("Couple",  m2);
    glutAddSubMenu("Singles", m3);
    
    return menuID;
}


//------------------------------------------------------------------------------
#pragma mark -

void processMenuAnimation(int item)
{
    switch (item)
    {
        case 0:  return;
        case 1:  Player::processNormalKey('z');   break;
        case 2:  Player::processNormalKey('a');   break;
        case 4:  Player::processNormalKey('s');   break;
        case 5:  Player::processNormalKey('r');   break;
        default:
            std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
            return;
    }
    glApp::postRedisplay();
}

int buildMenuAnimation()
{
    static int menuID = 0;
    
    if ( menuID == 0 )
    {
        menuID = glutCreateMenu(processMenuAnimation);
        glutAddMenuEntry("(z) New State",        1);
        glutAddMenuEntry("(a) Start Live",       2);
        glutAddMenuEntry("(s) One Step & Stop",  4);
        glutAddMenuEntry("(r) Read Parameters",  5);
    }
    return menuID;
}



//------------------------------------------------------------------------------

void processMenuReplay(int item)
{
    switch (item)
    {
        case 0:  return;
        case 1:  Player::processNormalKey('p');  break;
        case 2:  Player::processNormalKey('o');  break;
        case 3:  Player::processNormalKey('s');  break;
        case 4:  Player::processNormalKey('z');  break;
        case 5:  previousFrame();         break;
        case 6:  nextFrame();             break;
        case 7:  PP.loop = !PP.loop;      break;
        default:
            std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
            return;
    }
    buildMenus();
    glApp::postRedisplay();
}

int buildMenuReplay()
{
    static int menuID = 0;
    
    if ( menuID == 0 )
    {
        menuID = glutCreateMenu(processMenuReplay);
        glutAddMenuEntry("(p) Play/Faster",       1);
        glutAddMenuEntry("(o) Slower",            2);
        glutAddMenuEntry("(s) Stop",              3);
        glutAddMenuEntry("-",                     0);
        glutAddMenuEntry("(z) First Frame",       4);
        glutAddMenuEntry("(<) Previous Frame",    5);
        glutAddMenuEntry("(>) Next Frame",        6);
        glutAddMenuEntry(PP.loop?"Do not loop":"Loop", 7);
    }
    return menuID;
}


//------------------------------------------------------------------------------
void processMenuExport(int item)
{
    switch (item)
    {
        case 0:   return;
        case 1:   saveImage(PP.image_index++);  return;
        case 2:   PP.play = PLAY_FORWARD_WRITE;  return;
        
        case 3:   saveMagnifiedImage(2, PP.poster_index++);   return;
        case 4:   saveMagnifiedImage(3, PP.poster_index++);   return;
        case 5:   saveMagnifiedImage(5, PP.poster_index++);   return;
        
        case 10:  PP.image_format="png";  flashText("PNG");   break;
        case 11:  PP.image_format="ppm";  flashText("PPM");   break;
        case 15:  PP.image_index = 0;                         return;
        
        case 20:  writePlayParameters(std::cout, true);       return;
        case 21:  writeDisplayParameters(std::cout, true);    return;
        case 22:  simThread.writeProperties(std::cout, true); return;
        case 23:  simThread.writeState();                     return;
            
        default:
            std::cerr << "CYTOSIM ERROR: unknown menu code" << item << std::endl;
            return;
    }
    buildMenus();
    glApp::postRedisplay();
}


int buildMenuExport()
{
    static int menuID = 0;
    if ( menuID == 0 )
        menuID = glutCreateMenu(processMenuExport);
    else
        clearMenu(menuID);
    
    glutAddMenuEntry("Save Image to File (y)",    1);
    glutAddMenuEntry("Save Movie to Files (Y)",   2);
    glutAddMenuEntry("Save 2x Magnified Poster",  3);
    glutAddMenuEntry("Save 3x Magnified Poster",  4);
    glutAddMenuEntry("Save 5x Magnified Poster",  5);

    if ( PP.image_format == "ppm" )
    {
        if ( SaveImage::supported("png") )
            glutAddMenuEntry("Use PNG Format", 10);
        else
            glutAddMenuEntry("Use PPM Format", 11);
    }
    else
        glutAddMenuEntry("Use PPM Format", 11);

    glutAddMenuEntry("Reset Image-file Index",   15);
    glutAddMenuEntry("-",                         0);
    glutAddMenuEntry("Write Play Parameters",    20);
    glutAddMenuEntry("Write Display Parameters", 21);
    glutAddMenuEntry("Write Object Properties",  22);
    glutAddMenuEntry("Write State",              23);
    
    return menuID;
}

//------------------------------------------------------------------------------
//                    MAIN MENU
//------------------------------------------------------------------------------
#pragma mark -

void processMenu(int item)
{
    switch( item )
    {
        case 0: GP.show_message = 0; break;
        case 1: GP.show_message = 1; break;
        case 2: GP.show_message = 2; break;
        case 3: GP.show_message = 3; break;
        case 4: GP.show_message = 4; break;
        case 9: exit(EXIT_SUCCESS);
        default:
            printf("unknown item %i\n", item);
    }
    GP.message = buildMessage(GP.show_message);
}

void Player::buildMenus()
{
    setPointers(0);
    
    static int menuID = 0;
    int m1 = buildMenuDisplay();
    int m2 = buildMenuSelect();
    int m3 = buildMenuAnimation();
    int m4 = buildMenuReplay();
    int m6 = buildMenuExport();
    int mG = glApp::buildMenu();

    if ( menuID == 0 )
        menuID = glutCreateMenu(processMenu);
    else
        clearMenu(menuID);
    
    glutAddMenuEntry("Help",             3);
    glutAddSubMenu("Display",           m1);
    glutAddSubMenu("Object-Selection",  m2);
    glutAddSubMenu("Live-Simulation",   m3);
    glutAddSubMenu("File-Replay",       m4);
    glutAddSubMenu("Export",            m6);
    glutAddSubMenu("More",              mG);
    glutAddMenuEntry("Quit",             9);
    
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

