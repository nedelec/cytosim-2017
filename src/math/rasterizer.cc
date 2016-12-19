// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "assert_macro.h"
#include "rasterizer.h"
#include <cmath>


//keyword DISPLAY is defined for compiling test_rasterizer.cc
// to add display of things for visual testing
#ifdef DISPLAY
#include "opengl.h"
#endif

bool rasterizer_draw_things = 1;

//------------------------------------------------------------------------------

#if ( 0 )

#include <iostream>
#include <iomanip>

void printPolygon(std::ostream & os, int nb, const real xy[])
{
    os << "Polygon  ";
    for ( int ii = 0; ii < nb; ++ii )
    {
        os << std::setw(8) << std::setprecision(2) << std::fixed << xy[2*ii] << "  ";
        os << std::setw(8) << std::setprecision(2) << std::fixed << xy[2*ii+1] << "   ";
    }
    os << std::endl;
}

#endif



inline void SWAP2(real * a, real * b)
{
    real w0=a[0]; a[0]=b[0]; b[0]=w0;
    real w1=a[1]; a[1]=b[1]; b[1]=w1;
}


inline void SWAP4(real * a, real * b)
{
    real w0=a[0]; a[0]=b[0]; b[0]=w0;
    real w1=a[1]; a[1]=b[1]; b[1]=w1;
    real w2=a[2]; a[2]=b[2]; b[2]=w2;
    real w3=a[3]; a[3]=b[3]; b[3]=w3;
}

//------------------------------------------------------------------------------

unsigned int Rasterizer::convexHull2D(unsigned int nbp, real pts[])
{
    const unsigned int LD = 2;
    
    //---------- find the bottom and top points:
    unsigned int inx = 0, top = 0;
    real y_bot = pts[LD*inx+1], y_top = pts[LD*top+1];
    
    for ( unsigned int ii = 1; ii < nbp; ++ii )
    {
        if ( pts[LD*ii+1] < y_bot || ( pts[LD*ii+1] == y_bot  &&  pts[LD*ii] > pts[LD*inx] ) )
        {
            inx = ii;
            y_bot = pts[LD*inx+1];
        }
        if ( pts[LD*ii+1] > y_top || ( pts[LD*ii+1] == y_top  &&  pts[LD*ii] < pts[LD*top] ) )
        {
            top = ii;
            y_top = pts[LD*top+1];
        }
    }
    
    if ( inx == top )  //all points are equal ?
        return 1;
    
    //put the bottom point at position zero:
    if ( inx )
        SWAP2( pts, pts+LD*inx );
    
    //put the top point last:
    if ( top )
        SWAP2( pts+LD*(nbp-1), pts+LD*top );
    else  //we have to account for the previous swap
        SWAP2( pts+LD*(nbp-1), pts+LD*inx );
    
    inx = 0;
    top = nbp-1;
    
    unsigned int nxt;
    real dx, dy, dxt, dyt;
    
    // wrap upward on the right side of the hull
    do {
        
        nxt = top;
        dx = pts[LD*top  ] - pts[LD*inx  ];
        dy = pts[LD*top+1] - pts[LD*inx+1];
        
        for ( unsigned int ii = inx+1; ii < top; ++ii )
        {
            dxt = pts[LD*ii  ] - pts[LD*inx  ];
            dyt = pts[LD*ii+1] - pts[LD*inx+1];
            if ( dxt * dy > dyt * dx )
            {
                nxt = ii;
                dx = dxt;
                dy = dyt;
            }
        }
        
        ++inx;
        SWAP2( pts+LD*inx, pts+LD*nxt );
        
    } while ( nxt != top );
    
    // wrap downward the left side of the hull
    while ( 1 )
    {
        nxt = 0;
        dx = pts[0] - pts[LD*inx  ];
        dy = pts[1] - pts[LD*inx+1];
        
        for ( unsigned int ii = inx+1; ii < nbp; ++ii )
        {
            dxt = pts[LD*ii  ] - pts[LD*inx  ];
            dyt = pts[LD*ii+1] - pts[LD*inx+1];
            if ( dxt * dy > dyt * dx ){
                nxt = ii;
                dx = dxt;
                dy = dyt;
            }
        }
        if ( nxt == 0 )
            break;
        ++inx;
        SWAP2( pts+LD*inx, pts+LD*nxt );
        
    }
    
    return inx+1;
}


//------------------------------------------------------------------------------

