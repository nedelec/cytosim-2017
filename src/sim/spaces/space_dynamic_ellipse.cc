// Cytosim 3.0 - F. Nedelec and Laboratory, Copyright EMBL 2007

#include "dim.h"
#include "space_dynamic_ellipse.h"
#include "exceptions.h"
#include "smath.h"
#include "iowrapper.h"

/// prefactor for volume computation
#if ( DIM == 3 )
const real pref = M_PI*4.0/3.0;
#else
const real pref = M_PI;
#endif

/// power for ellipsoid surface calculation
const real POW = 1.6075;

/// building block for area of an ellipsoid in 3D
inline real surf_block(const real a, const real b)
{
    return pow(a*b,POW);
}

/// building block for area of an ellipsoid in 3D
inline real surf_block(const real a, const real b, const real c)
{
    return pow(a*b,POW) + pow(b*c,POW) + pow(a*c,POW);
}


SpaceDynamicEllipse::SpaceDynamicEllipse(const SpaceProp* p)
: SpaceEllipse(p)
{
    if ( DIM == 1 )
        throw InvalidParameter("dynamic_ellipse is not usable in 1D");
    
    pressure = 0;
    mat = MatrixD::one();
    inv = MatrixD::one();
    
    reset_forces();
    inter_forces.set(0,0,0);
}


void SpaceDynamicEllipse::report(std::ostream& out) const
{
    Vector3 L(mLength);
    out<<"% DynamicEllipse " << std::setw(10) << L << '\n';
    out<<"%   energy " << prop->tension * surfaceEllipse(mLength);
    out<<"  volume " << volumeEllipse(mLength) << "  pressure " << pressure << '\n';
    out<<"%   forces " << std::setw(10) << inter_forces << '\n';
    out<< std::endl;
}

//-------------------------------------------------------------------------------------
//        Set interactions and update forces felt by ellipse.
//-------------------------------------------------------------------------------------

void SpaceDynamicEllipse::setInteractions(Meca &) const
{
    reset_forces();
}


/**
 Call the appropriate interaction from \a meca, to confine \a pe, which is at position \a pos.
 
 The default implementation projects \a pos,
 to calculate the direction of the normal to the edge of the Space,
 and then calls Meca::interPlane, with the approprimate aguments.
 This generates a friction-less potential centered on the edge.

 Also update \a Rforces and \a Torques that will be use to evolve the Space
*/
void SpaceDynamicEllipse::setInteraction(Vector const &pos, PointExact const& pe, Meca & meca, real stiff) const
{
    Vector proj;
    project( pos, proj );
    Vector dir = pos - proj;
    real n = dir.normSqr();
    if ( n > 0 )
    {
        // Register the force to the ellipse
        decompose_force(stiff * dir, proj, dir);
        // And to the meca
        meca.interPlane(pe, dir, proj, stiff/n);
    }
}



//------------------------------------------------------------------------------
//        Computing forces
//------------------------------------------------------------------------------

void SpaceDynamicEllipse::reset_forces() const
{
    Torques = nullTorque;
    Rforces.set(0,0,0);
}

/**
 register forces applied to the space
 */
void SpaceDynamicEllipse::decompose_force(const Vector& forces, const Vector& pos,const Vector& dir) const
{
    add_radial_force(forces, pos);
    Torques += vecProd(pos, forces);
}

/**
 Add a point-like force acting on the ellipse
 */
void SpaceDynamicEllipse::add_radial_force(const Vector& forces, const Vector& pos)const
{
    Vector U = director(0);
    Rforces.XX += (U*forces) * (U*pos) / length(0);
#if ( DIM >= 2 )
    Vector V = director(1);
    Rforces.YY += (V*forces) * (V*pos) / length(1);
#endif
#if ( DIM == 3 )
    Vector W = director(2);
    Rforces.ZZ += (W*forces) * (W*pos) / length(2);
#endif
}


// ----------------------------------------------
//  Internal forces
// ----------------------------------------------


