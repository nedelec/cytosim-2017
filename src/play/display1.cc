// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "sim.h"
#include "simul.h"
#include "display1.h"
#include "modulo.h"

#include "fake.h"
#include "fiber_prop.h"
#include "fiber_disp.h"
#include "line_disp.h"
#include "point_disp.h"
#include "simul_prop.h"
#include "hand_prop.h"
#include "sphere_prop.h"
#include "solid_prop.h"

#include "opengl.h"
#include "gle.h"
#include "gle_color_list.h"
#include "glapp.h"

using namespace gle;
extern Modulo * modulo;

//------------------------------------------------------------------------------

Display1::Display1(DisplayProp const* dp) : Display(dp)
{
}

//------------------------------------------------------------------------------

void Display1::display(Simul const& sim)
{
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);

    displayFields(sim.fields);
    
#if ( DIM == 3 )

    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    prop->inner_color.back();

#endif
    
    displayBSpaces(sim.spaces);
    
    glDisable(GL_LIGHTING);
    
#if ( DIM <= 2 )
    
    if ( prop->couple_select & 1 )
        displayFCouples(sim.couples);
    
    if ( prop->single_select & 1 )
        displayFSingles(sim.singles);
#else
    
    if ( prop->couple_select & 4 )
        displayBCouples(sim.couples);

#endif
    
    if ( prop->single_select & 2 )
        displayASingles(sim.singles);

    
    displayFibers(sim.fibers);
    
#if ( DIM == 3 )
    
    glEnable(GL_LIGHTING);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    prop->inner_color.back();
    
#endif
    
    displaySolids(sim.solids);
    displayBeads(sim.beads);
    displaySpheres(sim.spheres);
    
    glDisable(GL_LIGHTING);

    displayOrganizers(sim.organizers);
    
    glDepthMask(GL_FALSE);

    if ( prop->couple_select & 2 )
        displayACouples(sim.couples);

#if ( DIM <= 2 )

    if ( prop->couple_select & 4 )
        displayBCouples(sim.couples);

#else
    
    if ( prop->couple_select & 1 )
        displayFCouples(sim.couples);
    
    if ( prop->single_select & 1 )
        displayFSingles(sim.singles);

#endif

    displayMisc(sim);
    
#if ( DIM == 3 )
    
    glEnable(GL_LIGHTING);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

#endif
    
    displayTransparentObjects(sim);
    displayFSpaces(sim.spaces);
    
    glDisable(GL_LIGHTING);
    glDepthMask(GL_TRUE);
}

//------------------------------------------------------------------------------
#pragma mark -

void Display1::displayBall(Vector const& pos, real radius)
{
    glPushMatrix();
    gleTranslate(pos);
    gleScale(radius);
    if ( DIM == 3 )
    {
        assert_true(glIsEnabled(GL_CULL_FACE));
        glCullFace(GL_FRONT);
        gleSphere4B();
        glCullFace(GL_BACK);
        gleSphere4B();
    }
    else
        gleCircleSB();
    glPopMatrix();
}


/**
 this version draws a bitmap object,
 and it is not necessary to set the current color before
 */