unsigned int Rasterizer::convexHull2D_4( unsigned int nbp, real pts[] )
{
    const unsigned int LD = 4;
    
    //---------- find the bottom and top points:
    unsigned int inx = 0, top = 0;
    real y_bot = pts[LD*inx+1], y_top = pts[LD*top+1];
    
    for ( unsigned int ii = 1; ii < nbp; ++ii )
    {
        if ( pts[LD*ii+1] < y_bot || ( pts[LD*ii+1] == y_bot  &&  pts[LD*ii] > pts[LD*inx] ) )
        {
            inx = ii;
            y_bot = pts[LD*inx+1];
        }
        if ( pts[LD*ii+1] > y_top || ( pts[LD*ii+1] == y_top  &&  pts[LD*ii] < pts[LD*top] ) )
        {
            top = ii;
            y_top = pts[LD*top+1];
        }
    }
    
    if ( inx == top )  //all points are equal ?
        return 1;
    
    //put the bottom point at position zero:
    if ( inx )
        SWAP4( pts, pts+LD*inx );
    
    //put the top point last:
    if ( top )
        SWAP4( pts+LD*(nbp-1), pts+LD*top );
    else  //we have to account for the previous swap
        SWAP4( pts+LD*(nbp-1), pts+LD*inx );
    
    inx = 0;
    top = nbp-1;
    
    unsigned int nxt;
    real dx, dy, dxt, dyt;
    
    // wrap upward on the right side of the hull
    do {
        
        nxt = top;
        dx = pts[LD*top  ] - pts[LD*inx  ];
        dy = pts[LD*top+1] - pts[LD*inx+1];
        
        for ( unsigned int ii = inx+1; ii < top; ++ii )
        {
            dxt = pts[LD*ii  ] - pts[LD*inx  ];
            dyt = pts[LD*ii+1] - pts[LD*inx+1];
            if ( dxt * dy > dyt * dx )
            {
                nxt = ii;
                dx = dxt;
                dy = dyt;
            }
        }
        
        ++inx;
        SWAP4( pts+LD*inx, pts+LD*nxt );
        
    } while ( nxt != top );
    
    // wrap downward the left side of the hull
    while ( 1 )
    {
        nxt = 0;
        dx = pts[0] - pts[LD*inx  ];
        dy = pts[1] - pts[LD*inx+1];
        
        for ( unsigned int ii = inx+1; ii < nbp; ++ii )
        {
            dxt = pts[LD*ii  ] - pts[LD*inx  ];
            dyt = pts[LD*ii+1] - pts[LD*inx+1];
            if ( dxt * dy > dyt * dx ){
                nxt = ii;
                dx = dxt;
                dy = dyt;
            }
        }
        if ( nxt == 0 )
            break;
        ++inx;
        SWAP4( pts+LD*inx, pts+LD*nxt );
        
    }
    
    return inx+1;
}


//==============================================================================
//                             1D
//==============================================================================
#pragma mark 1D

void Rasterizer::paintFatLine1D(void (*paint)(int, int, int, int, void*, void*),
                                void * arg1, void * arg2,
                                const real p[], const real q[], const real radius,
                                const real offset[], const real delta[])
{
    int inf, sup;
    
    if ( p[0] > q[0] )
    {
        inf = (int) ceil( ( q[0] - radius - offset[0] ) * delta[0] );
        sup = (int)floor( ( p[0] + radius - offset[0] ) * delta[0] );
    }
    else
    {
        inf = (int) ceil( ( p[0] - radius - offset[0] ) * delta[0] );
        sup = (int)floor( ( q[0] + radius - offset[0] ) * delta[0] );
    }
    
    paint(inf, sup, 0, 0, arg1, arg2);
}

//==============================================================================
//                               2D
//==============================================================================
#pragma mark 2D

void Rasterizer::paintPolygon2D(void (*paint)(int, int, int, int, void*, void*),
                                void * arg1, void * arg2,
                                const unsigned int nbpts, const real xy[],
                                const int zz)
{
    const unsigned int LD=2;
    
#ifdef DISPLAY
    if ( rasterizer_draw_things )
    {
        glLineWidth(1);
        glColor3f(0.0, 0.8, 0.0);
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, 0x000F);
        glBegin(GL_LINE_LOOP);
        for ( unsigned int ii = 0; ii < nbpts; ++ii )
            glVertex3d( xy[LD*ii], xy[LD*ii+1], zz );
        glEnd();
        glDisable(GL_LINE_STIPPLE);
    }
#endif
    
    unsigned int iR = 0, iL;
    for ( unsigned int ii = 0; ii < nbpts; ++ii )
        if ( xy[LD*ii+1] < xy[LD*iR+1] )
            iR = ii;
    iL = iR;
    
    real xxR = 0, yyR = 0, dxxR = 0, xxnR = xy[LD*iR], yynR = xy[LD*iR+1];
    real xxL = 0, yyL = 0, dxxL = 0, xxnL = xy[LD*iL], yynL = xy[LD*iL+1];
    
#ifdef DISPLAY
    if ( rasterizer_draw_things )
    {
        glPointSize(9);
        glColor3f(1.0, 1.0, 0.0);
        glBegin(GL_POINTS);
        glVertex3d(xxnR, yynR, zz);
        glEnd();
    }