/*
 Pressure is a Lagrange multiplier associated with volume conservation
 We follow Newtons's method to minimize
 F = Volume(next_time_step) - prop->volume
 Hence we iterate:
 P = P - F / dF
 until the machine precision is exhausted
*/
real SpaceDynamicEllipse::compute_pressure(const Vector & sizes, const Vector & radif)
{
    real P = pressure;
    real err = INFINITY, last_err;
    
    if ( prop->mobility_dt <= 0 )
        return 0;
    
    do {
        last_err = err;
        
        // the objective is to reach desired volume at the next time-step:
        Vector dim = sizes + ( radif + pressure_forces(P) ) * prop->mobility_dt;
       
        real der = pref * pref * prop->mobility_dt;
        
        real r0 = dim.XX;
#if ( DIM == 2 )
        real r1 = dim.YY;
        err = pref * r0 * r1 - prop->volume;
        der *= r0*r0 + r1*r1;
#elif ( DIM == 3 )
        real r1 = dim.YY;
        real r2 = dim.ZZ;
        err = pref * r0 * r1 * r2 - prop->volume;
        der *= r0*r0*r1*r1 + r0*r0*r2*r2 + r1*r1*r2*r2;
#endif

        P -= err / der;

    } while ( fabs(err) < fabs(last_err) );
    //std::clog << "volume error " <<  err << '\n';

    return P;
}


/**
 The derivative of surface energy with respect to each ellipse parameter:
 ES = Surface * tension
 dES/da = dS/da * tension
 */
Vector SpaceDynamicEllipse::tension_forces(real T)
{
    Vector ft;

#if ( DIM == 2 )

    real S = -M_PI * T;
    real N = sqrt( (3.0*length(0)+length(1))*(length(0)+3.0*length(1)) );

    ft.XX = S * (3.0 - ( 3.0*length(0) + 5.0*length(1) ) / N );
    ft.YY = S * (3.0 - ( 3.0*length(1) + 5.0*length(0) ) / N );
    
#elif ( DIM == 3 )
    
    real S = -T * surfaceEllipse(mLength);
    real pXY = surf_block(length(0), length(1));
    real pXZ = surf_block(length(0), length(2));
    real pYZ = surf_block(length(1), length(2));
    real XYZ = surf_block(length(0), length(1), length(2));

    ft.XX = S * ( pXY + pXZ ) / ( length(0) * XYZ );
    ft.YY = S * ( pXY + pYZ ) / ( length(1) * XYZ );
    ft.ZZ = S * ( pXZ + pYZ ) / ( length(2) * XYZ );

#endif
    return ft;
}

/**
 The derivative of pressure energy with respect to each ellipse parameter:
 EP = Volume * Pressure
 dEP/da = dV/da * Pressure
*/
Vector SpaceDynamicEllipse::pressure_forces(const real P)
{
    Vector fp;
#if ( DIM == 2 )
    fp.XX = pref*length(1)*P;
    fp.YY = pref*length(0)*P;
#elif ( DIM == 3 )
    fp.XX = pref*length(1)*length(2)*P;
    fp.YY = pref*length(2)*length(0)*P;
    fp.ZZ = pref*length(0)*length(1)*P;
#endif
    return fp;
}

//-------------------------------------------------------------------------------------
///    Update ellipse shape
//-------------------------------------------------------------------------------------
    

void SpaceDynamicEllipse::step()
{
    if ( prop->volume > 0 )
    {
        inter_forces = Rforces;
        
        // calculate forces:
        Rforces += tension_forces(prop->tension);
        pressure = compute_pressure(mLength, Rforces);
        Rforces += pressure_forces(pressure);

        // implement changes in shape:
        if ( prop->mobility_dt > 0 )
        {
            Vector delta = prop->mobility_dt * Rforces;
            for (int i=0; i<DIM ;++i)
            {
                assert_true(delta[i] == delta[i]);
                Space::resize(i, length(i) + delta[i]);
            }
            report(std::clog);
            //std::clog << "%  balance " << Rforces << "\n";
        }
        
        // implement rotation:
        if ( prop->mobility_rot_dt > 0 )
        {
            //std::clog << "% DynamicEllipse torque " << Torques << "\n";
#if ( DIM == 2 )
            real theta = prop->mobility_rot_dt * Torques;
            if ( theta > REAL_EPSILON )
            {
                Matrix2 rot = Matrix2::rotationFromEulerAngles(theta);
                mat = rot * mat;
            }
#elif ( DIM == 3 )
            real theta = prop->mobility_rot_dt * Torques.norm();
            if ( theta > REAL_EPSILON )
            {
                Matrix3 rot = Matrix3::rotationAroundAxis(Torques, theta);
                mat = rot * mat;
            }
#endif
            // update rotation:
            inv = mat.transposed();
        }
    }
}




