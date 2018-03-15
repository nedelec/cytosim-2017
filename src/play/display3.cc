// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "display3.h"

#include "fake.h"
#include "hand_prop.h"
#include "sphere_prop.h"
#include "solid_prop.h"
#include "fiber_prop.h"
#include "fiber_disp.h"
#include "line_disp.h"
#include "point_disp.h"
#include "simul.h"
#include "modulo.h"

#include "opengl.h"
#include "gle.h"
#include "gle_color_list.h"
#include "glapp.h"

using glApp::GP;
using namespace gle;

extern Modulo * modulo;


Display3::Display3(DisplayProp const* dp) : Display(dp)
{
    spaceDL  = 0;
    dlSpace  = 0;
}

Display3::~Display3()
{
    if ( spaceDL )
        glDeleteLists(spaceDL, 1);
    dlSpace = 0;
}

//------------------------------------------------------------------------------
#pragma mark -

void Display3::display(Simul const& sim)
{
    // display field in the back
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    displayFields(sim.fields);

    glEnable(GL_LIGHTING);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    prop->inner_color.back();

    displayBSpaces(sim.spaces);
    /*
     Draw opaque objects first:
     - depth buffer is writable
     - glColor specifies the Front and Back material color
    */

    // Since Fibers are open tubes, we do not enable CULL_FACE
    glDisable(GL_CULL_FACE);
    displayFibers(sim.fibers);
    
    glEnable(GL_CULL_FACE);
    displayFibersPoints(sim.fibers);
    displaySolids(sim.solids);
    displayBeads(sim.beads);
    displaySpheres(sim.spheres);

    if ( prop->couple_select & 4 )
        displayBCouples(sim.couples);

    if ( prop->single_select & 2 )
        displayASingles(sim.singles);
    
    displayOrganizers(sim.organizers);
    displayMisc(sim);

    /*
     Draw translucent objects second:
     - depth buffer is readible only
     - glColor specifies GL_FRONT and GL_BACK material color
     - the objects should all be depth-sorted, but we do not do this here
     - Dual pass is used to display back before front
     */
    glDepthMask(GL_FALSE);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    if ( prop->single_select & 1 )
        displayFSingles(sim.singles);
    
    if ( prop->couple_select & 1 )
        displayFCouples(sim.couples);
    
    if ( prop->couple_select & 2 )
        displayACouples(sim.couples);

    displayTransparentObjects(sim);
    displayFSpaces(sim.spaces);

    glDepthMask(GL_TRUE);
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 Display the back side of the Space
 */
void Display3::displayBSpace(Space const& obj)
{
    const PointDisp * disp = obj.prop->disp;
    if ( disp->width > 0 )
        glLineWidth(disp->width*uFactor);
     
    if ( disp->visible > 0 )
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);
    
    //create the display list to draw the space
    if ( 0 == spaceDL )
    {
        //a displayList is faster, but the problem is that the display is not
        //updated, if the Space change in size/shape.
        spaceDL = glGenLists(1);
        glNewList(spaceDL, GL_COMPILE);
        obj.display();
        glEndList();
        dlSpace = &obj;
    }

    if ( DIM == 3 )
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
    }

    if ( &obj == dlSpace &&  glIsList(spaceDL) )
        glCallList(spaceDL);
    else
        obj.display();
    
    glDepthMask(GL_TRUE);
    glCullFace(GL_BACK);
}

/**
 Display the front side of the Space
 using a display list to draw the space
 */
void Display3::displayFSpace(Space const& obj)
{
    if ( DIM == 3 )
    {
        const PointDisp * disp = obj.prop->disp;
        
        if ( disp->width > 0 )
            glLineWidth(disp->width*uFactor);
        disp->color.color();
        if ( disp->visible > 0 )
            glDepthMask(GL_TRUE);
        else
            glDepthMask(GL_FALSE);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        
        if ( &obj == dlSpace  &&  glIsList(spaceDL) )
            glCallList(spaceDL);
        else
            obj.display();
        
        glDepthMask(GL_TRUE);
    }
}

//------------------------------------------------------------------------------
#pragma mark -