#endif
    
    // start on the line just above the bottom point
    int yy = (int)ceil(yynR);
    
    unsigned int cnt = 0;
    while ( true )
    {
        //std::cerr << "section at Y = " << yy << std::endl;
        
        // find right edge:
        if ( yynR <= yy )
        {
            do {
                xxR   = xxnR;
                yyR   = yynR;
                
                if ( ++cnt > nbpts )
                    return;
                if ( ++iR >= nbpts )
                    iR = 0;
                
                xxnR  = xy[LD*iR  ];
                yynR  = xy[LD*iR+1];
                
            } while ( yynR <= yy );
            
            dxxR = ( xxnR - xxR ) / ( yynR - yyR );
            xxR += dxxR * ( yy - yyR );
        }
        
        // find left edge:
        if ( yynL <= yy )
        {
            do {
                xxL   = xxnL;
                yyL   = yynL;
                
                if ( ++cnt > nbpts )
                    return;
                if ( iL == 0 )
                    iL = nbpts-1;
                else
                    --iL;
                
                xxnL  = xy[LD*iL  ];
                yynL  = xy[LD*iL+1];
                
            } while ( yynL <= yy );
            
            dxxL = ( xxnL - xxL ) / ( yynL - yyL );
            xxL += dxxL * ( yy - yyL );
        }
        
        // index of the last line without changing edges:
        int yym = (int)floor( (yynL<yynR) ? yynL : yynR );
        
        for ( ; yy <= yym; ++yy )
        {
            int inf = (int) ceil(xxL);
            int sup = (int)floor(xxR);
            if ( inf <= sup )
            {
                // draw the horizontal line at yy:
                paint(inf, sup, yy, zz, arg1, arg2);
            }
            xxL += dxxL;
            xxR += dxxR;
        }
    }
}



void Rasterizer::paintPolygon2D_4(void (*paint)(int, int, int, int, void*, void*),
                                  void * arg1, void * arg2,
                                  const unsigned int nbpts, const real xy[],
                                  const int zz)
{
    const unsigned int LD=4;
    
#ifdef DISPLAY
    if ( rasterizer_draw_things )
    {
        glLineWidth(1);
        glColor3f(0.0, 0.8, 0.0);
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, 0x000F);
        glBegin(GL_LINE_LOOP);
        for ( unsigned int ii = 0; ii < nbpts; ++ii )
            glVertex3d( xy[LD*ii], xy[LD*ii+1], zz );
        glEnd();
        glDisable(GL_LINE_STIPPLE);
    }
#endif
    
    real xxR = 0, yyR = 0, dxxR = 0, xxnR = xy[0], yynR = xy[1];
    real xxL = 0, yyL = 0, dxxL = 0, xxnL = xy[0], yynL = xy[1];
    
    unsigned int iR = 0, iL = nbpts;
    
    // start on the line just above the bottom point
    int yy = (int)ceil(yynR);
    
    while ( true )
    {
        // find right edge:
        if ( yynR <= yy )
        {
            do {
                
                if ( ++iR > iL )
                    return;
                xxR  = xxnR;
                yyR  = yynR;
                xxnR = xy[LD*iR  ];
                yynR = xy[LD*iR+1];
                
            } while ( yynR <= yy );
            
            dxxR = ( xxnR - xxR ) / ( yynR - yyR );
            xxR += dxxR * ( yy - yyR );
        }
        
        // find left edge:
        if ( yynL <= yy )
        {
            do {
                
                if ( --iL < iR )
                    return;
                xxL  = xxnL;
                yyL  = yynL;
                xxnL = xy[LD*iL  ];
                yynL = xy[LD*iL+1];
                
            } while ( yynL <= yy );
            
            dxxL = ( xxnL - xxL ) / ( yynL - yyL );
            xxL += dxxL * ( yy - yyL );
        }
        
        // index of the last line without changing edges:
        int yym = (int)floor( (yynL<yynR) ? yynL : yynR );
        
        for ( ; yy <= yym; ++yy )
        {
            int inf = (int) ceil(xxL);
            int sup = (int)floor(xxR);
            if ( inf <= sup )
            {
                // draw the horizontal line at yy:
                paint(inf, sup, yy, zz, arg1, arg2);
            }
            xxL += dxxL;
            xxR += dxxR;
        }
    }
}

//------------------------------------------------------------------------------
void Rasterizer::paintFatLine2D(void (*paint)(int, int, int, int, void*, void*),
                                void * arg1, void * arg2,
                                const real p[], const real q[], const real radius)
{
    real pts[8];
    
    real dx = q[0] - p[0];
    real dy = q[1] - p[1];
    
    real s = radius / sqrt( dx*dx + dy*dy );
    
    dx *= s;
    dy *= s;
    
    pts[0] = p[0] + dy;
    pts[1] = p[1] - dx;
    
    pts[2] = q[0] + dy;
    pts[3] = q[1] - dx;
    
    pts[4] = q[0] - dy;
    pts[5] = q[1] + dx;
    
    pts[6] = p[0] - dy;
    pts[7] = p[1] + dx;
    
    paintPolygon2D(paint, arg1, arg2, 4, pts);
}



//------------------------------------------------------------------------------
void Rasterizer::paintFatLine2D(void (*paint)(int, int, int, int, void*, void*),
                                void * arg1, void * arg2,
                                const real p[], const real q[], const real radius,
                                const real offset[], const real delta[],
                                real lengthPQ )
{
    real pqx = q[0] - p[0];
    real pqy = q[1] - p[1];
    
    //scaling is known beforhand for microtubules, since all rods have the
    //same length. The optional argument enables a small optimisation here
    if ( lengthPQ <= 0 )
        lengthPQ = sqrt( pqx * pqx + pqy * pqy );
    
    pqx *= radius * delta[0] / lengthPQ;
    pqy *= radius * delta[1] / lengthPQ;
    
    real endPx, endPy, endQx, endQy,  pts[8];
    
    endPx = ( p[0] - offset[0] ) * delta[0] - pqx;
    endPy = ( p[1] - offset[1] ) * delta[1] - pqy;
    
    endQx = ( q[0] - offset[0] ) * delta[0] + pqx;
    endQy = ( q[1] - offset[1] ) * delta[1] + pqy;
    
    pts[0] = endPx + pqy;
    pts[1] = endPy - pqx;
    
    pts[2] = endQx + pqy;
    pts[3] = endQy - pqx;
    
    pts[4] = endQx - pqy;
    pts[5] = endQy + pqx;
    
    pts[6] = endPx - pqy;
    pts[7] = endPy + pqx;
    
    paintPolygon2D(paint, arg1, arg2, 4, pts);
}



