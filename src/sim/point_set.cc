// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "point_set.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "point_interpolated.h"
#include "space.h"
#include "modulo.h"
#include "random.h"
#include "cblas.h"

extern Random RNG;

//------------------------------------------------------------------------------
void PointSet::psConstructor()
{
    psPos       = 0;
    psFor       = 0;
    psSize      = 0;
    psAllocated = 0;
    psCenter.set(0,0,0);
}


PointSet::PointSet()
{
    psConstructor();
}

//------------------------------------------------------------------------------

PointSet::PointSet(const PointSet & o)
{
    psConstructor();
    allocatePoints( o.nbPoints() );
    psSize = o.psSize;
    for ( unsigned int p = 0; p < DIM*psSize; ++p )
        psPos[p] = o.psPos[p];
}


PointSet& PointSet::operator =(const PointSet& o)
{
    allocatePoints( o.nbPoints() );
    psSize = o.psSize;
    for ( unsigned int p = 0; p < DIM*psSize; ++p )
        psPos[p] = o.psPos[p];
    return *this;
}

//------------------------------------------------------------------------------
/** allocate(size) ensures that the set can hold `size` points
it returns the size if new memory was allocated 
*/
unsigned int PointSet::allocatePoints(const unsigned int nbp)
{
    if ( psAllocated < nbp )
    {
        // Keep memory aligned to 32 bytes:
        const unsigned chunk = 32 / sizeof(real);
        // make a multiple of chunk to align pointers:
        const unsigned size = ( nbp + chunk - 1 ) & -chunk;
        //std::cerr << "PointSet::allocatePoints(" << nbp << ") allocates " << size << std::endl;

        real* mem = new real[DIM*size];
        
        if ( psPos )
        {
            //copy the current position to the new array
            for ( unsigned int p = 0; p < DIM*psAllocated; ++p )
                mem[p] = psPos[p];
            delete[] psPos;
        }
        psPos = mem;
        psAllocated = size;
        return size;
    }
    
    return 0;
}


void PointSet::deallocatePoints()
{
    if ( psPos )
    {
        delete[] psPos;
        psPos = 0;
    }
    psFor = 0;
    psAllocated = 0;
    psSize = 0;
}

//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Modifying points

void PointSet::setPoint(const unsigned int indx, Vector const& w )
{
    assert_true( indx < psAllocated );
    assert_true( indx < psSize );
    
    psPos[DIM*indx  ] = w.XX;
#if ( DIM > 1 )
    psPos[DIM*indx+1] = w.YY;
#endif
#if ( DIM > 2 )
    psPos[DIM*indx+2] = w.ZZ;
#endif
}


unsigned int PointSet::addPoint( Vector const& w )
{
    allocatePoints(psSize+1);
    unsigned int indx = psSize++;
    
    psPos[DIM*indx  ] = w.XX;
#if ( DIM > 1 )
    psPos[DIM*indx+1] = w.YY;
#endif
#if ( DIM > 2 )
    psPos[DIM*indx+2] = w.ZZ;
#endif
    return indx;
}


void PointSet::removePoint(const unsigned int p)
{
    assert_true( p < psSize );
    
    //move part of the array down by DIM, to erase point p 
    for ( unsigned int ii = DIM*(p+1); ii < DIM*psSize; ++ii )
        psPos[ii-DIM]  =  psPos[ii];
    
    --psSize;
}

//------------------------------------------------------------------------------
/**
 shifts ending-part of the array to indices starting at 0.
*/
void PointSet::truncateM(const unsigned int p)
{
    assert_true( p < psSize - 1 );

    unsigned int np = psSize - p;
    
    for ( unsigned int ii = 0; ii < DIM*np; ++ii )
        psPos[ii] = psPos[ii+DIM*p];
    
    psSize = np;
}

/**
 erase higher indices of array
*/
void PointSet::truncateP(const unsigned int p)
{
    assert_true( p < psSize );
    assert_true( p > 0 );
    
    psSize = p+1;
}

//------------------------------------------------------------------------------

void PointSet::resetPoints()
{
    if ( psPos )
    {
        for ( unsigned int p = 0; p < DIM*psAllocated; ++p )
            psPos[p] = 0;
    }
}


void PointSet::addNoise( const real amount )
{
    for ( unsigned int p = 0; p < DIM*psSize; ++p )
        psPos[p] += amount * RNG.sreal();
}