void Display3::displayBall(Vector const& pos, real radius)
{
    assert_true(glIsEnabled(GL_CULL_FACE));
    glPushMatrix();
    gleTranslate(pos);
    gleScale(radius);
    glCullFace(GL_FRONT);
    gleSphere4B();
    glCullFace(GL_BACK);
    gleSphere4B();
    glPopMatrix();
}


void Display3::displayPoint(Vector const& pos, PointDisp const* disp)
{
    if ( disp->visible )
    {
        glPushMatrix();
        gleTranslate(pos);
        gleScale(disp->size*sFactor);
        gleSphere1B();
        glPopMatrix();
    }
}


void setClipPlane(GLenum plane, Vector dir, Vector pos)
{
#if ( DIM == 3 )
    GLdouble eq[4] = { dir.XX, dir.YY, dir.ZZ, -dir*pos };
#elif ( DIM == 2 )
    GLdouble eq[4] = { dir.XX, dir.YY, 0, -dir*pos };
#else
    GLdouble eq[4] = { dir.XX, 0, 0, -dir*pos };
#endif
    glClipPlane(plane, eq);
}


//------------------------------------------------------------------------------
#pragma mark -

void Display3::displayFiber(Fiber const& fib)
{
    const FiberDisp * disp = fib.prop->disp;
    // diameter of lines and points in space units:
    real rWidth = disp->line_width * sFactor;
    const gle_color col = fib.disp->color;

    col.color();
    //col.fade(1).back();
    
    if ( disp->line_style == 1 )
    {
        // display lines only
#if ( DIM > 1 )
        // this code makes nice joints for bent tubes, by using CLIP_PLANES
        const GLenum glp1 = GL_CLIP_PLANE3;
        const GLenum glp2 = GL_CLIP_PLANE4;
        glEnable(glp1);
        glEnable(glp2);
        
        unsigned int ii = 0;
        // the MINUS_END
        setClipPlane(glp1, fib.dirPoint(ii), fib.posPoint(ii));
        // the inner segments
        for ( ; ii < fib.lastSegment(); ++ii )
        {
            Vector dir = 0.5 * ( fib.dirPoint(ii) + fib.dirPoint(ii+1) );
            setClipPlane(glp2, -dir, fib.posPoint(ii+1));
            gleTube(fib.posPoint(ii), fib.posPoint(ii+1), rWidth, gleLongTube2B);
            setClipPlane(glp1, +dir, fib.posPoint(ii+1));
        }
        // the PLUS_END:
        setClipPlane(glp2, -fib.dirPoint(ii), fib.posPoint(ii+1));
        gleTube(fib.posPoint(ii), fib.posPoint(ii+1), rWidth, gleLongTube2B);
        
        glDisable(glp1);
        glDisable(glp2);
#else
        for ( unsigned int ii = 0; ii < fib.nbSegments(); ++ii )
            gleTube(fib.posPoint(ii), fib.posPoint(ii+1), rWidth, gleTube2B);
#endif
        
        //close the ends of the Tube with discs
        gleObject(fib.posEnd(MINUS_END), -fib.dirEnd(MINUS_END), rWidth, gleCircleSB);
        gleObject(fib.posEnd(PLUS_END),   fib.dirEnd(PLUS_END),  rWidth, gleCircleSB);
    }
    else if ( disp->line_style == 2 )
    {
        // display the internal tensions of the microtubules
        for ( unsigned int ii = 0; ii < fib.nbSegments(); ++ii )
        {
            // the Lagrange multipliers are negative under compression
            gleJetColor(1-fib.tension(ii)*disp->rainbow, col.alphaf());
            gleTube(fib.posPoint(ii), fib.posPoint(ii+1), rWidth, gleTube2B);
        }
    }
    else if ( disp->line_style == 3 )
    {
#if ( DIM == 1 )
        col.color();
#endif
        for ( unsigned int ii = 0; ii < fib.nbSegments(); ++ii )
        {
            Vector d = fib.diffPoints(ii);
#if ( DIM > 1 )
            // use the angle with respect to the XY-plane:
            gleHueColor(atan2(d.YY, d.XX) / ( 2 * M_PI ), 1.0);
#endif
            gleTube(fib.posPoint(ii), fib.posPoint(ii+1), rWidth, gleTube2B);
        }
    }
    else if ( disp->line_style == 4 )
    {
        // display segments which gradient of color indicating polarity
        const real grad = disp->interval;
        real ab = ceil( fib.abscissaM() );
        while ( ab <= fib.abscissaP() )
        {
            gleTube(fib.pos(ab), rWidth, 0x00000000, fib.pos(ab+grad), rWidth, col);
            ab += grad;
        }
    }
}