//------------------------------------------------------------------------------
void Rasterizer::paintBox2D(void (*paint)(int, int, int, int, void*, void*),
                            void * arg1, void * arg2,
                            const real p[], const real q[], const real radius,
                            const real offset[], const real delta[] )
{
    int inf[2], sup[2];
    
    for ( int d = 0; d < 2; ++d )
    {
        if ( p[d] > q[d] )
        {
            inf[d] = (int) ceil( ( q[d] - radius - offset[d] ) * delta[d] );
            sup[d] = (int)floor( ( p[d] + radius - offset[d] ) * delta[d] );
        }
        else
        {
            inf[d] = (int) ceil( ( p[d] - radius - offset[d] ) * delta[d] );
            sup[d] = (int)floor( ( q[d] + radius - offset[d] ) * delta[d] );
        }
    }
    
    for ( int yy = inf[1]; yy <= sup[1]; ++yy )
        paint(inf[0], sup[0], yy, 0, arg1, arg2);
}


//==============================================================================
//                               3D
//==============================================================================
#pragma mark 3D

/// function for qsort: compares the Z component of the two points
int rasterizer_comp_vector(const void * a, const void * b)
{
    const real * va = static_cast<const real*>(a);
    const real * vb = static_cast<const real*>(b);
    
    if ( va[2] > vb[2]) return  1;
    if ( va[2] < vb[2]) return -1;
    return 0;
}


//------------------------------------------------------------------------------
/*
 The polygon is the convex hull of the given points.
 Algorithm: we section at each integral Z:
 - find the intersection of all possible lines connecting two points,
 - find the convex hull of all these points,
 - call paintPolygon2D with the convex hull.
 .
 
 paintPolygon3D(..., Vertex pts[]) should be more efficient.
 */
void Rasterizer::paintPolygon3D(void (*paint)(int, int, int, int, void*, void*),
                                void * arg1, void * arg2,
                                const unsigned int nbpts,
                                real xyz[])
{
    const unsigned int LD = 4;
    if ( nbpts < 1 ) return;
    
    // first order the points in increasing Z:
    qsort( xyz, nbpts, 3*sizeof(real), &rasterizer_comp_vector );
    
    real * xy = new real[LD*nbpts*nbpts];
    real dxz, dyz, dzz;
    
    unsigned int nbl;
    unsigned int above = 0;
    
    int zz  = (int) ceil( xyz[2] );
    int zzn;
    
    while ( true )
    {
        //printf("restart at zz %4i\n", zz );
        
        //we move to the next point, ensuring that we will exit for sure...
        if ( ++above >= nbpts )
            goto cleanup;
        
        //find the first point strictly above the plane Z = zz:
        //the index of this point is (above-1)
        while ( xyz[3*above+2] <= zz )
            if ( ++above >= nbpts )
                goto cleanup;
        
        //the next time we have to recalculate the lines
        //is when this point will be below the plane Z = zzn:
        zzn = (int)ceil( xyz[3*above+2] );
        
        //set-up all the lines, which join any point below the plane
        //to any point above the plane:
        
        nbl = 0;
        
        for ( unsigned int ii = 0; ii < above; ++ii )
        {
            for (unsigned int jj = above; jj < nbpts; ++jj )
            {
                dzz = xyz[3*jj+2] - xyz[3*ii+2];
                assert_true( dzz > 0 );
                dxz = ( xyz[3*jj  ] - xyz[3*ii  ] ) / dzz;
                dyz = ( xyz[3*jj+1] - xyz[3*ii+1] ) / dzz;
                
                dzz = zz - xyz[3*ii+2];
                
                xy[LD*nbl  ] = xyz[3*ii  ] + dxz * dzz;
                xy[LD*nbl+1] = xyz[3*ii+1] + dyz * dzz;
                xy[LD*nbl+2] = dxz;
                xy[LD*nbl+3] = dyz;
                
                ++nbl;
                assert_true( nbl < nbpts*nbpts );
            }
        }
        
        //printf("Z %4i nxt Z %4i split point above %2i #lines %4i\n", zz, zzn, above, nbl);
        
        for ( ; zz < zzn; ++zz )
        {
            for ( unsigned int ii = 0; ii < nbl; ++ii )
            {
                xy[LD*ii  ]  +=  xy[LD*ii+2];
                xy[LD*ii+1]  +=  xy[LD*ii+3];
            }
            
            unsigned int nbp = convexHull2D_4(nbl, xy);
            paintPolygon2D_4(paint, arg1, arg2, nbp, xy, zz);
        }
    }
    
cleanup:
    delete [] xy;
}



