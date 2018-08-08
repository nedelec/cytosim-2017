// Cytosim 3.0 - F. Nedelec and Laboratory, Copyright EMBL 2007

#ifndef SPACE_DYNAMIC_ELLIPSE_H
#define SPACE_DYNAMIC_ELLIPSE_H

#include "dim.h"
#include "space.h"
#include "space_ellipse.h"
#include "meca.h"

/// ellipse in 2D, ellipsoid or spheroid in 3D 
/**
 Space `ellipse` is aligned with the principal axes X, Y and Z.
 In 2D, there are two principal axis, and this is called an ellipse.
 In 3D, it is called an ellipsoid.
 
 @code
    ellipse sizeX sizeY sizeZ
 @endcode 

 With:
 - sizeX = half length of X axis
 - sizeY = half length of Y axis
 - sizeZ = half length of Z axis
 .

 This Space has no corners: setInteraction() relies on project()
 */

class SpaceDynamicEllipse : public SpaceEllipse
{    
public:
 	
    /// constructor
    SpaceDynamicEllipse(const SpaceProp*);
    
    /// add interactions to a Meca
    void	setInteractions(Meca &) const;

	/// setInteraction and changes the forces the ellipse undergoes
	void    setInteraction(Vector const &pos, PointExact const& pe, Meca & meca, real stiff) const;

	///	ContractEllipse has a step function to adjust shape
	void    step();
	
    /// project point on the closest edge of the Space
    void	project(const real point[], real proj[]) const
	{
		real p[DIM], w[DIM];
		inv.vecMul(point,p);
		SpaceEllipse::project(p, w);
		mat.vecMul(w,proj);
	}
	
	/// true if the point is inside the Space 
	bool    inside(const real point[]) const
	{
		real p[DIM];
		inv.vecMul(point,p);
		return SpaceEllipse::inside(p);
	}

    ///
	void	resize();
	
	/// read from file
    void  read(InputWrapper& , Simul&);
    
    /// write to file
    void  write(OutputWrapper&) const;
	
	/// return forces
	void  report(std::ostream&) const;	

	
private:
	
	/// Orientation matrix
	MatrixD  mat;
	
	/// Inverse of mat
	MatrixD  inv;
	
	/// pressure : lagrange parameter for volume conservation
	real     pressure;
	
	/// Forces from interactions
	mutable Vector inter_forces;
	
	/// Radial and circular forces
	mutable Vector Rforces;
	mutable Torque Torques;
    
    /// Reset forces
    void		reset_forces() const;
	
	/// Decompose forces between radial and circular components
	void		decompose_force(Vector const &, Vector const &, Vector const &) const;
    
    /// Add radial component
	void		add_radial_force(Vector const &, Vector const &) const;
	
	/// Find the optimal value of the pressure
	real		compute_pressure(Vector const &, Vector const&);
	
	/// Return forces corresponding to given pressure
	Vector		pressure_forces(real pressure);
	
	/// return forces caused by given surface tension
	Vector		tension_forces(real tension);
	
	/// Gives the i-th eigenvector of the ellipsoid
	Vector		director(unsigned i) const;
	
	
	/// Surface area of an ellipse of given axis length
	static real	surfaceEllipse(const Vector &);
    
    /// Volume of an ellipse of given axis length
	static real	volumeEllipse(const Vector &);

	/// Display
	bool		display() const;
};

#endif

