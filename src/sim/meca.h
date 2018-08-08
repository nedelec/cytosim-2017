// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef MECA_H
#define MECA_H

#include "array.h"
#include "bicgstab.h"
#include "vector.h"
#include "matsym.h"
#include "matsparse.h"
#include "matsparsesym.h"
#include "matsparsesym1.h"

class Mecable;
class PointExact;
class PointInterpolated;
class SimulProp;
class Modulo;


/// A class to calculate the motion of objects in Cytosim
/**
Meca solves the motion of point-objects defined by the dynamic equation:
 
 @code
 d vPTS/dt = mobility * mP * ( vBAS + ( mB + mC + mR + mdiffP ) * vPTS )
 @endcode
 
 This equation includes internal forces (bending elasticity for RigidFiber), 
 and all the interactions between the Mecalbe in the system. 
 It is solved in the presence of Brownian motion, and at low Reynolds number
 ( mass/inertia effects are negligible ).
 
 - Vector vPTS contains all the Mecable coordinates (x, y, z): 
   Fiber, Sphere, Solid and other PointSet. size(vPTS) = DIM * nbPts.
 
 - Vector vBAS is of same size as vPTS, and includes the constant part obtained by
   linearization of the forces. It includes for instance the positions of Single,
   calibrated random forces simulating Brownian motion, and also offsets for periodic
   boundary conditions.
 
 - Matrix mB is the isotropic part obtained after linearization of the forces.
   It operates similarly and independently on the different dimension X, Y and Z.
   mB is square of size nbPts, symmetric and sparse.
 
 - Matrix mC is the non-isotropic part obtained after linearization of the forces.
   mC is square of size DIM*nbPts, symmetric and sparse.
 .
 
 Typically, mB and mC will inherit the stiffness coefficients of the interactions, 
 while vBAS will get forces (stiffness * position). They are set by using
 interLink(), interLongLink(), interSideLink(), interSlidingLink(), etc.

 - mR add the bending elasticity for RigidFiber, or other internal forces.
   mR is symmetric of size DIM*nbPts, diagonal by blocks, each block corresponding to a Fiber. 
 
 - mP applies the projection due to constrained dynamics.
   For RigidFiber, this maintains the distance between neighboring points (longitudinal incompressibility). 
   mP is symmetric of size DIM*nbPts, diagonal by blocks, each block corresponding to a Fiber. 
   mP is not actually calculated as a matrix:
   its application on each block is done by Mecable::setSpeedsFromForces()
 
 - mdiffP is a term coming from the derivative of the projection P,
   and it is added here because it provides better numerical stability in some situations.
   You can undefine PROJECTION_DIFF in meca.cc to remove mdiffP.
 .
 
 
 Note: All Links are disabled if the given PointExacts or PointInterpolated have a point 
 in common, because the matrix elements are not calcuated correctly in that case. 
 Generally, such interactions are anyway not desirable. It would correspond for 
 example to a link inside a segment, or between two successive segments on the same Fiber. 

 */

class Meca
{
private:
    
    /// same as Matrix::index_type
    typedef Matrix::index_type index_type;
    
    /// local copy of the SimulProp::time_step
    real            time_step;
    
    /// list of Mecable containing points to simulate
    Array<Mecable*> objs;
    
    /// total number of points in the system
    unsigned int    nbPts;
    
    /// size of the currently allocated memory
    unsigned int    allocated;
    
    /// max block size
    unsigned int    largestBlock;

    //--------------------------------------------------------------------------
    // Vectors of size DIM * nbPts
    
    real*  vPTS;         ///< position of the points
    real*  vSOL;         ///< the points at the previous steps
    real*  vBAS;         ///< base points of forces
    real*  vRHS;         ///< right hand side of the final system
    real*  vFOR;         ///< the calculated forces, with Brownian components
    real*  vTMP;         ///< intermediate of calculus

    //--------------------------------------------------------------------------
    
    /// true if the matrix mB is non-zero and used
    bool   use_mB;
    
    /// true if the matrix mC is non-zero and used
    bool   use_mC;
    
public:
    /// isotropic symmetric part of the dynamic, size (nbPts)^2
    /** 
        For interactions which have identical coefficients on the X, Y, Z subspaces
    */
    MatrixSparseSymmetric1  mB;

    
    /// non-isotropic symmetric part of the dynamic, size (DIM*nbPts)^2
    /** 
        For interactions which have different coefficients on the X, Y, Z subspaces,
        or which create interactions between two different subspaces.
    */
    MatrixSparseSymmetric1  mC;