//------------------------------------------------------------------------------
void Rasterizer::paintFatLine3D_old(void (*paint)(int, int, int, int, void*, void*),
                                    void * arg1, void * arg2,
                                    const real p[], const real q[], const real radius,
                                    const real offset[], const real delta[] )
{
    real n, pq[3];
    
    pq[0] = q[0] - p[0];
    pq[1] = q[1] - p[1];
    pq[2] = q[2] - p[2];
    
    //we normalize pq to norm=radius
    real  scaling = radius / sqrt( pq[0]*pq[0] + pq[1]*pq[1] + pq[2]*pq[2] );
    
    pq[0] *= scaling;
    pq[1] *= scaling;
    pq[2] *= scaling;
    
    // we will set-up two vectors A and B perpendicular to PQ,
    // we first set a perpendicular to PQ and one of the coordiate axis
    // discarding the dimension where PQ vector is smallest:
    real a[3], b[3];
    
    if ( fabs(pq[0]) < fabs(pq[1]) )
    {
        if ( fabs(pq[0]) < fabs(pq[2]) )
        {
            a[0] =  0.0;     a[1] = -pq[2];   a[2] =  pq[1];
        }
        else
        {
            a[0] = -pq[1];   a[1] =  pq[0];   a[2] =  0.0;
        }
        
    }
    else
    {
        if ( fabs(pq[1]) < fabs(pq[2]) )
        {
            a[0] = -pq[2];   a[1] =  0.0;     a[2] =  pq[0];
        }
        else
        {
            a[0] = -pq[1];   a[1] =  pq[0];   a[2] =  0.0;
        }
        
    }
    
    // vector B is set perpendicular to PQ and A by cross product
    b[0] = pq[1] * a[2] - pq[2] * a[1];
    b[1] = pq[2] * a[0] - pq[0] * a[2];
    b[2] = pq[0] * a[1] - pq[1] * a[0];
    
    //vectors A & B are now perpendicular to PQ, we normalize them:
    n = radius / sqrt( a[0]*a[0] + a[1]*a[1] + a[2]*a[2] );
    a[0] *= n * delta[0];
    a[1] *= n * delta[1];
    a[2] *= n * delta[2];
    
    n = radius / sqrt( b[0]*b[0] + b[1]*b[1] + b[2]*b[2] );
    b[0] *= n * delta[0];
    b[1] *= n * delta[1];
    b[2] *= n * delta[2];
    
    //we set up two points on the line PQ by extending the
    //segment PQ by radius on each side,
    //at the same time, we convert to the grid's coordinates:
    // grid coordinates = ( coordinates - min ) * delta
    real ends[6] = {
        ( p[0] - pq[0] - offset[0] ) * delta[0],
        ( p[1] - pq[1] - offset[1] ) * delta[1],
        ( p[2] - pq[2] - offset[2] ) * delta[2],
        ( q[0] + pq[0] - offset[0] ) * delta[0],
        ( q[1] + pq[1] - offset[1] ) * delta[1],
        ( q[2] + pq[2] - offset[2] ) * delta[2] };
    
    //last, we set 8 corners of a square volume aligned along PQ
    real pts[24] = {
        ends[0] + a[0], ends[1] + a[1], ends[2] + a[2],
        ends[0] + b[0], ends[1] + b[1], ends[2] + b[2],
        ends[0] - a[0], ends[1] - a[1], ends[2] - a[2],
        ends[0] - b[0], ends[1] - b[1], ends[2] - b[2],
        ends[3] + a[0], ends[4] + a[1], ends[5] + a[2],
        ends[3] + b[0], ends[4] + b[1], ends[5] + b[2],
        ends[3] - a[0], ends[4] - a[1], ends[5] - a[2],
        ends[3] - b[0], ends[4] - b[1], ends[5] - b[2] };
    
    paintPolygon3D(paint, arg1, arg2, 8, pts);
    
#ifdef DISPLAY
    if ( rasterizer_draw_things )
    {
        glPointSize(1);
        glColor3f(1.0, 1.0, 0.0);
        glBegin(GL_POINTS);
        for ( int ii=0; ii<=21; ii+=3)
            glVertex3d( pts[ii], pts[ii+1], pts[ii+2] );
        glColor3f(1.0, 1.0, 1.0);
        glEnd();
    }
#endif
}


//==============================================================================

/// function for qsort: compares the Z component of the two points
int rasterizer_comp_vertex(const void * a, const void * b)
{
    typedef const Rasterizer::Vertex cVertex;
    cVertex * va = static_cast<cVertex*>(a);
    cVertex * vb = static_cast<cVertex*>(b);
    
    if ( va->z > vb->z ) return  1;
    if ( va->z < vb->z ) return -1;
    return 0;
}


