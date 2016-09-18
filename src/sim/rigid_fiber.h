// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef RIGID_FIBER_H
#define RIGID_FIBER_H

#include "fiber_naked.h"


class Matrix;

/// incompressible FiberNaked with bending elasticity
/**
 Implements the methods of a Mecable for the FiberNaked:
 
 -# setSpeedsFromForces() includes longitudinal incompressibility,
 which means keeping successive points equidistants:
 norm( point(p+1) - point(p) ) = segmentation()
 
 -# addRigidity() implements bending elasticity.
 .
*/
class RigidFiber : public FiberNaked
{
private:
    
    /// allocation size for projection
    unsigned    rfAllocated;
    
    /// Lagrange multipliers associated with longitudinal imcompressibility
    real   *    rfLag;
    
    /// stored difference of model-points (array of size DIM*nbSegments)
    real   *    rfDiff;
    
    /// memory allocated to hold nb_points values
    real   *    rfTMP;
        
    /* variables used for projecting without an explicit matrix ( rigid_fiber_project.cc ) */
    
    /// J*J', a nbSegments^2 matrix. We store the diagonal and one off-diagonal
    real   *    mtJJt, * mtJJt2;
    
    /// vector for the projection correction of size nbSegments
    real   *    mtJJtiJforce;
        
protected:
    
    /// mobility of the points
    /** The entire Fiber's mobility is rfMobility/nbPoints() */
    real        rfMobility;
    
    /// rigidity scaling factor used in addRigidity()
    real        rfRigidity;
    
    /// calculate the average difference of successive model-point in rfDiff[]
    void        storeDifferences();

private:
    
    /// reset the memory pointers for the projection
    void        buildProjection();
    
    /// allocate memory for the projection
    void        allocateProjection(unsigned int nb_points);
    
    /// free the memory for the projection
    void        destroyProjection();

public:
    
    /// Constructor
    RigidFiber();
    
    /// copy constructor
    RigidFiber(RigidFiber const&);
    
    /// copy assignment
    RigidFiber& operator=(RigidFiber const&);
    
    /// Destructor
    virtual    ~RigidFiber();
    
    ///sets the number of points in the Fiber
    virtual unsigned allocatePoints(unsigned nbp);
    
    /// compute Lagrange multiplier corresponding to the longitudinal tensions in the segments
    void        computeTensions(const real* force);
    
    /// longitudinal force along segment `p` (positive=extension, negative=compression)
    real        tension(unsigned p) const { assert_true(p+1<nbPoints()); return rfLag[p]; }
    
    /// total drag-coefficient of object (force = drag * speed)
    real        dragCoefficient() const { return  nbPoints() / rfMobility; }
    
    /// mobility of one point
    real        pointMobility() const { return rfMobility; }
    
    //--------------------- Projection  / Dynamics
    
    /// prepare for projection
    void        makeProjection();
    
    /// apply the projection: Y <- s * P * X
    /** work should be of size nbSegments() at least */
    void        projectForces(const real* X, real* Y, real s, real* work) const;
    
    /// prepare the correction to the projection
    void        makeProjectionDiff(const real* );
    
    /// add the contribution from the projection correction
    void        addProjectionDiff(const real*, real*) const;
    
    
    
    /// add displacements due to the Brownian motion to rhs[]
    real        addBrownianForces(real* rhs, real sc) const;
    
    /// calculate the speeds from the forces, including projection
    void        setSpeedsFromForces(const real* X, real* Y, real, bool) const;
    
    //--------------------- Rigidity
        
    /// add the rigidity force corresponding to configuration X into vector Y  
    void        addRigidity(const real* X, real* Y) const;
    
    /// add rigidity terms into the matrix
    void        addRigidityMatUp(Matrix &, int offset ) const;
    
};


#endif