void Display3::displayFiberMinusEnd(Fiber const& fib)
{
    const int IM = 1;
    const FiberDisp * disp = fib.prop->disp;
    const real hlen = disp->end_section[IM];

    if ( hlen > 0 )
    {
        real width = disp->end_size[IM]*sFactor;
        fib.disp->end_color[IM].color();
        
        unsigned int ii = 1;
        real len = fib.segmentation();
        while ( len < hlen  &&  ii < fib.nbPoints() )
        {
            gleTube(fib.posPoint(ii-1), fib.posPoint(ii), width, gleTube2B);
            len += fib.segmentation();
            ++ii;
        }
        if ( ii < fib.nbPoints() )
            gleTube(fib.posPoint(ii-1), fib.pos(hlen, MINUS_END), width, gleTube2B);
        //close the ends of the Tube with discs
        gleObject(fib.posEnd(MINUS_END), -fib.dirEnd(MINUS_END), width, gleCircleSB);
        gleObject(fib.pos(hlen, MINUS_END),  fib.dir(hlen, MINUS_END),  width, gleCircleSB);
    }

    
    if ( disp->end_style[IM]  &&  disp->end_size[IM] > 0 )
    {
        fib.disp->end_color[IM].color();
        displayMinusEnd(disp->end_style[IM], fib, disp->end_size[IM]*sFactor);
    }
}


void Display3::displayFiberPlusEnd(Fiber const& fib)
{
    const int IP = 0;
    const FiberDisp * disp = fib.prop->disp;
    const real hlen = disp->end_section[IP];
    
    if ( hlen > 0 )
    {
        real width = disp->end_size[IP]*sFactor;
        fib.disp->end_color[IP].color();

        int ii = fib.lastPoint();
        real len = fib.segmentation();
        while ( len < hlen  &&  0 < ii )
        {
            gleTube(fib.posPoint(ii-1), fib.posPoint(ii), width, gleTube2B);
            len += fib.segmentation();
            --ii;
        }
        if ( 0 < ii )
            gleTube(fib.pos(hlen, PLUS_END), fib.posPoint(ii), width, gleTube2B);
        //close the ends of the Tube with discs
        gleObject(fib.posEnd(PLUS_END),  fib.dirEnd(PLUS_END), width, gleCircleSB);
        gleObject(fib.pos(hlen, PLUS_END), -fib.dir(hlen, PLUS_END), width, gleCircleSB);
    }


    if ( disp->end_style[IP]  &&  disp->end_size[IP] > 0 )
    {
        fib.disp->end_color[IP].color();
        displayPlusEnd(disp->end_style[IP], fib, disp->end_size[IP]*sFactor);
    }
}



