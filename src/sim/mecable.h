// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#ifndef MECABLE_H
#define MECABLE_H

#include "dim.h"
#include "object.h"
#include "sim.h"
#include "matrix.h"
#include "buddy.h"

class Meca;

/// Can be simulated using a Meca.
/**
 A Mecable is an Object made of points that can can be simulated in Meca.
 
 Mecable defines an interface that is implemented in Bead and PointSet.
 Mecable is also a Buddy, and can thus be part of an Organizer.
 
 */
class Mecable : public Object, public Buddy
{
    
private:
        
    /// index in the matrices and vectors using in Meca
    Matrix::index_type mIndex;
    
    /// block matrix used to precondition
    real *        pBlock;
    
    /// allocated size of pBlock
    unsigned int  pBlockSize;
    
    /// flag for preconditionning
    bool          pBlockUse;
    
    ///\todo add Mecable copy constructor and copy assignment
    
    /// Disabled copy constructor
    Mecable(Mecable const&);
    
    /// Disabled copy assignment
    Mecable& operator=(Mecable const&);
    
public:
        
    /// The constructor resets the pointers
    Mecable();
    
    /// Destructor de-allocates memory
    virtual ~Mecable();
    
    //--------------------------------------------------------------------------
    
    /// Number of points
    virtual unsigned nbPoints() const = 0;
    
    /// return position of point P
    virtual Vector posPoint(unsigned p) const = 0;
    
    /// copy current coordinates to provided array
    virtual void   putPoints(real[]) const = 0;

    /// replace current coordinates of points by values from the provided array
    virtual void   getPoints(const real[]) = 0;
    
    //--------------------------------------------------------------------------

    /// return force-vector on point `p` calculated at previous step by Meca
    virtual Vector netForce(unsigned p) const = 0;
    
    /// replace current forces by the ones provided as argument
    virtual void   getForces(const real[]) = 0;
    
    /// compute Lagrange multiplier corresponding to mechanical constraints
    virtual void   computeTensions(const real* force) {}

    //--------------------------------------------------------------------------
    
    /// Store the index where coordinates are located in Meca
    void           matIndex(Matrix::index_type index)  { mIndex = index; }
    
    /// Index in mB of the first point. the index in the vectors is DIM*matIndex()
    /** X1 is stored at DIM*matIndex(), Y1 at DIM*matIndex()+1, Z1 at DIM*matIndex()+2
     then X2, Y2, Z2...
     */
    Matrix::index_type matIndex() const { return mIndex; }
    
    
    /// Tell Meca to use preconditionning on this object or not
    bool          useBlock()      const { return pBlockUse; }
    
    /// change preconditionning flag
    void          useBlock(bool b)      { pBlockUse = b; }
    
    /// Allocate a block of the requested size
    real *        allocateBlock(unsigned);

    /// return allocated block
    real *        block()          const { return pBlock; }
    
    //--------------------------------------------------------------------------
    /// Calculate the mobility coefficient
    virtual void  setDragCoefficient() = 0;
    
    /// The total drag coefficient of object ( force = drag * speed )
    virtual real  dragCoefficient() const = 0;

    
    /// prepare the Mecable to solve the mechanics in Meca::solve()
    /**
     This is called in Meca, before the system is solved, 
     and it should prepare all the variables necessary for:
     - addRigidity() -> set rigidity coefficients
     - setSpeedsFromForces() -> call setDragCoefficient(),
     - set matrix/variables necessary for constrained dynamics
     .
     */
    virtual void  prepareMecable() = 0;
        
    /// Add Brownian noise terms to a force vector (sc = kT / dt)
    virtual real  addBrownianForces(real* rhs, real sc) const { return INFINITY; }
    
    //--------------------------------------------------------------------------
    
    /// Add rigidity terms Y <- Y + Rigidity * X
    /**
        Rigidity can be any force acting internally to the objects
     for example, the bending rigidity of Fibers.
     This version is used to calculate the Matrix * Vector in Meca.
     */
    virtual void  addRigidity(const real* X, real* Y) const {}
    
    /// Add rigidity matrix elements (which should be symmetric) to provided matrix
    /**
       The function should add terms to the upper part of matrix mB, at indices starting from offset.
     It should fill at maximum the upper part of the diagonal block corresponding to indices [offset - offset+DIM*nbPoints()].
     This version is used to build the preconditionner in Meca.
     It should be consistent with addRigidity(): add the same terms.
     */
    virtual void  addRigidityMatUp(Matrix & mB, int offset) const {}
    
    /// Calculate speeds from given forces
    /**
        The function should perform Y <- sc * mobility * X.
        - X and Y are vectors of size DIM*nbPoints().
        - sc is a provided scalar, and mobility is known by the object. 
        - if rhs==true, the call was made with X containing the true force in the system.
     */
    virtual void  setSpeedsFromForces(const real* X, real* Y, real sc=1.0, bool rhs=false) const = 0;
    
    //--------------------------------------------------------------------------

    /// set the terms obtained from the linearization of the Projection operator, from the given forces
    /** This is enabled by a keyword PROJECTION_DIFF in meca.cc */
    virtual void  makeProjectionDiff(const real* forces) {}
    
    /// add terms from projection correction terms: Y <- Y + P' * X;
    /** This is enabled by a keyword PROJECTION_DIFF in meca.cc */
    virtual void  addProjectionDiff(const real* X, real* Y) const {}
    
    //--------------------------------------------------------------------------
    /// add the interactions (for example due to confinements)
    virtual void  setInteractions(Meca &) const {}
    
};

#endif
