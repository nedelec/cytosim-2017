// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "hand_prop.h"
#include "picket.h"

//------------------------------------------------------------------------------
//---------------------------------  mouse -------------------------------------
//------------------------------------------------------------------------------


/**
 Processes mouse clicks
 */
void Player::processMouseAction(const Vector3& pos3, int)
{
    // distance in pixels where mouse-Hand binds:
    const int pixrad = 5;
    
    const real range = pixrad * glApp::currentView().pixelSize();
    Vector pos(pos3.XX, pos3.YY, pos3.ZZ);

    if ( simThread.selectClosestHandle(pos, range) )
        simThread.moveHandle(pos);
    else
    {
        if ( simThread.handle() )
        {
            simThread.detachHandle();
            simThread.moveHandle(pos);
        }
        else
        {
            Single * s = simThread.createHandle(pos, range);
            assert_true(s);
            HandProp * hp = const_cast<HandProp*>(s->hand()->prop);
            assert_true(hp);
            PointDisp *& pd = hp->disp;
            if ( pd == 0 )
            {
                pd        = new PointDisp("hand:display", "mouse");
                pd->size  = 2 * pixrad;
                pd->color = DP.front_color;
                dproperties.deposit(pd);
            }
        }
    }
}


//------------------------------------------------------------------------------
/**
 Processes mouse motion
 */
void Player::processMouseActionMotion(Vector3& ori3, const Vector3& pos3, int mode)
{
    Vector pos(pos3.XX, pos3.YY, pos3.ZZ);
    Vector ori(ori3.XX, ori3.YY, ori3.ZZ);
    
    if ( mode )
    {
        simThread.moveHandles(pos-ori);
        ori3 = pos3;
    }
    else
        simThread.moveHandle(pos);
}


/**
 Called when mouse enters/leaves current window
 */
void Player::processMouseEntry(int state)
{
    //std::cerr << "mouseEntry " << state << std::endl;
}