/// Checking consistency of ellipse sizes
void SpaceDynamicEllipse::resize()
{
    if ( prop->volume <= 0 )
    {
        const_cast<SpaceProp*>(prop)->volume = volume();
        //std::cout << " dynamic_ellipse:volume set to " << prop->volume << std::endl;
    }
}


//-------------------------------------------------------------
// Utilities
//-------------------------------------------------------------


Vector SpaceDynamicEllipse::director(unsigned ix) const
{
    assert_true(ix < DIM);
    return mat.getColumn(ix);
}


real SpaceDynamicEllipse::surfaceEllipse(Vector const& sizes)
{
#if ( DIM == 3 )
    real a = sizes[0];
    real b = sizes[1];
    real c = sizes[2];
    return    4.0*M_PI*pow(surf_block(a,b,c)/3.0, 1.0/POW);
#elif ( DIM == 2 )
    // In 2D, the 'surface' is a line
    real a = sizes[0];
    real b = sizes[1];
    real r = square( (a-b)/(a+b) );
    return M_PI*(a+b)*(1.0+3.0*r/(10.0+sqrt(4.0-3.0*r)));
#else
    return 0;
#endif
}


real SpaceDynamicEllipse::volumeEllipse(Vector const& sizes)
{
#if ( DIM == 3 )
    return pref*sizes[0]*sizes[1]*sizes[2];
#elif ( DIM == 2 )
    return pref*sizes[0]*sizes[1];
#endif
	return 0;
}


void  SpaceDynamicEllipse::read(InputWrapper& in, Simul& sim)
{
    Space::read(in, sim);
    unsigned n = in.readUInt16();
    if ( n != 10 )
        throw InvalidIO("Unexpected data in SpaceDynamicEllipse::read");
    real vol = in.readFloat();
    // adjust volume in Property:
    const_cast<SpaceProp*>(prop)->volume = vol;
    // read 3x3 orientation matrix:
#if ( DIM == 3 )
    for ( unsigned i = 0; i < 9; ++i )
        mat[i] = in.readFloat();
#else
    real m[9] = { 0 };
    for ( unsigned i = 0; i < 9; ++i )
        m[i] = in.readFloat();
#if ( DIM == 2 )
    mat[0] = m[0];
    mat[1] = m[1];
    mat[2] = m[3];
    mat[3] = m[4];
#endif
#endif
}


void  SpaceDynamicEllipse::write(OutputWrapper& out) const
{
    Space::write(out);
    out.writeUInt16(10);
    out.writeFloat(prop->volume);
#if ( DIM == 3 )
    for ( unsigned i = 0; i < 9; ++i )
        out.writeFloat(mat[i]);
#else
    real m[9] = { 0 };
#if ( DIM == 2 )
    m[0] = mat[0];
    m[1] = mat[1];
    m[3] = mat[2];
    m[4] = mat[3];
#endif
    for ( unsigned i = 0; i < 9; ++i )
        out.writeFloat(m[i]);
#endif
}



//------------------------------------------------------------------------------
//                         OPENGL  DISPLAY
//------------------------------------------------------------------------------

#ifdef DISPLAY

#include "gle.h"

bool SpaceDynamicEllipse::display() const
{
    if ( 0 )
    {
        // display principal axes:
        glBegin(GL_LINES);
        for ( unsigned n=0; n < DIM; ++n )
        {
            glVertex2f(0,0);
            gle::gleVertex(length(n)*director(n));
        }
        glEnd();
    }

    GLfloat MM[16] = { 0 };
    MM[ 5]=1.0f;
    MM[10]=1.0f;
    MM[15]=1.0f;
    for ( int i=0; i<DIM; ++i )
    for ( int j=0; j<DIM; ++j )
        MM[i+4*j] = mat(i,j);
    glPushMatrix();
    glMultMatrixf(MM);
    SpaceEllipse::display();
    glPopMatrix();

    return true;
}

#else

bool SpaceDynamicEllipse::display() const
{
    return false;
}


#endif

