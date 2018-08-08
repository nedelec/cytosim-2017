// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "space_polygon.h"
#include "point_exact.h"
#include "exceptions.h"
#include "polygon.h"
#include "meca.h"
#include <fstream>

//------------------------------------------------------------------------------

SpacePolygon::SpacePolygon(SpaceProp const* p, std::string const& file)
: Space(p), height(mLength[0])
{
    nPoints = 0;
    mPoints = 0;
    mVolume = 0;
    
    if ( DIM < 2 )
        throw InvalidParameter("polygon is not usable in 1D.");
    
    std::ifstream in(file.c_str());
    
    if ( ! in.good() )
        throw InvalidParameter("polygon:file `"+file+"' not found");

    nPoints = Polygon::read(in, 0, 0);
    
    if ( nPoints < 3 )
        throw InvalidParameter("polygon: too few points specified in `"+file+"'");
    
    mPoints = new Polygon::Point2D[nPoints+2];
    
    in.clear();
    in.seekg(0);
    Polygon::read(in, mPoints, nPoints);
    in.close();
    
    resize();
}


SpacePolygon::~SpacePolygon()
{
    if ( mPoints )
        delete[] mPoints;
    mPoints = 0;
}


/**
 recalculate bounding box, volume
 and points offsets that are used to project
 */
void SpacePolygon::resize()
{
    if ( Polygon::prepare(mPoints, nPoints) )
        throw InvalidParameter("unfit polygon: consecutive points may overlap");

    real box[4];
    Polygon::boundingBox(mPoints, nPoints, box);
    real x = ( -box[0] > box[1] ) ? -box[0] : box[1];
    real y = ( -box[2] > box[3] ) ? -box[2] : box[3];
    boundingBox.set(x, y, height);
    
    mVolume = fabs(Polygon::surface(mPoints, nPoints));
    std::cerr << "Surface of polygon is " << mVolume << std::endl;
    
#if ( DIM == 3 )
    // total height = half_height
    mVolume *= 2 * height;
#endif
}


bool SpacePolygon::inside( const real w[] ) const
{
#if ( DIM == 3 )
    if ( w[2] < -height  ||  w[2] > height )
        return false;
#endif
    return Polygon::inside(mPoints, nPoints, w[0], w[1], 1);
}


void SpacePolygon::project( const real w[], real p[] ) const
{    
#if ( DIM == 1 )
    
    p[0] = w[0];
    
#elif ( DIM == 2 )
    
    real nX, nY;
    Polygon::project(mPoints, nPoints, w[0], w[1], p[0], p[1], nX, nY);
    
#elif ( DIM == 3 )
    
    real nX, nY;
    if ( fabs(w[2]) > height )
    {
        if ( Polygon::inside(mPoints, nPoints, w[0], w[1], 1) )
        {
            // too high or too low in the Z axis, but inside XY
            p[0] = w[0];
            p[1] = w[1];
        }
        else
        {
            // outside in Z and XY
            Polygon::project(mPoints, nPoints, w[0], w[1], p[0], p[1], nX, nY);
        }
        p[2] = (w[2]>0) ? height : -height;
    }
    else
    {
        Polygon::project(mPoints, nPoints, w[0], w[1], p[0], p[1], nX, nY);
        
        if ( Polygon::inside(mPoints, nPoints, w[0], w[1], 1) )
        {
            // inside in the Z axis and the XY polygon: compare distances
            
            real hdis = (w[0]-p[0])*(w[0]-p[0]) + (w[1]-p[1])*(w[1]-p[1]);
            
            //we are inside in both the Z and XY sense, we compare the distances
            //to the top/bottom planes, and to the sides of the polygon
            //calculate the distance to the top/bottom planes:
            real vdis = height - fabs(w[2]);
            if ( vdis * vdis < hdis )
            {
                p[0] = w[0];
                p[1] = w[1];
                p[2] = (w[2]>0) ? height : -height;
                return;
            }
            else
            {
                p[2] = w[2];
            }
        }
        else 
        {
            // outsize in XY, inside in Z
            p[2] = w[2];
        }
    }
    
#endif
}


/**
 The current procedure tests the model-points of fibers against the segments of the polygon.
 This fails for non-convext polygon since the re-entrant corners can intersect the fibers.
 
 @todo Also project re-entrant polygon corners on the segments of the Fiber.
 */
