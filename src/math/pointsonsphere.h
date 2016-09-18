// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef POINTSONSPHERE_H
#define POINTSONSPHERE_H

#include "smath.h"
#include "random.h"
#include "real.h"
#include "allot.h"


///\todo we could replace here the Coulomb repulsive interaction by a linear force
/* 
A linear forces will allow us to solve an associated linear system on 
the coordinates of the point, using an interative solver.
that might be quite fast. The repulsive interaction only need to take
the neighbour into account, so we could have local force only, which
would scale better than having everybody interact with everybody (Coulomb)
*/

/// Distribute points on the unit sphere, minimizing the 'electrostatic' energy
/**  The number of points is arbitrary, see
http://mathworld.wolfram.com/SphericalCode.html
\n

Algorithm:
 -# The points are distributed randomly on the sphere
 -# A 1/r^3 repulsive force is assumed for all points, 
 to which corresponds a certain potential energy in 1/r^2
 -# New positions are calculated form the current one, the forces
 and an adjustable scaling factor:  dx = scale * ( force at x )
 -# The potential energy of the new configuration is calculated, and:
     - If the total energy is lower, the move is accepted and the scaling factor is increased,
     - If the energy is higher, the move is rejected and the scaling factor is reduced.
     .
 .
 
The procedure (steps 2-4) is continues until convergence.\n

The main method is the class constructor, or equivalently distributePoints(),
which take the number of points as argument and makes the calculation.
Points coordinates can then be retrieved using either:
    - copyPositionsForAllPoints()
    - copyCoordinatesOfPoint()

\author
F. Nedelec, created August 2002, last modified October 10, 2002
*/
class PointsOnSphere
{
    ///This number affects convergence speed but not the result
    static const int  magic = 7;
    
    ///max. number of iterations
    static const unsigned int  max_nb_iterations = 50000;
    
    /// number of point on the sphere
    unsigned int   nPoints;
    
    /// coordinates of the points in a array
    /** in the array all the coordinates are together (x,y,z) point 1, (x,y,z) point 2, etc.
        so the coordinates for the first point are:
        x = mCoord[0], y = mCoord[1], z = mCoord[3]
        the coordinates of point ii are:
        x = mCoord[3*i+0], y = mCoord[3*i+1], z = mCoord[3*i+2] 
        */
    Allot<real> mCoord;
    
    /// Coulomb energy that must be minimize 
    real  mEnergy;
    
private:
    
    /// project point on the sphere
    void project(real P[3], const real S[3]);
    
    /// set coordinates point P randomly on the sphere
    void randomize(real P[3]);
    
    /// Calculate distance between point given their coordinates P and Q (3-dim)
    real distance3(const real P[], const real Q[]);
    
    /// Calculate distance between point given their coordinates P and Q (3-dim)
    real distance3Sqr(const real P[], const real Q[]);
    
    /// coulomb energy
    real coulombEnergy(const real P[]);
    
    /// coumomb force
    void calculateForces(real forces[], real threshold);
    
    /// move point from old to new coordinates
    void movePoints(real Pnew[], real Pold[], real forces[], real S);
    
    
public:
    
    /// expected distance between neighboring points
    static real   expectedDistance(int);
    
    /// default constructor, does nothing
    PointsOnSphere();
    
    /// constructor that also calls distributePoints(), 
    PointsOnSphere(int nbp);
    
    /// default destructor
    virtual ~PointsOnSphere();
    
    /// number of points in the configuration
    unsigned int    nbPoints()     { return nPoints;  }
    
    /// the 'virtual' total energy of the configuration
    real   finalEnergy()           { return mEnergy; }
    
    /// minimum distance in the configuration, in 3D space
    real   minimumDistance();
    
    /// multiply all coordinates by the given factor
    void   scale(real factor);
    
    /// address where the coordinates for point ii are  
    const real*  addr(const unsigned int ii) const { return mCoord + 3 * ii; }
    
    /// copy the coordinates from point ii onto the given 3-dim array x
    void   copyCoordinatesOfPoint(real x[3], unsigned int ii);
    
    /// copy the coordinates from point ii onto x,y,z
    void   copyCoordinatesOfPoint(real* x, real* y, real* z, unsigned int ii);
    
    /// copy the array points coordinates onto the given array x
    void   copyPositionsForAllPoints(real x[]);
    
    /// write points coordinates
    void   printAllPositions(FILE* file = stdout);
    
    /// distribute the nbp points on the sphere and store their coordinates
    int    distributePoints(unsigned int nbp, real precision = 1e-4);
    
};

#endif
