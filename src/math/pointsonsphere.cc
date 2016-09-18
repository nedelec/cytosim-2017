// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "pointsonsphere.h"

extern Random RNG;


//------------------------------------------------------------------------------
PointsOnSphere::PointsOnSphere() : nPoints(0), mCoord(0, 0)
{
}

//------------------------------------------------------------------------------
PointsOnSphere::PointsOnSphere( int nbp ) : nPoints(0), mCoord(0, 0)
{
    distributePoints(nbp);
}

//------------------------------------------------------------------------------
PointsOnSphere::~PointsOnSphere( )
{
}

//------------------------------------------------------------------------------
void PointsOnSphere::copyCoordinatesOfPoint( real x[3], const unsigned int ii )
{
    x[0] = mCoord[3*ii+0];
    x[1] = mCoord[3*ii+1];
    x[2] = mCoord[3*ii+2];
}

//------------------------------------------------------------------------------
void PointsOnSphere::copyCoordinatesOfPoint( real* x, real* y, real* z, const unsigned int ii )
{
    *x = mCoord[3*ii+0];
    *y = mCoord[3*ii+1];
    *z = mCoord[3*ii+2];
}

//------------------------------------------------------------------------------
void PointsOnSphere::copyPositionsForAllPoints( real x[] )
{
    for ( unsigned int ii = 0; ii < 3*nPoints; ++ii )
        x[ii] = mCoord[ii];
}

//------------------------------------------------------------------------------
void PointsOnSphere::printAllPositions( FILE* file )
{
    for ( unsigned int ii = 0; ii < nPoints; ++ii )
        fprintf( file, "%f %f %f\n", mCoord[3*ii], mCoord[3*ii+1], mCoord[3*ii+2]);
}

//------------------------------------------------------------------------------
/** hypercube rejection method, calling the Random Number Generator RNG */
void PointsOnSphere::randomize(real P[3])
{
    real n;
    do {
        P[0] = RNG.sreal();
        P[1] = RNG.sreal();
        P[2] = RNG.sreal();
        n = P[0]*P[0] + P[1]*P[1] + P[2]*P[2];
        if ( n == 0 )
        {
            fprintf(stderr, "RNG may not be properly initialized");
        }
    } while ( n > 1.0 );
    
    n = sqrt(n);
    P[0] /= n;
    P[1] /= n;
    P[2] /= n;
}

//------------------------------------------------------------------------------
void PointsOnSphere::project(real P[3], const real S[3])
{
    real n = S[0]*S[0] + S[1]*S[1] + S[2]*S[2];
    if ( n > 0 ) 
    {
        n = sqrt(n);
        P[0] = S[0] / n;
        P[1] = S[1] / n;
        P[2] = S[2] / n;
    }
    else
    {
        randomize(P);
    }
}


//------------------------------------------------------------------------------
real PointsOnSphere::distance3( const real P[], const real Q[] )
{
    return sqrt( (P[0]-Q[0])*(P[0]-Q[0]) + (P[1]-Q[1])*(P[1]-Q[1]) + (P[2]-Q[2])*(P[2]-Q[2]) );
}

//------------------------------------------------------------------------------
real PointsOnSphere::distance3Sqr( const real P[], const real Q[] )
{
    return (P[0]-Q[0])*(P[0]-Q[0]) + (P[1]-Q[1])*(P[1]-Q[1]) + (P[2]-Q[2])*(P[2]-Q[2]);
}

//------------------------------------------------------------------------------
/**
 With N points on the sphere according to a triagular lattice, 
 each of ~2N triangles should occupy an area of S = 4*PI/2*N, 
 and the distance between points should be ~2 * sqrt(S/sqrt(3)).
 */
real PointsOnSphere::expectedDistance(int n)
{
    real surface = 2 * 3.14159 / n;
    return 2 * sqrt( surface / sqrt(3) );
}

real PointsOnSphere::minimumDistance()
{
    real dist, result = 4.;
    for ( unsigned int ii = 1; ii < nPoints; ++ii )
    {
        for ( unsigned int jj = 0; jj < ii; ++jj )
        {
            dist = distance3Sqr( mCoord + 3 * ii, mCoord + 3 * jj );
            if ( dist < result )
                result = dist;
        }
    }
    return sqrt(result);
}

//------------------------------------------------------------------------------
void PointsOnSphere::scale(const real factor)
{
    for ( unsigned int ii = 0; ii < 3*nPoints; ++ii )
        mCoord[ii] *= factor;
}


//------------------------------------------------------------------------------
real PointsOnSphere::coulombEnergy( const real P[] )
{
    real dist, result = 0;
    for ( unsigned int ii = 1; ii < nPoints; ++ii )
    {
        for ( unsigned int jj = 0; jj < ii; ++jj )
        {
            dist = distance3( P + 3 * ii, P + 3 * jj );
            if ( dist > 0 ) result += 1.0 / dist;
        }
    }
    return result;
}