void Display1::displayPoint(Vector const& pos, PointDisp const* disp)
{
    if ( disp->visible  &&  disp->size*uFactor > 1 )
    {
#ifdef POINTDISP_USES_BITMAPS
        gleRasterPos(pos);
        disp->drawA();
#else
        glPushMatrix();
        gleTranslate(pos);
        gleScale(disp->size*sFactor);
        disp->drawA();
        glPopMatrix();
#endif
    }
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 Display the MINUS_END of a Fiber, according to \a style:
 - 1: draw a sphere
 - 2: draw a cone
 - 3: draw a flat cylinder
 - 4: draw an arrow-head
 .
 */
void Display1::displayMinusEnd(const int style, Fiber const& fib, real width) const
{
    switch(style)
    {
        case 1:
            gleObject(fib.posPoint(0), width, gleSphere2B);
            break;
        case 2:
            gleCone(fib.posPoint(0), -fib.dirPoint(0), width);
            break;
        case 3:
            gleCylinder(fib.posPoint(0), -fib.dirPoint(0), width);
            break;
        case 4:
            gleArrowTail(fib.posPoint(0), fib.dirPoint(0), width);
            break;
        case 5:
            gleArrowTail(fib.posPoint(0), -fib.dirPoint(0), width);
            break;
    }
}

/**
 Display the PLUS_END of a Fiber, according to \a style:
 - 1: draw a sphere
 - 2: draw a cone
 - 3: draw a flat cylinder
 - 4: draw an arrow-head
 .
 */
void Display1::displayPlusEnd(const int style, Fiber const& fib, real width) const
{
    switch(style)
    {
        case 1:
            gleObject(fib.posEnd(PLUS_END), width, gleSphere2B);
            break;
        case 2:
            gleCone(fib.posEnd(PLUS_END), fib.dirEnd(PLUS_END), width);
            break;
        case 3:
            gleCylinder(fib.posEnd(PLUS_END), fib.dirEnd(PLUS_END), width);
            break;
        case 4:
            gleArrowTail(fib.posEnd(PLUS_END), fib.dirEnd(PLUS_END), width);
            break;
        case 5:
            gleArrowTail(fib.posEnd(PLUS_END), -fib.dirEnd(PLUS_END), width);
            break;
    }
}

//------------------------------------------------------------------------------

void Display1::displayFiber(Fiber const& fib)
{
    const FiberDisp * disp = fib.prop->disp;
    // diameter of lines and points in pixel:
    GLfloat pWidth = ( disp->line_width > 0 ) ? disp->line_width * uFactor : 0.25;
    GLfloat pSize  = ( disp->point_size > 0 ) ? disp->point_size * uFactor : 0.25;
    const gle_color col = fib.disp->color;
    

    if ( disp->line_style == 1 )
    {
        // display plain lines:
        glLineWidth(pWidth);
        col.color();
        glBegin(GL_LINE_STRIP);
        for ( unsigned int ii = 0; ii < fib.nbPoints(); ++ii )
            gleVertex( fib.posPoint(ii) );
        glEnd();
    }
    else if ( disp->line_style == 2 )
    {
        // display segments with color indicating internal tension
        glLineWidth(pWidth);
        glBegin(GL_LINES);
        for ( unsigned int ii = 0; ii < fib.lastPoint(); ++ii )
        {
            // the Lagrange multipliers are negative under compression
            gleJetColor(1-fib.tension(ii)*disp->rainbow, col.alphaf());
            gleVertex( fib.posPoint(ii) );
            gleVertex( fib.posPoint(ii+1) );
        }
        glEnd();
    }
    else if ( disp->line_style == 3 )
    {
#if ( DIM == 1 )
        col.color();
#endif
        glLineWidth(pWidth);
        glBegin(GL_LINES);
        for ( unsigned int ii = 0; ii < fib.lastPoint(); ++ii )
        {
            Vector d = fib.diffPoints(ii);
#if ( DIM > 1 )
            // use the angle with respect to the XY-plane:
            gleHueColor(atan2(d.YY, d.XX) / ( 2 * M_PI ), 1.0);
#endif
            gleVertex( fib.posPoint(ii) );
            gleVertex( fib.posPoint(ii+1) );
        }
        glEnd();
    }
    else if ( disp->line_style == 4 )
    {
        // display colored segments indicating polarity
        const real grad = disp->interval;
        real ab = ceil( fib.abscissaM() ) + grad;
        glLineWidth(pWidth);
        glBegin(GL_LINE_STRIP);
        glColor4f(0,0,0,0);
        gleVertex(fib.posPoint(0));
        while ( ab <= fib.abscissaP() )
        {
            col.color();
            Vector pos = fib.pos(ab);
            gleVertex(pos);
            glColor4f(0,0,0,0);
            gleVertex(pos);
            ab += grad;
        }
        col.color();
        gleVertex(fib.posEnd(PLUS_END));
        glEnd();
    }
    
    // display random speckles:
    if ( disp->speckle_style == 1 )
    {
        /*
         A simple random number generators is used to distribute
         points always in the same way for each Fiber, 
         because it is seeded by fib.signature();
         */
        const real grad = 0x3p-32 * disp->interval;
        
        glPointSize(pSize);
        glBegin(GL_POINTS);
        col.color();
        
        if ( fib.abscissaM() < 0 )
        {
            uint32_t z = lcrng2(lcrng2(fib.signature()));
            real ab = -0.5 * grad * z;
            while ( ab > fib.abscissaP() )
            {
                z = lcrng2(z);
                ab -= grad * z;
            }
            while ( ab >= fib.abscissaM() )
            {
                gleVertex( fib.pos(ab) );
                z = lcrng2(z);
                ab -= grad * z;
            }
        }
        if ( fib.abscissaP() > 0 )
        {
            uint32_t z = lcrng1(lcrng1(fib.signature()));
            real ab = 0.5 * grad * z;
            while ( ab < fib.abscissaM() )
            {
                z = lcrng1(z);
                ab += grad * z;
            }
            while ( ab <= fib.abscissaP() )
            {
                gleVertex( fib.pos(ab) );
                z = lcrng1(z);
                ab += grad * z;
            }
        }
        glEnd();
    }
    else if ( disp->speckle_style == 2 )
    {
        // display regular speckles
        glPointSize(pSize);
        glBegin(GL_POINTS);
        col.color();
        //we distribute points regularly along the tube
        const real grad = disp->interval;
        real ab = grad * ceil( fib.abscissaM() / grad );
        while ( ab <= fib.abscissaP() ) {
            gleVertex( fib.pos(ab) );
            ab += grad;
        }
        glEnd();
    }
    
    
    if ( disp->point_style == 1 )
    {
        // display model-points
        glPointSize(pSize);
        col.color();
        glBegin(GL_POINTS);
        for ( unsigned int ii = 0; ii < fib.nbPoints(); ++ii )
            gleVertex( fib.posPoint(ii) );
        glEnd();
    }
    else if ( disp->point_style == 2 )
    {
        // display arrow-heads along the fiber
        col.color();
        real ab = ceil( fib.abscissaM() );
        const real grad = disp->interval;
        while ( ab <= fib.abscissaP() )
        {
            gleCone(fib.pos(ab), fib.dir(ab), 0.75*pSize*mPixelSize);
            ab += grad;
        }
    }
    else if ( disp->point_style == 3 )
    {
        // display integral abscissa along the fiber
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
        glLineWidth(pSize);
        disp->forces_color.color();
        glBegin(GL_LINES);
        for ( unsigned int ii = 0; ii < fib.nbPoints(); ++ii )
        {
            gleVertex( fib.posPoint(ii) );
            gleVertex( fib.posPoint(ii) + disp->forces * fib.netForce(ii) );
        }
        glEnd();
    }
}


void Display1::displayFiberMinusEnd(Fiber const& fib)
{
    const int IM = 1;
    const FiberDisp * disp = fib.prop->disp;
    const real hlen = disp->end_section[IM];

    if ( hlen > 0 )
    {
        glLineWidth(disp->end_size[IM]*uFactor);
        gle_color ecol = fib.disp->end_color[IM];
        
        glBegin(GL_LINE_STRIP);
        unsigned int ii = 0;
        real len = 0;
        while ( len < hlen  &&  ii < fib.nbPoints() )
        {
            ecol.colorA(1-0.7*len/hlen);
            gleVertex(fib.posPoint(ii));
            len += fib.segmentation();
            ++ii;
        }
        if ( ii < fib.nbPoints() )
        {
            ecol.colorA(0.3);
            gleVertex(fib.pos(hlen, MINUS_END));
        }
        glEnd();
    }

    
    if ( disp->end_style[IM]  &&  disp->end_size[IM] > 0 )
    {
        fib.disp->end_color[IM].color();
        displayMinusEnd(disp->end_style[IM], fib, disp->end_size[IM]*sFactor);
    }
}


void Display1::displayFiberPlusEnd(Fiber const& fib)
{
    const int IP = 0;
    const FiberDisp * disp = fib.prop->disp;
    const real hlen = disp->end_section[IP];

    if ( hlen > 0 )
    {
        glLineWidth(disp->end_size[IP]*uFactor);
        gle_color ecol = fib.disp->end_color[IP];
        
        glBegin(GL_LINE_STRIP);
        int ii = fib.lastPoint();
        real len = 0;
        while ( len < hlen  &&  ii >= 0 )
        {
            ecol.colorA(1-0.7*len/hlen);
            gleVertex(fib.posPoint(ii));
            len += fib.segmentation();
            --ii;
        }
        if ( ii >= 0 )
        {
            ecol.colorA(0.3);
            gleVertex(fib.pos(hlen, PLUS_END));
        }
        glEnd();
    }

    
    if ( disp->end_style[IP]  &&  disp->end_size[IP] > 0 )
    {
        fib.disp->end_color[IP].color();
        displayPlusEnd(disp->end_style[IP], fib, disp->end_size[IP]*sFactor);
    }
}


//------------------------------------------------------------------------------
#pragma mark -

void Display1::displayBead(Bead const& obj)
{
    const PointDisp * disp = obj.prop->disp;
    gle_color col = bodyColor(disp, obj.number());

    // display center:
    if ( disp->style & 1 )
    {
        //col.color();
        displayPoint(obj.position(), disp);
    }
    
    // display outline:
    if ( DIM == 2  && disp->style & 4  &&  disp->width > 0 )
    {
        col.color();
        glLineWidth(disp->width*uFactor);
        gleObject(obj.position(), obj.radius(), gleCircleLB);
    }
}


/**
 Display a semi-transparent disc / sphere if ( disp->style & 2 ) 
 */
void Display1::displayTBead(Bead const& obj)
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

void Display1::displaySolid(Solid const& obj)
{
    const PointDisp * disp = obj.prop->disp;
    gle_color col = bodyColor(disp, obj.number());
    
    //display points of the Solids
    if ( disp->size > 0  &&  disp->style & 1 )
    {
        //col.color();
        for ( unsigned int ii = 0; ii < obj.nbPoints(); ++ii )
            displayPoint(obj.posPoint(ii), disp);
    }
    
#if ( DIM == 3 )
    //special display for ParM simulations (DYCHE)
    if ( obj.mark()  &&  disp->style & 2  &&  obj.nbPoints() >= 3 )
    {
        col.color();
        glLineWidth(uFactor);
        gleObject(obj.posPoint(0), obj.diffPoints(1, 0), obj.radius(0), gleCircleLB);
    }
#endif
    
    //display outline of spheres
    if ( DIM == 2  &&  disp->width > 0  &&  disp->style & 4 )
    {
        col.color();
        glLineWidth(disp->width*uFactor);
        for ( unsigned int ii = 0; ii < obj.nbPoints(); ++ii )
        {
            if ( obj.radius(ii) > 0 )
                gleObject(obj.posPoint(ii), obj.radius(ii), gleCircleLB);
        }
    }
    
    //print a signature for each solid
    if ( disp->style & 8 )
    {
        char tmp[8];
        snprintf(tmp, sizeof(tmp), "%lu", obj.number());
        col.color();
        gleDrawText(obj.posPoint(0), tmp, GLUT_BITMAP_HELVETICA_10);
    }

    //print outline for each solid
    if ( disp->style & 16 )
    {
        col.color();
        glBegin(GL_LINE_LOOP);
        for ( unsigned ii = 0; ii < obj.nbPoints(); ++ii )
            gleVertex(obj.posPoint(ii));
        glEnd();
    }
}

/**
 Display a semi-transparent disc / sphere if ( disp->style & 2 ) 
 */
void Display1::displayTSolid(Solid const& obj, unsigned int ii)
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

void Display1::displaySphere(Sphere const& obj)
{
    const PointDisp * disp = obj.prop->disp;
    
    //display the points of the Sphere
    if ( disp->style & 1 )
    {
        //bodyColor(disp, obj.number()).color();
        for ( unsigned int ii = 0; ii < obj.nbPoints(); ii++ )
            displayPoint(obj.posPoint(ii), disp);
    }
}


void Display1::displayTSphere(Sphere const& obj)
{
    const PointDisp * disp = obj.prop->disp;

    if ( disp->style & 6 )
    {
        bodyColor(disp, obj.number()).color();
        glLineWidth(disp->width*uFactor);
        
#if (DIM <= 2)
        
        if ( disp->style & 2 )
            gleObject(obj.posPoint(0), obj.radius(), gleCircleSB);
        
        if ( disp->style & 4 )
            gleObject(obj.posPoint(0), obj.radius(), gleCircleLB);
        
#elif (DIM == 3)
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
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
        glPopMatrix();
        
#endif
    }
}

//------------------------------------------------------------------------------
void Display1::displayOrganizer(Organizer const& obj)
{
    const PointDisp * disp = obj.pointDisp();
    
    if ( disp )
    {
        disp->color.color();
        
        glLineWidth(disp->size*uFactor);
        glBegin(GL_LINES);
        for ( unsigned int ix = 0 ; ix < obj.nbLinks(); ++ix )
        {
            gleVertex( obj.posLink1(ix) );
            gleVertex( obj.posLink2(ix) );
        }
        glEnd();
    }

    
#if (DIM==3)
    /**
     This display the Solid connecting two Aster as a spindle.
     Used for Cleo Kozlowski simulation of C. elegans
     */
    if ( obj.tag() == Fake::TAG )
    {
        Solid const* so = static_cast<const Fake&>(obj).solid();
        if ( so && so->nbPoints() >= 4 )
        {
            glEnable(GL_LIGHTING);
            glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
            prop->inner_color.back();
            so->prop->disp->color.color();
            glPushMatrix();
            Vector3 a = 0.5*(so->posPoint(0) + so->posPoint(2));
            Vector3 b = 0.5*(so->posPoint(1) + so->posPoint(3));
            real diam = 1;
            Vector3 dir = b-a;
            Vector3 P1  = dir.orthogonal(diam);
            Vector3 P2  = vecProd(dir, P1).normalized(diam);
            gleTransRotate( P1, P2, dir, a );
            glColor4f(1,1,1,1);
            gleDualPass(gleBarrel1);
            glPopMatrix();
            glDisable(GL_LIGHTING);
        }
    }
#endif
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 display the attached position of free singles
 */
void Display1::displayFSingles(const SingleSet & set)
{
    for ( Single * gh=set.firstF(); gh ; gh=gh->next() )
    {
        displayPoint(gh->posFoot(), gh->hand()->prop->disp);
    }
}


void Display1::displayASingles(const SingleSet & set)
{
    // display the Hands
    for ( Single * gh=set.firstA(); gh ; gh=gh->next() )
    {
        if ( ! gh->fiber()->disp->visible )
            continue;
        
        const PointDisp * disp = gh->hand()->prop->disp;
        Vector ph = gh->posHand();
        
        //disp->color.color();
        displayPoint(ph, disp);
        
        if ( gh->hasInteraction() )
        {
            Vector ps = gh->posSide();
            Vector pf = gh->posFoot();
            if (modulo)
            {
                modulo->fold(pf, ph);
                modulo->fold(ps, ph);
            }
             
#if ( DIM == 3 )
            gleTube(ph, disp->width*sFactor, disp->color, pf, disp->width*sFactor, disp->color.fade_alpha(2));
#else
            disp->color.color();
            gleBand(ph, disp->width*sFactor, ps, disp->width*sFactor);
            gleBand(ps, disp->width*sFactor, disp->color, pf, disp->width*sFactor, disp->color.fade_alpha(2));
#endif
        }
    }
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 We display either Hand1 or Hand2, exposing both sides with equal chances.
 This gives the impression that Couple flicker randomly between frames,
 as if they were two-sided balls 'rotating' very fast.
 */
void Display1::displayFCouples(CoupleSet const& set)
{
    Couple * nxt;
    Couple * obj = set.firstFF();
    // this loop is unrolled, processing objects 2 by 2:
    if ( set.sizeFF() % 2 )
    {
        nxt = obj->next();
        displayPoint(obj->posFree(), obj->disp1());
        obj = nxt;
    }
    while ( obj )
    {
        nxt = obj->next();
        displayPoint(obj->posFree(), obj->disp2());
        obj = nxt->next();
        displayPoint(nxt->posFree(), nxt->disp1());
    }
}


void Display1::displayACouples(CoupleSet const& set)
{
    // display bound couples
    for (Couple * cx=set.firstAF(); cx ; cx=cx->next() )
        if ( cx->fiber1()->disp->visible )
        {
            displayPoint(cx->pos1(), cx->disp1());
        }
    
    for (Couple * cx=set.firstFA(); cx ; cx=cx->next() )
        if ( cx->fiber2()->disp->visible )
        {
            displayPoint(cx->pos2(), cx->disp2());
        }
}


void Display1::displayBCouples(CoupleSet const& set)
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
        
        if ( pd1 == pd2 )
        {
#if ( DIM == 2 )
            pd1->color.color();
            gleDumbbell(p1, p2, pd1->size*sFactor);
            //gleBand(p1, pd1->width*sFactor, p2, pd2->width*sFactor);
            //displayPoint(p1, pd1);
            //displayPoint(p2, pd2);
#else
            gleTube(p1, pd1->width*sFactor, pd1->color, p2, pd2->width*sFactor, pd2->color);
            //pd1->color.color();
            displayPoint(p1, pd1);
            displayPoint(p2, pd2);
#endif
        }
        else
        {
#if ( DIM == 3 )
            gleTube(p1, pd1->width*sFactor, pd1->color, p2, pd2->width*sFactor, pd2->color);
#else
            gleBand(p1, pd1->width*sFactor, pd1->color, p2, pd2->width*sFactor, pd2->color);
#endif
            //pd1->color.color();
            displayPoint(p1, pd1);
            //pd2->color.color();
            displayPoint(p2, pd2);
        }
    }
}