    /// base for force
    real&   base(index_type ix) { return vBAS[ix]; }
    
    /// position of a point
    Vector  pos(index_type ix) const { return Vector::make(vPTS+DIM*ix); }
    
private:

    /// Y is set as the DIM-duplicate of Y, and symmetrized
    void  duplicateMat(int ps, const real* X, real* Y) const;
    
    /// add the linear part of forces:  Y <- Y + ( mB + mC ) * X;
    void  addLinearForces(const real* X, real* Y, bool with_rigidity) const;
    
    /// calculate the forces for all points: Y <- vBAS + ( mB + mC ) * X
    void  computeForces(const real* X, real* Y, bool with_rigidity) const;
    
    /// extract the matrix diagonal block corresponding to a Mecable
    void  getBlock(const Mecable *, real*) const;
    
    /// extract the matrix diagonal block corresponding to a Mecable
    void  getBlockS(const Mecable *, real*) const;

    /// allocate memory, compute preconditionner and return true if completed
    int   computePreconditionner();
    
    /// compute preconditionner using the provided temporary memory
    int   computePreconditionner(Mecable*, int*, real*, int);
    
public:
    

    /// constructor
    Meca();
    
    /// Clear list of Mecable
    void  clear();
    
    /// Add a Mecable to the list of objects to be simulated
    void  add(Mecable* ob);
    
    /// Number of Mecable
    unsigned nbMecables() const { return objs.size(); }
    
    /// number of points in the system
    unsigned nbPoints() const { return nbPts; }
    
    /// Implementation of Solver::LinearOperator
    unsigned size() const { return DIM * nbPts; }
    
    /// calculate Y = M*X, where M is the matrix associated with the system
    void multiply(const real* X, real* Y) const;

    /// apply preconditionner: Y = P*X
    void precondition(const real* X, real* Y) const;
    

    /// Add a constant force at a PointExact
    void  addPureForce(PointExact const&, Vector const& force);
    /// Add a constant force at a PointInterpolated
    void  addPureForce(PointInterpolated const&, Vector const& force);
    /// Add a torque to a segment
    void  addPureTorque(PointInterpolated const&, Torque const& torque);
    /// Add a torque to constrain the segment in given direction
    void  addTorqueClamp(PointInterpolated const&, Vector const& dir, real weight);
    
    // Hookean interactions between PointExact or PointInterpolated:
    
    /// Force of stiffness weight
    void  interLink(PointExact const&, PointExact const&, real weight);
    /// Force of stiffness weight
    void  interLink(PointInterpolated const&, PointExact const&, real weight);
    /// Force of stiffness weight
    void  interLink(PointInterpolated const&, PointInterpolated const&, real weight);

    /// Force of stiffness weight and resting length len
    void  interLongLink(PointExact const&, PointExact const&, real len, real weight);
    /// Force of stiffness weight and resting length len
    void  interLongLink(PointInterpolated const&, PointExact const&, real len, real weight);
    /// Force of stiffness weight and resting length len
    void  interLongLink(PointInterpolated const&, PointInterpolated const&, real len, real weight);

#if ( DIM == 2 )
    /// Force of stiffness weight and resting length len, on the side of first fiber
    void  interSideLink2D(PointInterpolated const&, PointExact const&, real arm, real weight);
#elif ( DIM == 3 )
    /// Force of stiffness weight and resting length len, on the side of first fiber
    void  interSideLinkS(PointInterpolated const&, PointExact const&, Vector const& arm, real len, real weight);
#endif
    /// Force of stiffness weight and resting length len, on the side of first fiber
    void  interSideLink(PointInterpolated const&, PointExact const&, real len, real weight);

    
#if ( DIM == 2 )
    /// Force of stiffness weight and resting length len, on the side of first fiber
    void  interSideLink2D(PointInterpolated const&, PointInterpolated const&, real arm, real weight);
#elif ( DIM == 3 )
    /// Force of stiffness weight and resting length len, on the side of first fiber
    void  interSideLinkS(PointInterpolated const&, PointInterpolated const&, Vector const& arm, real len, real weight);
#endif
    /// Force of stiffness weight and resting length len, on the side of first fiber
    void  interSideLink(PointInterpolated const&, PointInterpolated const&, real len, real weight);

#if ( DIM == 2 )
    /// Force of stiffness weight and resting length len, on the sides of both fibers
    void  interSideSideLink2D(PointInterpolated const&, PointInterpolated const&, real len, real weight, int side1, int side2);
#endif
    /// Force of stiffness weight and resting length len, on the sides of both fibers
    void  interSideSideLink(PointInterpolated const&, PointInterpolated const&, real len, real weight);