void PointSet::translate( Vector const& T )
{
    for ( unsigned int p = 0; p < DIM*psSize;  )
    {
        psPos[p++] += T.XX;
#if ( DIM > 1 )
        psPos[p++] += T.YY;
#endif
#if ( DIM > 2 )
        psPos[p++] += T.ZZ;
#endif
    }
}


void PointSet::rotate( Rotation const& T )
{
    for ( unsigned int p = 0; p < DIM*psSize; p += DIM)
        T.vecMul( &psPos[p] );
}


//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Export/Inport


void PointSet::putPoints(real * x) const
{
    blas_xcopy(DIM*nbPoints(), psPos, 1, x, 1);
}


void PointSet::getPoints(const real * x)
{
    blas_xcopy(DIM*nbPoints(), x, 1, psPos, 1);
}


Vector PointSet::netForce(const unsigned p) const
{
    assert_true( p < psSize );
    if ( psFor )
        return Vector::make(psFor+DIM*p);
    else
        return Vector(0,0,0);
}

//------------------------------------------------------------------------------
/**
 Calculate the center of gravity of all points
 */
Vector PointSet::position() const
{
    Vector result(0,0,0);
    for ( unsigned int p = 0; p < DIM*psSize; p += DIM )
    {
        result.XX += psPos[p  ];
#if ( DIM > 1 )
        result.YY += psPos[p+1];
#endif
#if ( DIM > 2 )
        result.ZZ += psPos[p+2];
#endif
    }
    if ( psSize > 1 )
        result /= real( psSize );
    return result;
}

/**
 Calculate cc[DIM] = sum( psPos )
 */
void PointSet::calculateMomentum(real cc[], bool divide)
{
    for ( unsigned int d = 0; d < DIM; ++d )
        cc[d] = psPos[d];
    
    // calculate first and second moments:
    for ( unsigned int p = DIM; p < DIM*psSize; p += DIM )
    {
        cc[0] += psPos[p  ];
#if ( DIM > 1 )
        cc[1] += psPos[p+1];
#endif
#if ( DIM > 2 )
        cc[2] += psPos[p+2];
#endif
    }
    
    if ( divide )
    {
        const real N = 1.0 / psSize;
        for ( int d = 0; d < DIM; ++d )
            cc[d] *= N;
    }
}

/**
 Calculate cc[DIM] = sum( psPos ) and pp[DIM] = sum( psPos * psPos );
 */
void PointSet::calculateMomentum(real cc[], real pp[], bool divide)
{
    for ( unsigned int d = 0; d < DIM; ++d )
    {
        cc[d] = psPos[d];
        pp[d] = psPos[d] * psPos[d];
    }
    
    // calculate first and second moments:
    for ( unsigned int p = DIM; p < DIM*psSize; p += DIM )
    {
        cc[0] += psPos[p  ];
        pp[0] += psPos[p  ] * psPos[p  ];
#if ( DIM > 1 )
        cc[1] += psPos[p+1];
        pp[1] += psPos[p+1] * psPos[p+1];
#endif
#if ( DIM > 2 )
        cc[2] += psPos[p+2];
        pp[2] += psPos[p+2] * psPos[p+2];
#endif
    }
    
    if ( divide )
    {
        const real N = 1.0 / psSize;
        for ( int d = 0; d < DIM; ++d )
            cc[d] *= N;
    }    
}



void PointSet::foldPosition( const Modulo * s )
{
    Vector off;
    psCenter = position();
    s->foldOffset(psCenter, off);
    if ( off.norm_inf() > REAL_EPSILON )
        translate(-off);
}


//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Read/write



void PointSet::write(OutputWrapper& out) const
{
    out.writeUInt16(psSize);
    for ( unsigned int p = 0; p < psSize ; ++p )
        out.writeFloatVector(psPos+DIM*p, DIM, '\n');
}



void PointSet::read(InputWrapper & in, Simul&)
{
    try {
        unsigned int nb = in.readUInt16();
        allocatePoints( nb );
    
        //we reset the point for a clean start:
        resetPoints();
        
        psSize = nb;
#if ( 1 )
        for ( unsigned int p = 0; p < nb ; ++p )
            in.readFloatVector(psPos+DIM*p, DIM);
#else
        in.readFloatVector(psPos, nb, DIM);
#endif
    }
    catch( Exception & e ) {
        e << ", in PointSet::read()";
        psSize = 0;
        throw;
    }
}