void Rasterizer::paintPolygon3D(void (*paint)(int, int, int, int, void*, void*),
                                void * arg1, void * arg2,
                                const unsigned int nbpts,
                                Vertex pts[])
{
    assert_true( nbpts > 1 );
    
#ifdef DISPLAY
    if ( rasterizer_draw_things )
    {
        //draw the vertex of the volume:
        glPointSize(6);
        glBegin(GL_POINTS);
        glColor3f(1.0, 0.0, 0.0);
        for ( unsigned int ii=0; ii<nbpts; ++ii )
            glVertex3d( pts[ii].x, pts[ii].y, pts[ii].z );
        glEnd();
        
        //draw the edges of the volume:
        glLineWidth(0.5);
        glBegin(GL_LINES);
        glColor3f(1.0, 1.0, 0.0);
        for ( unsigned int ii=0; ii<nbpts; ++ii )
        {
            for ( unsigned int jj=ii+1; jj<nbpts; ++jj )
            {
                if ( pts[ii].u  &  pts[jj].u )
                {
                    glVertex3d( pts[ii].x, pts[ii].y, pts[ii].z );
                    glVertex3d( pts[jj].x, pts[jj].y, pts[jj].z );
                }
            }
        }
        glEnd();
    }
#endif
    
    //order the points in increasing Z:
    qsort( pts, nbpts, sizeof(Vertex), &rasterizer_comp_vertex );
    
    //we can normally only cross four sides of a parallelogram in 3D
    //but in some degenerate cases, it can be more
    const unsigned int LD  = 4;
    const unsigned int max = 16;
    real xy[LD*max];
    
    unsigned int above = 0;
    int zz  = (int) ceil( pts[0].z );
    
    while ( ++above < nbpts )
    {
        //printf("restart at zz %4i\n", zz );
        
        //find the first point strictly above the plane Z = zz:
        //the index of this point is (above-1)
        while ( pts[above].z <= zz )
        {
            if ( ++above >= nbpts )
                return;
        }
        
        //the next time we have to recalculate the lines
        //is when pts[above] will be below the plane Z = zzn:
        int zzn = (int)ceil( pts[above].z );
        
        //number of edges crossing the plane at Z=zz;
        unsigned int nbl = 0;
        //set-up all the lines, which join any point below the plane
        //to any point above the plane, being a edge of the solid polygon:
        for ( unsigned int ii = 0; ii < above; ++ii )
        {
            for ( unsigned int jj = above; jj < nbpts; ++jj )
            {
                //test if [ii, jj] are joined:
                if ( pts[ii].u  &  pts[jj].u )
                {
                    real dzz = pts[jj].z - pts[ii].z;
                    
                    if ( dzz > 0 )
                    {
                        real dxz = ( pts[jj].x - pts[ii].x ) / dzz;
                        real dyz = ( pts[jj].y - pts[ii].y ) / dzz;
                        real dz  = zz - pts[ii].z;
                        
                        xy[LD*nbl  ] = pts[ii].x + dxz * dz;
                        xy[LD*nbl+1] = pts[ii].y + dyz * dz;
                        xy[LD*nbl+2] = dxz;
                        xy[LD*nbl+3] = dyz;
                        
                        ++nbl;
                        assert_true( nbl < max );
                    }
                }
            }
        }
        
        // the edges of the convex solid polygon should not intersect,
        // so we can take the convex hull only once here:
        bool need_hull = true;
        unsigned int nbp; //number of points in the hull.
        
        for ( ; zz < zzn; ++zz )
        {
            if ( need_hull )
            {
                //make the convex hull of the points from xy[]:
                nbp = convexHull2D_4(nbl, xy);
                //printf("zz %3i : nbp = %i\n", zz, nbp);
                
                //in the particular case where some points overlap, we might
                //loose them, in which case we need to redo the hull later
                need_hull = ( nbp != nbl );
            }
            
            paintPolygon2D_4(paint, arg1, arg2, nbp, xy, zz);
            
            //update the coordinates according to the slopes, for the next zz:
            for ( unsigned int ii = 0; ii < nbl; ++ii )
            {
                xy[LD*ii  ] += xy[LD*ii+2];
                xy[LD*ii+1] += xy[LD*ii+3];
            }
        }
    }
}