void Display3::displayFiberPoints(Fiber const& fib)
{
    const FiberDisp * disp = fib.prop->disp;
    // diameter of lines and points in space units:
    real rSize  = disp->point_size * sFactor;
    const gle_color col = fib.disp->color;
    
    if ( rSize <= 0 ) return;
    
    // display random speckles:
    if ( disp->speckle_style == 1 )
    {
        /*
         A simple random number generators is used to distribute
         points always in the same way for each Fiber, 
         because it is seeded by fib.signature();
         */
        const real grad = 0x3p-32 * disp->interval;
        col.color();
        
        if ( fib.abscissaM() < 0 )
        {
            uint32_t z = lcrng1(lcrng1(fib.signature()));
            real ab = -0.5 * grad * z;
            while ( ab > fib.abscissaP() )
            {
                z = lcrng1(z);
                ab -= grad * z;
            }
            while ( ab >= fib.abscissaM() )
            {
                gleObject(fib.pos(ab), rSize, gleSphere1B);
                z = lcrng1(z);
                ab -= grad * z;
            }
        }
        if ( fib.abscissaP() > 0 )
        {
            uint32_t z = lcrng2(lcrng2(fib.signature()));
            real ab = 0.5 * grad * z;
            while ( ab < fib.abscissaM() )
            {
                z = lcrng2(z);
                ab += grad * z;
            }
            while ( ab <= fib.abscissaP() )
            {
                gleObject(fib.pos(ab), rSize, gleSphere1B);
                z = lcrng2(z);
                ab += grad * z;
            }
        }
    }
    else if ( disp->speckle_style == 2 )
    {
        // display regular speckles:
        col.color();
        //we distribute points regularly along the tube
        const real grad = disp->interval;
        real ab = grad * ceil( fib.abscissaM() / grad );
        while ( ab <= fib.abscissaP() )
        {
            gleObject(fib.pos(ab), rSize, gleSphere1B);
            ab += grad;
        }
    }
    
    
    if ( disp->point_style == 1 )
    {
        // display model-points:
        col.color();
        for ( unsigned int ii = 0; ii < fib.nbPoints(); ++ii )
            gleObject(fib.posPoint(ii), rSize, gleSphere2B);
    }
    else if ( disp->point_style == 2 )
    {
        // display an arrow-head every micro-meter:
        col.color();
        real ab = ceil( fib.abscissaM() );
        const real grad = disp->interval;
        while ( ab <= fib.abscissaP() )
        {
            gleCone(fib.pos(ab), fib.dir(ab), 1.5*rSize);
            ab += grad;
        }
    }
    else if ( disp->point_style == 3 )
    {
        // display integral abscissa
        char tmp[16];
        col.color();
        snprintf(tmp, sizeof(tmp), "%.2f", fib.abscissaM());
        gleDrawText(fib.posEnd(MINUS_END), tmp, GLUT_BITMAP_HELVETICA_10);
        
        int s = ceil( fib.abscissaM() + disp->interval );
        int e = floor( fib.abscissaP() - disp->interval );
        for ( int a = s; a <= e; ++a )
        {
            snprintf(tmp, sizeof(tmp), "%i", a);
            gleDrawText(fib.pos(a), tmp, GLUT_BITMAP_HELVETICA_10);
        }
        
        snprintf(tmp, sizeof(tmp), "%.2f", fib.abscissaP());
        gleDrawText(fib.posEnd(PLUS_END), tmp, GLUT_BITMAP_HELVETICA_10);
    }
    
    
    // display forces acting on the points:
    if ( disp->forces )
    {
        disp->forces_color.color();
        Vector P;
        for ( unsigned int ii = 0; ii < fib.nbPoints(); ++ii )
        {
            P = fib.posPoint(ii) + disp->forces * fib.netForce(ii);
            gleTube(fib.posPoint(ii), P, rSize, gleTube1B);
        }
    }
}


void Display3::displayFibersPoints(FiberSet const& set)
{
    for ( Fiber * obj = set.first(); obj ; obj = obj->next() )
        if ( obj->disp->visible > 0 )
            displayFiberPoints(*obj);
}


//------------------------------------------------------------------------------
#pragma mark -

void Display3::displayBead(Bead const& obj)
{
    const PointDisp * disp = obj.prop->disp;
    gle_color col = bodyColor(disp, obj.number());

    // display center
    if ( disp->style & 1 )
    {
        col.color();
        displayPoint(obj.position(), disp);
    }
    
    // display outline:
    if ( DIM == 2  &&  disp->style & 4  &&  disp->width > 0 )
    {
        col.color();
        glLineWidth(disp->width*uFactor);
        gleObject(obj.position(), obj.radius(), gleCircleLB);
    }
}

/**
 Display a semi-transparent sphere if ( disp->style & 2 ) 
 */
void Display3::displayTBead(Bead const& obj)
{
    const PointDisp * disp = obj.prop->disp;

    if ( disp->style & 2 )
    {
        bodyColor(disp, obj.number()).color();
        displayBall(obj.position(), obj.radius());
    }
}


//------------------------------------------------------------------------------
#pragma mark -