void SpacePolygon::setInteraction(Vector const& pos, PointExact const& pe, Meca & meca, real stiff) const
{    
#if ( DIM > 1 )
    Matrix::index_type inx = DIM * pe.matIndex();
    
    real pX, pY, nX, nY;
    int edg = Polygon::project(mPoints, nPoints, pos.XX, pos.YY, pX, pY, nX, nY);
    
#if ( DIM == 3 )
    
    if ( pos.ZZ >= height )
    {
        meca.mC(inx+2, inx+2) -= stiff;
        meca.base(inx+2)      += stiff * height;
        if ( Polygon::inside(mPoints, nPoints, pos.XX, pos.YY, 1) )
            return;
    }
    else if ( pos.ZZ <= -height )
    {
        meca.mC(inx+2, inx+2) -= stiff;
        meca.base(inx+2)      -= stiff * height;
        if ( Polygon::inside(mPoints, nPoints, pos.XX, pos.YY, 1) )
            return;
    }
    else
    {
        // Compare distance to top/bottom plate, and distance to edge in XY plane
        real vdis = height - fabs(pos.ZZ);
        real hdis = (pos.XX-pX)*(pos.XX-pX) + (pos.YY-pY)*(pos.YY-pY);
        
        if ( vdis * vdis < hdis  &&  Polygon::inside(mPoints, nPoints, pos.XX, pos.YY, 1) )
        {
            if ( pos.ZZ >= 0 )
            {
                meca.mC(inx+2, inx+2) -= stiff;
                meca.base(inx+2)      += stiff * height;
            }
            else
            {
                meca.mC(inx+2, inx+2) -= stiff;
                meca.base(inx+2)      -= stiff * height;
            }
            return;
        }
    }

#endif

    if ( edg )
    {
        // projection on an edge of normal (nX, nY) already normalized
        const real pr = ( pX * nX + pY * nY ) * stiff;
        
        meca.mC(inx, inx) -= nX * nX * stiff;
        meca.base(inx)    += nX * pr;

        meca.mC(inx  , inx+1) -= nX * nY * stiff;
        meca.mC(inx+1, inx+1) -= nY * nY * stiff;
        meca.base(inx+1)      += nY * pr;
    }
    else
    {
        // projection on a vertex:
#if ( DIM == 2 )
        meca.mB(pe.matIndex(), pe.matIndex()) -= stiff;
#elif ( DIM == 3 )
        meca.mC(inx,   inx  ) -= stiff;
        meca.mC(inx+1, inx+1) -= stiff;
#endif
        meca.base(inx  )  += stiff * pX;
        meca.base(inx+1)  += stiff * pY;
    }
#endif
}


void SpacePolygon::setInteraction(Vector const& pos, PointExact const& pe, real rad, Meca & meca, real stiff) const
{
    setInteraction(pos, pe, meca, stiff);
    std::cerr << "unfinished SpacePolygon::setInteractions(with radius)\n";
}

//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY
#include "opengl.h"
#include "gle.h"

bool SpacePolygon::display() const
{
#if ( 0 )
    char tmp[8];
    for ( unsigned int n=0; n < nPoints; n++ )
    {
        snprintf(tmp, sizeof(tmp), "%i", n);
        Vector p(mPoints[n].x, mPoints[n].y );
        gle::gleDrawText(p, tmp, GLUT_BITMAP_HELVETICA_10);
    }
#endif
    
    glBegin(GL_LINE_LOOP);
    for ( unsigned int n=0; n < nPoints; n++ )
    {
#if ( DIM == 3 )
        gle::gleVertex( mPoints[n].x, mPoints[n].y, -height );
#else
        gle::gleVertex( mPoints[n].x, mPoints[n].y );
#endif
    }
    glEnd();
    
#if ( DIM == 3 )
    //display top of generalized cylinder
    glBegin(GL_LINE_LOOP);
    for ( unsigned int n=0; n < nPoints; n++ )
    {
        gle::gleVertex( mPoints[n].x, mPoints[n].y,  height );
    }
    glEnd();
    //display sides of generalized cylinder
    glBegin(GL_LINES);
    for ( unsigned int n=0; n < nPoints; n++ )
    {
        gle::gleVertex( mPoints[n].x, mPoints[n].y, -height );
        gle::gleVertex( mPoints[n].x, mPoints[n].y,  height );
    }
    glEnd();
#endif
    
    return true;
}

#else

bool SpacePolygon::display() const
{
    return false;
}

#endif