    /// Force of stiffness weight and resting length len, which can slide on first fiber
    void  interSlidingLink(PointInterpolated const&, PointExact const&, real weight);
    /// Force of stiffness weight and resting length len, which can slide on first fiber
    void  interSlidingLink(PointInterpolated const&, PointInterpolated const&, real weight);

    
#if ( DIM == 2 )
    /// Force of stiffness weight and resting length len, on the side of first fiber
    void  interSideSlidingLink2D(PointInterpolated const&, PointExact const&, real arm, real weight);
#elif ( DIM == 3 )
    /// Force of stiffness weight and resting length len, on the side of first fiber
    void  interSideSlidingLinkS(PointInterpolated const&, PointExact const&, Vector const& arm, real len, real weight);
#endif
    /// Force of stiffness weight and resting length len, on the side of first point, which can slide
    void  interSideSlidingLink(PointInterpolated const&, PointExact const&, real len, real weight);
    
    
#if ( DIM == 2 )
    /// Force of stiffness weight and resting length len, on the side of first fiber
    void  interSideSlidingLink2D(PointInterpolated const&, PointInterpolated const&, real arm, real weight);
#elif ( DIM == 3 )
    /// Force of stiffness weight and resting length len, on the side of first fiber
    void  interSideSlidingLinkS(PointInterpolated const&, PointInterpolated const&, Vector const& arm, real len, real weight);
#endif
    
    /// Force of stiffness weight and resting length len, on the side of first point, which can slide
    void  interSideSlidingLink(PointInterpolated const&, PointInterpolated const&, real len, real weight);
    
#if (DIM == 2)
    void interTorque2D(PointInterpolated const&, PointInterpolated const&, real cosinus, real sinus, real torque_weight);
#endif
    
    /// Force of stiffness weight with fixed position g
    void  interClamp(PointExact const&, const real g[], real weight);
    /// Force of stiffness weight with fixed position g
    void  interClamp(PointInterpolated const&, const real g[], real weight);
    /// Force of stiffness weight with fixed position g, on the side
    void  interLongClamp(PointExact const&, Vector const& center, real len, real weight);
    /// Force of stiffness weight with fixed position g, which can slide
    void  interSlidingClamp(PointInterpolated const&, Vector const& g, real weight);

#if ( DIM == 2 )
    /// Force of stiffness weight and resting length len, on the side of first fiber
    void  interSideClamp2D(PointInterpolated const&, const real g[], real arm, real weight);
#elif ( DIM == 3 )
    /// Force of stiffness weight and resting length len, on the side of first fiber
    void  interSideClamp3D(PointInterpolated const&, const real g[], Vector const& arm, real weight);
#endif
    /// Force of stiffness weight with fixed position g, on the side
    void  interSideClamp(PointInterpolated const&, const real g[], real len, real weight);

    /// Force of stiffness weight with a plane defined by g and its normal dir 
    void  interPlane(PointExact const&, Vector const& dir, Vector const& g, real weight);
    /// Force of stiffness weight with a plane defined by g and its normal dir 
    void  interPlane(PointInterpolated const&, Vector const& dir, Vector const& g, real weight);

    /// Linearized Coulomb repulsive force (experimental)
    void  interCoulomb(PointExact const&, PointExact const&, real weight);
    
    
    /// Allocate the memory necessary to solve(). This must be called after the last add()
    void  prepare(SimulProp const*);
    
    /// Calculate motion of the system
    void  solve(SimulProp const*, bool precondition);
    
    /// calculate Forces on objects and Lagrange multipliers for Fiber, without thermal motion
    void  computeForces();
    
    
    
    /// Print the complete matrix (for debugging)
    void  printMatrix(std::ostream &) const;
    
    /// Save complete matrix in binary file
    void  dumpMatrix(FILE *) const;

    /// Save complete matrix in binary file
    void  dumpProjection(FILE *, Mecable const*, bool diff) const;

    /// Save complete matrix in binary file
    void  dumpDiagonal(FILE *) const;

    /// Output vectors and matrices in various files (for debugging)
    void  dump() const;
    
};

#endif