void Display3::displaySolid(Solid const& obj)
{
    const PointDisp * disp = obj.prop->disp;
    gle_color col = bodyColor(disp, obj.number());

    //display points:
    if ( disp->size > 0  &&  disp->style & 1 )
    {
        col.color();
        for ( unsigned int ii = 0; ii < obj.nbPoints(); ii++ )
            displayPoint(obj.posPoint(ii), disp);
    }
    
#if (DIM==3)
    //special display for ParM simulations (DYCHE)
    if ( obj.mark()  &&  disp->style & 2  &&  obj.nbPoints() >= 3 )
    {
        col.color();
        gleObject(obj.posPoint(0), obj.diffPoints(1, 0), obj.radius(0), gleCircleLB);
    }
#endif
    
    //display a signature for each Solid
    if ( disp->style & 8 )
    {
        char tmp[8];
        snprintf(tmp, sizeof(tmp), "%lu", obj.number());
        col.color();
        gleDrawText(obj.posPoint(0), tmp, GLUT_BITMAP_HELVETICA_10);
    }
}


/**
 Display a semi-transparent disc / sphere if ( disp->style & 2 ) 
 */
void Display3::displayTSolid(Solid const& obj, unsigned int ii)
{
    const PointDisp * disp = obj.prop->disp;

    if ( disp->style & 2 )
    {
        bodyColor(disp, obj.number()).color();
        if ( obj.radius(ii) > 0 )
            displayBall(obj.posPoint(ii), obj.radius(ii));
    }
}


//------------------------------------------------------------------------------
#pragma mark -

void Display3::displaySphere(Sphere const& obj)
{
    const PointDisp * disp = obj.prop->disp;

    //display the points on the Sphere (without the center-point)
    if ( disp->size > 0  &&  disp->style & 1 )
    {
        bodyColor(disp, obj.number()).color();
        for ( unsigned int ii = 0; ii < obj.nbPoints(); ++ii )
            displayPoint(obj.posPoint(ii),  disp);
    }
}

void Display3::displayTSphere(Sphere const& obj)
{
    const PointDisp * disp = obj.prop->disp;

    //display the  envelope
    if ( disp->style & 6 )
    {
        bodyColor(disp, obj.number()).color();
        glLineWidth(disp->width*uFactor);
        
        glPushMatrix();
        
#if (DIM == 2)
        
        gleTranslate(obj.posPoint(0));
        glutSolidTorus(disp->size*sFactor, obj.radius(), 2*gle::finesse, 10*gle::finesse);
        
#elif (DIM == 3)
        
        /* Note: The rotation matrix for the sphere calculated below from the
            reference points, includes scaling by the radius of the sphere.
            We then use a primitive for a sphere of radius 1.
            */
        const Vector C = obj.posPoint(0);
        gleTransRotate(obj.posPoint(1)-C, obj.posPoint(2)-C, obj.posPoint(3)-C, C);
        
        if ( disp->style & 4 )
            gleDualPass(gleDecoratedSphere);
        else
            gleDualPass(gleSphere4B);
        
#endif
        glPopMatrix();
    }
}

//------------------------------------------------------------------------------

void Display3::displayOrganizer(Organizer const& obj)
{
    const PointDisp * disp = obj.pointDisp();
    
    if ( disp )
    {
        disp->color.color();
        
        for ( unsigned int ix = 0 ; ix < obj.nbLinks(); ++ix )
            gleTube(obj.posLink1(ix), obj.posLink2(ix), disp->size*sFactor, gleTube1B);
    }

    
#if (DIM==3)
    /**
     This displays the Solid connecting two Aster as a spindle.
     Used for Cleo Kozlowski simulation of C. elegans
     */
    if ( obj.tag() == Fake::TAG )
    {
        Solid const* so = static_cast<const Fake&>(obj).solid();
        if ( so && so->nbPoints() >= 4 )
        {
            so->prop->disp->color.color();
            glPushMatrix();
            Vector3 a = 0.5 * (so->posPoint(0) + so->posPoint(2));
            Vector3 b = 0.5 * (so->posPoint(1) + so->posPoint(3));
            real diam = 1;
            Vector3 dir = b-a;
            Vector3 P1  = dir.orthogonal(diam);
            Vector3 P2  = vecProd(dir, P1).normalized(diam);
            gleTransRotate( P1, P2, dir, a );
            glColor3f(0.6,0.6,0.6);
            gleDualPass(gleBarrel1);
            glPopMatrix();
        }
    }
#endif    
}

//------------------------------------------------------------------------------
#pragma mark -

void Display3::displayFSingles(SingleSet const& set)
{
    //display the attached position:
    for ( Single *gh=set.firstF(); gh ; gh=gh->next() )
    {
        gh->hand()->prop->disp->color2.color();
        displayPoint(gh->posFoot(), gh->hand()->prop->disp);
    }
}