//------------------------------------------------------------------------------
void PointsOnSphere::calculateForces( real forces[], real threshold )
{
    real dx[3];
    real dist;
    
    //--------- reset forces:
    for ( unsigned int ii = 0; ii < 3 * nPoints; ++ii )
        forces[ii] = 0;
    
    //--------- calculate coulomb pair interactions:
    // first particle is ii, second one is jj:
    for ( unsigned int ii = 1; ii < nPoints; ++ii )
    {
        for ( unsigned int jj = 0; jj < ii; ++jj )
        {
            //calculate vector and distance^2 between from jj to ii
            dist = 0;
            for ( int dd = 0; dd < 3 ; ++dd )
            {
                dx[dd] = mCoord[3*ii+dd] - mCoord[3*jj+dd];
                dist += dx[dd] * dx[dd];
            }
            
            if ( dist == 0 )
            {   //if ii and jj overlap, we use a random force
                for ( int dd = 0 ; dd < 3; ++dd )
                {
                    dx[dd] = 0.1 * RNG.sreal();
                    forces[3*ii+dd] += dx[dd];
                    forces[3*jj+dd] -= dx[dd];
                }
            }
            else if ( dist < threshold )
            {
                // points do not overlap:
                //force = vector / r^3, but here dist = r^2
                dist = 1.0 / ( dist * sqrt(dist) );
                //update forces for jj and ii:
                for ( unsigned int dd = 0 ; dd < 3; ++dd )
                {
                    dx[dd] *= dist;
                    forces[3*ii+dd] += dx[dd];
                    forces[3*jj+dd] -= dx[dd];
                }
            }
        }
    }
    

#if ( 1 )
    /*
     Remove centripede contribution of forces:
     assuming here that points are already on the sphere (norm=1)
     ( the algorithm converge even without this, but slower )
     */
    for ( unsigned int ii = 0; ii < nPoints; ++ii )
    {
        dist = 0;
        for ( int dd = 0; dd < 3; ++dd )
            dist += mCoord[3*ii+dd] * forces[3*ii+dd];
        
        for ( int dd = 0; dd < 3; ++dd )
            forces[3*ii+dd] -= dist * mCoord[3*ii+dd];
    }
#endif
}
//------------------------------------------------------------------------------
/** move the points in the direction of the forces, with scaling factor S */
void PointsOnSphere::movePoints( real Pnew[], real Pold[], real forces[], real S )
{
    for ( unsigned int ii = 0; ii < nPoints; ++ii )
    {
        real tmp[3];
        for ( int dd = 0; dd < 3; ++dd )
            tmp[dd] = Pold[3*ii+dd] + S * forces[3*ii+dd];
        
        project(Pnew+3*ii, tmp);
    }
}


//------------------------------------------------------------------------------
// creates a relatively even distribution of nbp points on the sphere
// the coordinates are stored in real array mCoord
int PointsOnSphere::distributePoints( unsigned int nbp, real precision )
{
    //reallocate the array if needed:
    mCoord.allocate(3*nbp);
    nPoints = nbp;
    
    // the precision is rescaled with the expected distance:
    real distance = expectedDistance(nbp);
    
    /* 
     Threshold cut-off for repulsive force:
     The best results are obtained for threshold > 2
     */
    real threshold = 10 * distance;
    real mag = 0.1 * distance * distance * distance * distance / nPoints;
    precision *= mag;

    //------------ distribute the points randomly on the sphere:
    for ( unsigned int ii = 0; ii < nPoints; ++ii )
        randomize(mCoord+ii*3);
    
    //--------- for one point only, we return:
    if ( nPoints < 2 )
    {
        mEnergy = 0;
        return 0;
    }
    
    //------------ calculate the initial energy:
    mEnergy = coulombEnergy(mCoord);
    
    // allocate forces and new coordinates:
    Allot<real> forces(3*nPoints, 0);
    Allot<real> coord(3*nPoints, 0);
    
    //make an initial guess for the step size:
    int history = 0;
    
    unsigned int step = 0;
    for ( step = 0; step < max_nb_iterations; ++step )
    {
        calculateForces(forces, threshold);
        
        while ( 1 ) 
        {
            movePoints(coord, mCoord, forces, mag);
            
            // energy of new configuration:
            real energy = coulombEnergy(coord);
            
            //printf("%3i : step %5i : mEnergy = %18.8f   mag = %8.5f %s\n",
            //     nbp, step, mEnergy, mag, (energy_new<mEnergy?"yes":"no"));

            if ( energy < mEnergy )
            {
                // swapp pointers to accept configuration:
                mCoord.swap(coord);                
                mEnergy = energy;
                
                /*
                 If we have done 'magic' successful moves at a given step size,
                 then we try to increase the step size.
                 Values for 'magic' below were tested in term of convergence:
                 a few trials seemed to agree for magic = 7...
                 */
                if ( ++history >= magic )
                {
                    mag *= 1.4147;   //this value is somewhat arbitrary
                    history = 0;
                }
                break;
            }
            else
            {
                /*
                 If the new configuration has higher energy,
                 we try a smaller step size with the same forces:
                 */
                history = 0;
                mag /= 2;
                
                //exit when the desired precision is reached
                if ( mag < precision )
                    return step;
            }
        }
    }
    return step;
}
//------------------------------------------------------------------------------