//------------------------------------------------------------------------------
void Rasterizer::paintFatLine3D(void (*paint)(int, int, int, int, void*, void*),
                                void * arg1, void * arg2,
                                const real p[], const real q[], const real radius,
                                const real offset[], const real delta[],
                                real lengthPQ )
{
    real pqx = q[0] - p[0];
    real pqy = q[1] - p[1];
    real pqz = q[2] - p[2];
    
    //we calculate the distance PQ if needed:
    if ( lengthPQ <= 0 )
        lengthPQ = sqrt( pqx*pqx + pqy*pqy + pqz*pqz );
    
    // we will set-up two vectors A and B perpendicular to PQ,
    // we first set A perpendicular to PQ and one of the coordiate axis
    // discarding the dimension where PQ vector is smallest:
    real ax, ay, az, an;
    if ( fabs(pqx) < fabs(pqy) )
    {
        if ( fabs(pqx) < fabs(pqz) )
        {
            ax =  0;  //pqx is the smallest
            ay = -pqz;
            az =  pqy;
            an =  sqrt( (pqy*pqy + pqz*pqz)*0.5 );
        }
        else
        {
            ax = -pqy;
            ay =  pqx;
            az =  0; //pqz is the smallest
            an =  sqrt( (pqx*pqx + pqy*pqy)*0.5 );
        }
    }
    else
    {
        if ( fabs(pqy) < fabs(pqz) )
        {
            ax = -pqz;
            ay =  0; //pqy is the smallest
            az =  pqx;
            an =  sqrt( (pqx*pqx + pqz*pqz)*0.5 );
        }
        else
        {
            ax = -pqy;
            ay =  pqx;
            az =  0; //pqz is the smallest
            an =  sqrt( (pqx*pqx + pqy*pqy)*0.5 );
        }
    }
    
    //normalize pq to norm=radius:
    const real pqs = radius / lengthPQ;
    pqx *= pqs;
    pqy *= pqs;
    pqz *= pqs;
    
    // vector B is set perpendicular to PQ and A by cross product.
    // we scale appropriately, to have a norm = sqrt(2)*radius
    real bx = ( pqy * az - pqz * ay ) * delta[0] / an;
    real by = ( pqz * ax - pqx * az ) * delta[1] / an;
    real bz = ( pqx * ay - pqy * ax ) * delta[2] / an;
    
    //normalize a to sqrt(2)*radius, and divide by delta[]:
    const real anr = radius / an;
    ax *= anr * delta[0];
    ay *= anr * delta[1];
    az *= anr * delta[2];
    
    //we set up two points on the line PQ by extending the
    //segment PQ by radius on each side,
    //at the same time, we convert to the grid's coordinates:
    // grid coordinates = ( coordinates - min ) / delta
    
    real endPx = ( p[0] - pqx - offset[0] ) * delta[0];
    real endPy = ( p[1] - pqy - offset[1] ) * delta[1];
    real endPz = ( p[2] - pqz - offset[2] ) * delta[2];
    
    real endQx = ( q[0] + pqx - offset[0] ) * delta[0];
    real endQy = ( q[1] + pqy - offset[1] ) * delta[1];
    real endQz = ( q[2] + pqz - offset[2] ) * delta[2];
    
    // set the vertex of a rectangular volume aligned along PQ
    Vertex pts[8];
    
    //the static matrix of vertices connections:
    //it defines which point are connected to form
    //the edges of the polygon.
    
    pts[0].x = endPx + ax;
    pts[0].y = endPy + ay;
    pts[0].z = endPz + az;
    pts[0].u = 25; //1+8+16;
    
    pts[1].x = endPx + bx;
    pts[1].y = endPy + by;
    pts[1].z = endPz + bz;
    pts[1].u = 35; //1+2+32;
    
    pts[2].x = endPx - ax;
    pts[2].y = endPy - ay;
    pts[2].z = endPz - az;
    pts[2].u = 70; //2+4+64;
    
    pts[3].x = endPx - bx;
    pts[3].y = endPy - by;
    pts[3].z = endPz - bz;
    pts[3].u = 140; //4+8+128;
    
    pts[4].x = endQx + ax;
    pts[4].y = endQy + ay;
    pts[4].z = endQz + az;
    pts[4].u = 2320; //16+256+2048;
    
    pts[5].x = endQx + bx;
    pts[5].y = endQy + by;
    pts[5].z = endQz + bz;
    pts[5].u = 800; //32+256+512;
    
    pts[6].x = endQx - ax;
    pts[6].y = endQy - ay;
    pts[6].z = endQz - az;
    pts[6].u = 1600; //64+512+1024;
    
    pts[7].x = endQx - bx;
    pts[7].y = endQy - by;
    pts[7].z = endQz - bz;
    pts[7].u = 3200; //128+1024+2048;
    
    //paint the volume:
    paintPolygon3D(paint, arg1, arg2, 8, pts);
}