void Display3::displayASingles(SingleSet const& set)
{
    // display the Hands
    for ( Single * gh=set.firstA(); gh ; gh=gh->next() )
    {
        if ( !gh->fiber()->disp->visible )
            continue;

        const PointDisp * disp = gh->hand()->prop->disp;
        Vector ph = gh->posHand();
        
        disp->color.color();
        displayPoint(ph, disp);
        
        if ( gh->hasInteraction() )
        {
            Vector pf = gh->posFoot();
            if (modulo)
                modulo->fold(pf, ph);
#if ( DIM == 3 )
            gleTube(ph, disp->width*sFactor, disp->color, pf, disp->width*sFactor, disp->color.fade_alpha(2));
#else
            gleBand(ph, disp->width*sFactor, disp->color, pf, disp->width*sFactor, disp->color.fade_alpha(2));
#endif
        }
    }
}


//------------------------------------------------------------------------------
#pragma mark -

/**
 Display either Hand1 or Hand2, exposing both sides with equal chances.
 This gives the impression that Couple flicker randomly between frames,
 as if they were two-sided balls 'rotating' very fast.
 */
void Display3::displayFCouples(CoupleSet const& set)
{
    Couple * nxt;
    Couple * obj = set.firstFF();
    
    if ( set.sizeFF() % 2 )
    {
        nxt = obj->next();
        obj->disp1()->color2.color();
        displayPoint(obj->posFree(), obj->disp1());
        obj = nxt;
    }
    while ( obj )
    {
        nxt = obj->next();
        obj->disp1()->color2.color();
        displayPoint(obj->posFree(), obj->disp2());
        obj = nxt->next();
        nxt->disp1()->color2.color();
        displayPoint(nxt->posFree(), nxt->disp1());
    }
}


void Display3::displayACouples(CoupleSet const& set)
{
    for ( Couple * cx=set.firstAF(); cx ; cx=cx->next() )
        if ( cx->fiber1()->disp->visible )
        {
            cx->disp1()->color2.color();
            displayPoint(cx->pos1(), cx->disp1());
        }
    
    for ( Couple * cx=set.firstFA(); cx ; cx=cx->next() )
        if ( cx->fiber2()->disp->visible )
        {
            cx->disp2()->color2.color();
            displayPoint(cx->pos2(), cx->disp2());
        }
}


void Display3::displayBCouples(CoupleSet const& set)
{    
    for ( Couple * cx=set.firstAA(); cx ; cx=cx->next() )
    {
        // do not display Couple if the associated Fibers are both hidden
        if ( cx->fiber1()->disp->visible==0  &&  cx->fiber2()->disp->visible==0 )
            continue;
        // only display couples bound on anti-parallel sections
        if ( ( prop->couple_select & 8 ) && ( cx->cosAngle() > 0 ) )
            continue;
        
        const PointDisp * pd1 = cx->disp1();
        const PointDisp * pd2 = cx->disp2();
        Vector p1 = cx->pos1();
        Vector p2 = cx->pos2();
        if (modulo)
            modulo->fold(p2, p1);
        
        Vector dir = ( p1 - p2 );
        real dn = dir.norm();
        
        if ( dn > ( pd1->size + pd2->size ) * sFactor )
        {
            pd1->color.color();
            displayPoint(p1, pd1);
            pd2->color.color();
            displayPoint(p2, pd2);
            // display the link
            if ( pd1 == pd2 )
            {
                pd1->color.color();
                gleTube(p1, p2, pd2->width*sFactor, gleTube1B);
            }
            else
            {
                gleTube(p1, pd1->width*sFactor, pd1->color, p2, pd2->width*sFactor, pd2->color);
            }
        }
        else
        {
            if ( dn > REAL_EPSILON )
                dir /= dn;
            else
                dir.set(0.707,0.707,0);
            Vector mid = 0.5 * ( p1 + p2 );
            const GLenum glp = GL_CLIP_PLANE3;        
            setClipPlane(glp, dir, mid);
            glEnable(glp);
            pd1->color.color();
            displayPoint(p1, pd1);
            setClipPlane(glp, -dir, mid);
            pd2->color.color();
            displayPoint(p2, pd2);
            glDisable(glp);
        }
    }
}