//------------------------------------------------------------------------------
void Rasterizer::paintHexLine3D(void (*paint)(int, int, int, int, void*, void*),
                                void * arg1, void * arg2,
                                const real p[], const real q[], const real radius,
                                const real offset[], const real delta[],
                                real lengthPQ )
{
    real pqx = q[0] - p[0];
    real pqy = q[1] - p[1];
    real pqz = q[2] - p[2];
    
    //we calculate the distance PQ if needed:
    if ( lengthPQ <= 0 )
        lengthPQ = sqrt( pqx*pqx + pqy*pqy + pqz*pqz );
    
    // we will set-up two vectors A and B perpendicular to PQ,
    // we first set A perpendicular to PQ and one of the coordiate axis
    // discarding the dimension where PQ vector is smallest:
    real ax, ay, az, an;
    if ( fabs(pqx) < fabs(pqy) )
    {
        if ( fabs(pqx) < fabs(pqz) )
        {
            ax =  0;  //pqx is the smallest
            ay = -pqz;
            az =  pqy;
            an =  sqrt( (pqy*pqy + pqz*pqz)*0.75 );
        }
        else
        {
            ax = -pqy;
            ay =  pqx;
            az =  0; //pqz is the smallest
            an =  sqrt( (pqx*pqx + pqy*pqy)*0.75 );
        }
    }
    else
    {
        if ( fabs(pqy) < fabs(pqz) )
        {
            ax = -pqz;
            ay =  0; //pqy is the smallest
            az =  pqx;
            an =  sqrt( (pqx*pqx + pqz*pqz)*0.75 );
        }
        else
        {
            ax = -pqy;
            ay =  pqx;
            az =  0; //pqz is the smallest
            an =  sqrt( (pqx*pqx + pqy*pqy)*0.75 );
        }
    }
    
    //normalize pq to norm = radius:
    real sca = radius / lengthPQ;
    pqx *= sca;
    pqy *= sca;
    pqz *= sca;
    
    // vector D is set perpendicular to PQ and A by cross product.
    // we scale appropriately, to have a norm = sqrt(2)*radius
    sca = 0.5 * sqrt(3) / an;
    real cx = ( pqy * az - pqz * ay ) * sca * delta[0];
    real cy = ( pqz * ax - pqx * az ) * sca * delta[1];
    real cz = ( pqx * ay - pqy * ax ) * sca * delta[2];
    
    //normalize a to sqrt(2)*radius, and divide by delta[]:
    sca = radius / an;
    ax *= sca * delta[0];
    ay *= sca * delta[1];
    az *= sca * delta[2];
    
    // vectors B and C are (with A) the vertices of the hexagon
    real bx = cx + 0.5 * ax;
    real by = cy + 0.5 * ay;
    real bz = cz + 0.5 * az;
    
    cx = bx - ax;
    cy = by - ay;
    cz = bz - az;
    
    //we set up two points on the line PQ by extending the
    //segment PQ by radius on each side,
    //at the same time, we convert to the grid's coordinates:
    // grid coordinates = ( coordinates - min ) * delta
    
    real endPx = ( p[0] - pqx - offset[0] ) * delta[0];
    real endPy = ( p[1] - pqy - offset[1] ) * delta[1];
    real endPz = ( p[2] - pqz - offset[2] ) * delta[2];
    
    real endQx = ( q[0] + pqx - offset[0] ) * delta[0];
    real endQy = ( q[1] + pqy - offset[1] ) * delta[1];
    real endQz = ( q[2] + pqz - offset[2] ) * delta[2];
    
    // set the vertex of a rectangular volume aligned along PQ
    Vertex pts[12];
    
    //the static matrix of vertices connections:
    //it defines which point are connected to form
    //the edges of the polygon.
    
    pts[0].x = endPx + ax;
    pts[0].y = endPy + ay;
    pts[0].z = endPz + az;
    pts[0].u = 0x61; //97;
    
    pts[1].x = endPx + bx;
    pts[1].y = endPy + by;
    pts[1].z = endPz + bz;
    pts[1].u = 0x83; //131;
    
    pts[2].x = endPx + cx;
    pts[2].y = endPy + cy;
    pts[2].z = endPz + cz;
    pts[2].u = 0x106; //262;
    
    pts[3].x = endPx - ax;
    pts[3].y = endPy - ay;
    pts[3].z = endPz - az;
    pts[3].u = 0x20c; //524;
    
    pts[4].x = endPx - bx;
    pts[4].y = endPy - by;
    pts[4].z = endPz - bz;
    pts[4].u = 0x418; //1048;
    
    pts[5].x = endPx - cx;
    pts[5].y = endPy - cy;
    pts[5].z = endPz - cz;
    pts[5].u = 0x830; //2096;
    
    
    pts[6].x = endQx + ax;
    pts[6].y = endQy + ay;
    pts[6].z = endQz + az;
    pts[6].u = 0x21040; //135232;
    
    pts[7].x = endQx + bx;
    pts[7].y = endQy + by;
    pts[7].z = endQz + bz;
    pts[7].u = 0x3080; //12416;
    
    pts[8].x = endQx + cx;
    pts[8].y = endQy + cy;
    pts[8].z = endQz + cz;
    pts[8].u = 0x6100; //24832;
    
    pts[9].x = endQx - ax;
    pts[9].y = endQy - ay;
    pts[9].z = endQz - az;
    pts[9].u = 0xc200; //49664;
    
    pts[10].x = endQx - bx;
    pts[10].y = endQy - by;
    pts[10].z = endQz - bz;
    pts[10].u = 0x18400; //99328;
    
    pts[11].x = endQx - cx;
    pts[11].y = endQy - cy;
    pts[11].z = endQz - cz;
    pts[11].u = 0x30800; //198656;
    
    paintPolygon3D(paint, arg1, arg2, 12, pts);
}



//------------------------------------------------------------------------------
void Rasterizer::paintBox3D(void (*paint)(int, int, int, int, void*, void*),
                            void * arg1, void * arg2,
                            const real p[], const real q[], const real radius,
                            const real offset[], const real delta[] )
{
    int inf[3], sup[3];
    
    for ( int d = 0; d < 3; ++d )
    {
        if ( p[d] > q[d] )
        {
            inf[d] = (int) ceil( ( q[d] - radius - offset[d] ) * delta[d] );
            sup[d] = (int)floor( ( p[d] + radius - offset[d] ) * delta[d] );
        }
        else
        {
            inf[d] = (int) ceil( ( p[d] - radius - offset[d] ) * delta[d] );
            sup[d] = (int)floor( ( q[d] + radius - offset[d] ) * delta[d] );
        }
    }
    
    for ( int zz = inf[2]; zz <= sup[2]; ++zz )
        for ( int yy = inf[1]; yy <= sup[1]; ++yy )
            paint(inf[0], sup[0], yy, zz, arg1, arg2);
}

