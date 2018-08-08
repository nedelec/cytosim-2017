// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
//------------------------------------------------------------------------------
//                  -- here is the heart of cytosim --
//------------------------------------------------------------------------------
//             solving the equations of motion for the PointSet
//                  implicit integration, sparse matrix
//------------------------------------------------------------------------------

/**
 Add correction term to the constrainted dynamics
 The effect is to stabilize fibers under traction, at some modest CPU cost.
*/
#define PROJECTION_DIFF


/// this define will enable explicit integration (should be off)
//#define EXPLICIT


/// use this to generate code for validation
#define DEBUG_MECA 0


#include "meca.h"
#include "mecable.h"
#include "messages.h"
#include "cblas.h"
#include "clapack.h"
#include "exceptions.h"
#include <fstream>
#include "allot.h"
#include "vecprint.h"

#include "meca_inter.cc"

//------------------------------------------------------------------------------

Meca::Meca()
{
    nbPts = 0;
    largestBlock = 0;
    allocated = 0;
    vPTS = 0;
    vSOL = 0;
    vBAS = 0;
    vRHS = 0;
    vFOR = 0;
    vTMP = 0;
    use_mB = false;
    use_mC = false;
}



void Meca::clear()
{
    objs.clear();
    nbPts = 0;
    largestBlock = 0;
}

/**
 Register a new Mecable,
 set Mecable::matIndex()
 and update Meca::nbPts and Meca::largestBlock
 */
void Meca::add(Mecable* o)
{
    objs.push_back(o);
    
    o->matIndex(nbPts);
    
    const unsigned int n = o->nbPoints();
    
    nbPts += n;
    
    if ( largestBlock < n )
        largestBlock = n;
}


//==========================================================================
//                          MAT_VECT + PROJECT
//==========================================================================
#pragma mark -

/**
 Compute the linear part of the forces.
 The forces in a system with coordinates X are:
 @code
 forces = (mB+mC)*X + vBAS
 @endcode
 
 This will perform:
 @code
 Y = Y + (mB+mC)*X
 @endcode
 */
void Meca::addLinearForces( const real* X, real* Y, const bool with_rigidity ) const
{    
#if ( DIM > 1 )
    if ( with_rigidity )
    {
        for ( Mecable ** mci = objs.begin(); mci < objs.end(); ++mci )
        {
            const index_type indx = DIM * (*mci)->matIndex();
            (*mci)->addRigidity( X+indx, Y+indx );
        }
    }
#endif

    // Y <- Y + mB * X
    if ( use_mB )
    {
#if ( DIM == 1 )
        mB.vecMulAdd( X, Y );
#elif ( DIM == 2 )
        mB.vecMulAddIso2D( X, Y );
#elif ( DIM == 3 )
        mB.vecMulAddIso3D( X, Y );
#endif
    }
    
    // Y <- Y + mC * X
    if ( use_mC )
        mC.vecMulAdd( X, Y );
}



/**
 Compute the forces.
 The forces in a system with coordinates X are:
 @code
 forces = (mB+mC)*X + vBAS
 @endcode
 
 This will perform:
 @code
 Y = (mB+mC)*X + vBAS
 @endcode
 */
void Meca::computeForces( const real* X, real* Y, const bool with_rigidity ) const
{
    blas_xcopy( DIM*nbPts, vBAS, 1, Y, 1 );
    addLinearForces( X, Y, with_rigidity );
}


//------------------------------------------------------------------------------
/**
 calculate the matrix product needed for the conjugate gradient algorithm
 @code
 Y = X - time_step * P ( mB + mC + P' ) * X;
 @endcode
*/
void Meca::multiply( const real* X, real* Y ) const
{
    // vTMP is a temporary storage !
    assert_true( X != Y  &&  X != vTMP  &&  Y != vTMP );

    // vTMP <= Forces = ( mB + mC ) * X
    blas_xzero(DIM*nbPts, vTMP);
    addLinearForces( X, vTMP, true );
    
    /*
     Constrained dynamic: Y <- X - time_step * P ( mB + mC ) * X;
     ALWAYS USE THIS!
     */
    for ( Mecable ** mci = objs.begin(); mci < objs.end(); ++mci )
    {
        Mecable const * mec = *mci;
        const index_type indx = DIM * mec->matIndex();
#ifdef PROJECTION_DIFF
        mec->addProjectionDiff( X+indx, vTMP+indx );
#endif
        mec->setSpeedsFromForces( vTMP+indx, Y+indx, -time_step );
    }
    
    blas_xaxpy(DIM*nbPts, 1.0, X, 1, Y, 1);
}

//==========================================================================
//======================   PRECONDITIONNING   ==============================
//==========================================================================
#pragma mark -

void Meca::duplicateMat( int ps, const real* X, real* Y ) const
{
    real xx = 0;
    int d, kk, ii, jj, ll;
    int bs = DIM * ps;
    
    blas_xzero(bs*bs, Y);
    
    for ( ii = 0; ii < ps; ++ii )
    {
        xx = X[ii + ps * ii];
        
        kk = ( bs+1 ) * DIM * ii;
        for ( d = 0; d < DIM; ++d, kk += bs+1 )
            Y[kk] = xx;
        
        for ( jj = ii+1; jj < ps; ++jj )
        {
            xx = X[ii + ps * jj];
            kk = DIM * ( ii + bs * jj );
            ll = DIM * ( jj + bs * ii );
            for ( d = 0; d < DIM; ++d, kk += bs+1, ll += bs+1 )
            {
                Y[kk] = xx;
                Y[ll] = xx;
            }
        }
    }
    
#if ( DEBUG_MECA == 1 )
    std::cerr << "\noriginal:\n";
    VecPrint::matPrint(std::cerr, ps, ps, X);
    std::cerr << "duplicate:\n";
    VecPrint::matPrint(std::cerr, bs, bs, Y);
#endif
}


//------------------------------------------------------------------------------
/**
 Return the diagonal-block of the full matrix corresponding to an Object
 i.e. I - time_step * P ( B + C + P' ). This is not symmetric!
*/
void Meca::getBlock(const Mecable * mec, real* blk) const
{
    const unsigned int ps = mec->nbPoints();
    const unsigned int bs = DIM * ps;
    
    Allot<real> tmp1(std::max(ps*ps, bs), 0);
    Allot<real> tmp2(bs*bs, 0);
    
    blas_xzero(ps*ps, tmp1);
    
#if ( DIM > 1 )
    MatrixSymmetric adapterMatrix( ps, tmp1 );
    mec->addRigidityMatUp( adapterMatrix, 0 );
#endif
    
    if ( use_mB )
        mB.addTriangularBlock( tmp1, mec->matIndex(), ps );
    
    duplicateMat( ps, tmp1, tmp2 );
    
    if ( use_mC )
        mC.addDiagonalBlock( tmp2, DIM*mec->matIndex(), bs );
    
#if ( DEBUG_MECA == 2 )
    std::cerr<<"mB+mC block:\n";
    VecPrint::matPrint(std::cerr, bs, bs, tmp2);
#endif
    
#ifdef PROJECTION_DIFF
    if ( 1 )
    {
        // Include the corrections P' in preconditioner, vector by vector.
        blas_xzero(bs, tmp1);
        for ( unsigned ii = 0; ii < bs; ++ii )
        {
            tmp1[ii] = 1;
            mec->addProjectionDiff( tmp1, tmp2+bs*ii );
            tmp1[ii] = 0;
        }
#if ( DEBUG_MECA == 3 )
        std::cerr<<"dynamic with P'\n";
        VecPrint::matPrint(std::cerr,bs,bs,tmp2);
#endif
    }
#endif
    
    //compute the projection, by applying it to each column vector:
    for ( unsigned ii = 0, jj = 0; ii < bs; ++ii, jj+=bs )
    {
        mec->setSpeedsFromForces(tmp2+jj, blk+jj, -time_step);
        blk[ii+jj] += 1.0;
    }
}


//------------------------------------------------------------------------------
/**
 this version builds the diagonal block directly from Meca:matVect().
 This is a very slow method: used for validation only.
*/
void Meca::getBlockS( const Mecable * mec, real* blk ) const
{
    const unsigned sz = DIM * nbPts;
    const unsigned bs = DIM * mec->nbPoints();
    const unsigned off= DIM * mec->matIndex();
    
    assert_true( off+bs <= sz );
    real * tmp1 = new real[sz];
    real * tmp2 = new real[sz];
    
    blas_xzero(sz,   tmp1);
    blas_xzero(bs*bs, blk);
    
    for ( unsigned int ii=0; ii<bs; ++ii )
    {
        tmp1[ii+off] = 1.0;
        multiply(tmp1, tmp2);
        tmp1[ii+off] = 0.0;
        for ( unsigned int jj=0; jj<bs; ++jj )
            blk[ii*bs+jj] = tmp2[jj+off];
    }
    
    delete [] tmp1;
    delete [] tmp2;
}

//------------------------------------------------------------------------------
#pragma mark -
/**
 @todo Do not call lapack_xgetri() here, but call lapack_xgetrs() in Meca::precondition()
 
 */
int Meca::computePreconditionner(Mecable* mec, int* ipiv, real* work, int worksize)
{
    assert_true( ipiv && work );
    
    int bs = DIM * mec->nbPoints();
    real* blk = mec->allocateBlock(bs);
    if ( blk == 0 )
        return 1;
    
    //we get the block corresponding to this Mecable:
    getBlock(mec, blk);
    
#if ( DEBUG_MECA == 4 )
    std::cerr<<"\nblock:\n"; VecPrint::matPrint(std::cerr,bs,bs,blk);
    // compare two methods to get the corresponding block
    real * blk2 = new real[bs*bs];    
    // get block corresponding to this object:
    getBlockS(mec, blk2);
    std::cerr<<"block2:\n"; VecPrint::matPrint(std::cerr,bs,bs,blk2);
    for ( int kk=0; kk<bs*bs; ++kk )
        blk2[kk] -= blk[kk];
    real err = blas_xnrm8(bs*bs,blk2);
    fprintf(stderr, "block difference: %f\n", err);
    delete(blk2);
#endif
    
    //invert the matrix blk by LU factorization:
    int info = 0;
    
    lapack_xgetrf( bs, bs, blk, bs, ipiv, &info );
    if ( info ) return 2;      //failed to factorize matrix !!!
    
    lapack_xgetri( bs, blk, bs, ipiv, work, worksize, &info );
    if ( info ) return 3;      //failed to invert matrix !!!

#if ( DEBUG_MECA == 5 )
    std::cerr<<"inverse:\n";
    VecPrint::matPrint(std::cerr,bs,bs,blk);
#endif

#if ( DEBUG_MECA == 6 )
    {
        // compare two methods to get the corresponding block
        real * blk2 = new real[bs*bs];
        real * RES = new real[bs*bs];
        
        for ( int kk=0; kk<bs*bs; ++kk )
        {
            RES[kk] = 0;
            blk2[kk] = 0;
        }
        
        // get block corresponding to this object:
        getBlock(mec, blk2);
        
        // printf("\nblock2:\n"); VecPrint::matPrint(std::cerr,bs,bs,blk2);
        blas_xgemm('N','N',bs,bs,bs,1.0,blk,bs,blk2,bs,0.0,RES,bs);
        
        //printf("\nblock * inverse:\n"); VecPrint::matPrint(std::cerr,bs,bs,RES);
        
        for ( int kk=0; kk<bs; ++kk )
            RES[kk+bs*kk] -= 1.0;
        real err = blas_xnrm2(bs*bs,RES,1);
        
        fprintf(stderr, "norm( 1 - block * inverse ) = %e   ", err);
        
        getBlock(mec, RES);
        for ( int kk=0; kk<bs*bs; ++kk )
            RES[kk] -= blk2[kk];
        err = blas_xnrm2(bs*bs,RES,1);
        
        fprintf(stderr, "norm( block1 - block2 ) = %e\n", err);

        delete [] blk2;
        delete [] RES;
    }
#endif
    
    return 0;
}


// Using dynamic memory to compute preconditionner
/**
 The code can be parallelized here:
 - allocate temporary memory for each thread
 - distribute block calculation to different threads
 */
int Meca::computePreconditionner()
{
    int work_size = 2048;
    
    if ( 1 )
    {
        int  tmp, info = 0;
        real w;
        const int block_size = DIM * largestBlock;
        lapack_xgetri(block_size, 0, block_size, &tmp, &w, -1, &info);
        if ( info == 0 )
        {
            work_size = (int)w;
            //std::cerr << "Lapack::dgetri optimal size is " << work_size << std::endl;
        }
    }
    
    
    // allocate memory:
    int* ipiv  = new int[DIM*largestBlock];
    real* work = new real[work_size];

    for ( Mecable ** mci = objs.begin(); mci < objs.end(); ++mci )
    {
        Mecable * mec = *mci;
        assert_true( mec->nbPoints() <= largestBlock );
        int res = computePreconditionner(mec, ipiv, work, work_size);
        mec->useBlock(res==0);
    }
    
    delete[] ipiv;
    delete[] work;
    return 0;
}


//------------------------------------------------------------------------------
void Meca::precondition(const real* X, real* Y) const
{
    for ( Mecable ** mci = objs.begin(); mci < objs.end(); ++mci )
    {
        Mecable const* mec = *mci;
        const unsigned bs = DIM * mec->nbPoints();
        const index_type indx = DIM * mec->matIndex();
        if ( mec->useBlock() )
        {
            //we use the block that was calculated
            blas_xgemv('N', bs, bs, 1.0, mec->block(), bs, X+indx, 1, 0.0, Y+indx, 1);
        }
        else
        {
            //we just 'multiply' by the Identity block
            blas_xcopy( bs, X+indx, 1, Y+indx, 1);
        }
    }
}


//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&       SOLVE        &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
#pragma mark -

void allocate(unsigned int s, real *& ptr, bool reset)
{
    if ( ptr )
        delete(ptr);
    ptr = new real[s];
    if ( reset )
        blas_xzero(s, ptr);
}


/**
 Allocate and reset matrices and vectors necessary for Meca::solve(),
 copy coordinates of Mecables into vPTS[]
 */
void Meca::prepare(SimulProp const* prop)
{
#ifndef NDEBUG
    unsigned int n = 0;
    for ( Mecable ** mci = objs.begin(); mci < objs.end(); ++mci )
    {
        assert_true( (*mci)->matIndex() == n );
        n += (*mci)->nbPoints();
        assert_true( largestBlock >= (*mci)->nbPoints() );
    }
#endif
    
    //allocate the sparse matrices:
    mB.allocate( nbPts );
    mC.allocate( DIM*nbPts );
    
    //reset matrices:
    mB.makeZero();
    mC.makeZero();
    
    //allocate the vectors
    if ( nbPts > allocated )
    {
        // Keep memory aligned to 32 bytes:
        const unsigned chunk = 32 / sizeof(real);
        // make a multiple of chunk to align pointers:
        allocated = ( nbPts + chunk - 1 ) & -chunk;
        
        allocate(DIM*allocated, vBAS, 0);
        allocate(DIM*allocated, vPTS, 1);
        allocate(DIM*allocated, vSOL, 1);
        allocate(DIM*allocated, vRHS, 1);
        allocate(DIM*allocated, vFOR, 1);
        allocate(DIM*allocated, vTMP, 0);
    }
    
    // reset vectors:
    blas_xzero(DIM*nbPts, vBAS);
    
    // get global time step
    time_step = prop->time_step;
    
    // import coordinates of mecables:
    for ( Mecable ** mci = objs.begin(); mci < objs.end(); ++mci )
    {
        Mecable * mec = *mci;
        mec->putPoints(vPTS+DIM*mec->matIndex());
        mec->prepareMecable();
    }
}



/**
 Calculates the force in the objects, that can be accessed by Mecable::netForce()
 and calculate the speed of the objects in vRHS, in the abscence of Thermal motion,
 ie. the motion is purely due to external forces.

 This also sets the Lagrange multipliers for the Fiber.
 
 The function will not change the position of the Mecables.
 */
void Meca::computeForces()
{
    // prepare matrices:
    if ( mB.nonZero() )
    {
        use_mB = true;
        mB.prepareForMultiply();
    }
    else
        use_mB = false;

    if ( mC.nonZero() )
    {
        use_mC = true;
        mC.prepareForMultiply();
    }
    else
        use_mC = false;

    // calculate forces in vFOR:
    computeForces(vPTS, vFOR, false);
    
    // return forces to Mecable, and calculate resulting speed:
    for ( Mecable ** mci = objs.begin(); mci < objs.end(); ++mci )
    {
        Mecable * mec = *mci;
        const unsigned indx = DIM * mec->matIndex();
        mec->getForces(vFOR+indx);
        mec->computeTensions(vFOR+indx);
    }
}



#define not_a_number(x) ((x) != (x))

/**
 The equation solved is: (Xnew - Xold)/dt = P*force(X) + BrownF
 
 Explicit integration:
 Xnew = Xold + dt * force(Xold) + Brown
 
 Implicit integration using the linearized force(X) = A.X + B:
 ( I - dt*P*A ) ( Xnew - Xold ) = dt * P * force(Xold) + Brown
 
 where, in both cases, Brown = sqrt(2*kT*dt*mobility) * Gaussian(0,1)
 Implicit integration is more robust.
 */
void Meca::solve(SimulProp const* prop, const bool precondition)
{
    assert_true( time_step == prop->time_step );

    if ( objs.size() == 0 )
        return;
    
    if ( mB.nonZero() )
    {
        use_mB = true;
        mB.prepareForMultiply();
    }
    else
        use_mB = false;

    if ( mC.nonZero() )
    {
        use_mC = true;
        mC.prepareForMultiply();
    }
    else
        use_mC = false;

    // calculate forces before constraints in vFOR:
    computeForces(vPTS, vFOR, true);
    
    //----------------------------------------------------------------
 
    /*
     As Brownian motions are aded, we record the magnitude of the typical smallest
     contribution to vRHS. The system will be solved with a residual that is propotional:
     SimulProp::tolerance * noiseLevel
     when SimulProp::tolerance is smaller than 1, that should work well.
     */
    
    real noiseLevel = INFINITY;
    
    //add the Brownian contribution
    for ( Mecable ** mci = objs.begin(); mci < objs.end(); ++mci )
    {
        Mecable const * mec = *mci;
        real th = mec->addBrownianForces( vFOR + DIM * mec->matIndex(), prop->kT/time_step );
        if ( th < noiseLevel )
            noiseLevel = th;
    }
    noiseLevel *= time_step;
    
    //calculate the right-hand side of the system in vRHS:
    for ( Mecable ** mci = objs.begin(); mci < objs.end(); ++mci )
    {
        Mecable const * mec = *mci;
        const index_type indx = DIM * mec->matIndex();
        mec->setSpeedsFromForces( vFOR+indx, vRHS+indx, time_step, true );
    }
    
#ifdef NEW_CYTOPLASMIC_FLOW
    
    /**
     Includes a constant fluid flow displacing all the objects along
     */
    if ( prop->flow.norm() > REAL_EPSILON )
    {
        MSG_ONCE("NEW_CYTOPLASMIC_FLOW code enabled\n");
        Vector flow_dt = prop->flow * time_step;
        
        const real *const end = vRHS + DIM*nbPts;
        
        for ( real * mx = vRHS; mx < end; mx += DIM )
            flow_dt.add_to(mx);
    }
    
#endif

#ifdef EXPLICIT
    /*  This implements a forward Euler integration, for testing purposes
        It is very slow, since we have built the entire stiffness matrix,
        which is not necessary for this explicit scheme.
        */
    blas_xaxpy(DIM*nbPts, 1, vRHS, 1, vPTS, 1);
    for ( Mecable ** mci = objs.begin(); mci < objs.end(); ++mci )
    {
        Mecable * mec = *mci;
        mec->getPoints(vPTS+DIM*objs[ii]->matIndex());
        mec->getForces(vFOR+DIM*objs[ii]->matIndex());
    }
    return;
#endif
    
#ifdef PROJECTION_DIFF
    //set the differential of the projection with the current forces:
    for ( Mecable ** mci = objs.begin(); mci < objs.end(); ++mci )
    {
        Mecable * mec = *mci;
        mec->makeProjectionDiff(vFOR+DIM*mec->matIndex());
    }
#endif
    
    /*
     Choose the initial guess for the solution of the system (Xnew - Xold):
     we could use the solution at the previous step, or a vector of zeros.
     Using the previous solution could be advantageous if the speed were 
     somehow continuous. However, the system is without inertia. In addition,
     objects are considered in a random order to build the linear system, such
     that the blocks from two consecutive iterations do not match.
     Using zero for the initial guess seems a safe bet:
     */
    blas_xzero(DIM*nbPts, vSOL);

    /*
     We now solve the system MAT * vSOL = vRHS  by an iterative method:
     the tolerance is in scaled to the contribution of Brownian
     motions contained in vRHS, assuming that the error is equally spread 
     along all degrees of freedom, this should work for tolerance << 1
     here a printf() can be used to check that the estimate is correct:
    */ 
     //printf("noiseLeveld = %8.2e   variance(vRHS) / estimate = %8.4f\n", 
     //       noiseLevel, blas_xnrm2(DIM*nbPts, vRHS, 1) / (noiseLevel * sqrt(DIM*nbPts)) );
    
    // TEST: the tolerance to solve the system should be such that the solution
    // found does not depend on the initial guess.
    
    assert_true( noiseLevel > 0 );

    /*
     One can use static memory by making the Allocator static
     */
    static Solver::Allocator allocator;

    /*
     With exact arithmetic, biConjugate Gradient should converge at most
     in a number of iterations equal to the size of the linear system.
     This is the max limit that is set here to the number of iterations:
     */
    Solver::Monitor monitor(DIM*nbPts, prop->tolerance*noiseLevel);

    //------- call the iterative solver:
    //@todo: we may not need to calculate the preconditioner at every step
    //std::cerr << "Solve: " << DIM*nbPts << "  " << residual_ask << std::endl;

    if ( precondition  &&  0 == computePreconditionner() ) 
        Solver::BCGSP(*this, vRHS, vSOL, monitor, allocator);
    else
        Solver::BCGS(*this, vRHS, vSOL, monitor, allocator);
    
#if ( 0 )
    std::cerr << "BCGS" << precondition << "  " << code;
    std::cerr << " iter " << monitor.iterations() << " " << monitor.residual() << std::endl;
#endif
    
    //------- in case the solver did not converge, we try other methods:
    
    if ( !monitor.converged() )
    {
        Cytosim::MSG("Solver failed: precond %i flag %i, nb_iter %3i residual %.2e\n", 
            precondition, monitor.flag(), monitor.iterations(), monitor.residual());
        
        //---we try vRHS as a different initial seed:
        blas_xcopy(DIM*nbPts, vRHS, 1, vSOL, 1);
        
        //---reset tolerance and iteration counters:
        monitor.reset();
        
        //---try the same method again:
        if ( precondition )
            Solver::BCGSP(*this, vRHS, vSOL, monitor, allocator);
        else
            Solver::BCGS(*this, vRHS, vSOL, monitor, allocator);
        
        //---check again for convergence:
        if ( monitor.converged() )
            Cytosim::MSG("Solver rescued by changing seed: nb_iter %3i residual %.2e\n", monitor.iterations(), monitor.residual());
        else 
        {
            //---use zero as an initial guess:
            blas_xzero(DIM*nbPts, vSOL);
            
            //---reset tolerance and iteration counters:
            monitor.reset();
            
            //---try the other method:
            if ( precondition ) {
                Solver::BCGS(*this, vRHS, vSOL, monitor, allocator);
            }
            else {
                if ( 0 == computePreconditionner() )
                    Solver::BCGSP(*this, vRHS, vSOL, monitor, allocator);
                else
                    Cytosim::MSG("Failed to compute precondionner");
            }
            
            //---check again for convergence:
            if ( monitor.converged() )
                Cytosim::MSG("Solver rescued by changing precond: nb_iter %3i residual %.2e\n", monitor.iterations(), monitor.residual());
            else {
                //no solver could converge... this is really bad!
                //we could still try to change the initial guess, to recover convergence
                Cytosim::MSG("Solver dead nb_iter %i residual %.2e\n", monitor.iterations(), monitor.residual());
                throw Exception("convergence failure in solver");
                return;
            }
        }
    }
    
    //add the solution of the system (=dPTS) to the points coordinates
    blas_xaxpy(DIM*nbPts, 1., vSOL, 1, vPTS, 1);
    
    
#ifndef NDEBUG
    
    //check validity of the data (slow):
    for( unsigned int ii = 0; ii < DIM*nbPts; ++ii )
    {
        if ( not_a_number(vPTS[ii]) )
        {
            std::cerr << "Solve produced invalid results (NAN)" << std::endl;
            abort();
        }
    }
    
#endif

    /*
     Re-calculate forces once the objects have moved:
     Bending elasticity of fibers and other internal forces are not included,
     as well as the Brownian terms in vFOR (which are not included in vBAS).
     In this way the result returned to the fibers does not sum-up to zero,
     and is appropriate for example to calculate the effect of force on assembly.
     */
    computeForces(vPTS, vFOR, false);
    
    
    // export new coordinates to Mecables:
    for ( Mecable ** mci = objs.begin(); mci < objs.end(); ++mci )
    {
        Mecable * mec = *mci;
        mec->getPoints(vPTS+DIM*mec->matIndex());
        mec->getForces(vFOR+DIM*mec->matIndex());
    }
    
    //report on the matrix type and size, sparsity, and the number of iterations
    if ( prop->verbose )
    {
        Cytosim::MSG("Meca degree %i*%-5i", DIM, nbPts);
        if ( use_mB ) Cytosim::MSG(" iso: %s ", mB.what().c_str());
        if ( use_mC ) Cytosim::MSG(" mat: %s ", mC.what().c_str());
        Cytosim::MSG(" precond %i  nb_iter %i  residual %.2e\n", precondition, monitor.iterations(), monitor.residual());
    }
}






//==========================================================================
//                            DEBUG  -  DUMP
//==========================================================================
#pragma mark -

/**
 Extract and print the full matrix associated with matVect, for debugging purposes
 */
void Meca::printMatrix(std::ostream & os) const
{
    const unsigned sz = size();
    
    real * src = new real[sz];
    real * res = new real[sz];
    
    blas_xzero(sz, src);
    blas_xzero(sz, res);
    
    for ( unsigned ii=0; ii<sz; ++ii )
    {
        src[ii] = 1.0;
        multiply(src, res);
        VecPrint::vecPrint(os, sz, res);
        src[ii] = 0.0;
    }
    
    delete [] res;
    delete [] src;
}


/**
 Save the full matrix associated with matVect, for debugging purposes
 */
void Meca::dumpMatrix(FILE * file) const
{
    const unsigned sz = size();
    
    real * src = new real[sz];
    real * res = new real[sz];
    
    blas_xzero(sz, src);
    blas_xzero(sz, res);
    
    for ( unsigned ii=0; ii<sz; ++ii )
    {
        src[ii] = 1.0;
        multiply(src, res);
        fwrite(res, sizeof(real), sz, file);
        src[ii] = 0.0;
    }
    
    delete [] res;
    delete [] src;
}

/**
 Save the projection matrix 
 */
void Meca::dumpProjection(FILE * file, Mecable const * mec, bool diff) const
{
    const unsigned sz = DIM * mec->nbPoints();
    
    real * src = new real[sz];
    real * res = new real[sz];
    
    blas_xzero(sz, src);
    blas_xzero(sz, res);
    
    for ( unsigned ii=0; ii<sz; ++ii )
    {
        src[ii] = 1.0;
#ifdef PROJECTION_DIFF
        if ( diff )
            mec->addProjectionDiff(src, res);
        else
#endif
            mec->setSpeedsFromForces(src, res);
        fwrite(res, sizeof(real), sz, file);
        src[ii] = 0.0;
    }
    
    delete [] res;
    delete [] src;
}


void Meca::dumpDiagonal(FILE * file) const
{
    for ( unsigned ii = 0; ii < objs.size(); ++ii )
    {
        Mecable const* mec = objs[ii];
        const real drag = mec->dragCoefficient()/mec->nbPoints();
        for ( unsigned p=0; p < mec->nbPoints(); ++p )
        {
            Vector pos = mec->posPoint(p);
            for ( unsigned int d = 0; d < DIM; ++d )
                fprintf(file,"%5u %16.8e %16.8e\n", ii, pos[d], drag);
        }
    }
}


/**
 This dump the matrix and some vectors in binary files.
 
 Here is some matlab code to read the output:
 @code
 dim = load('ddim.txt');
 mat = fread(fopen('dmat.bin'), [dim, dim], 'double');
 rhs = fread(fopen('drhs.bin'), dim, 'double');
 sol = fread(fopen('dsol.bin'), dim, 'double');
 @endcode
 
 And to compare the results with matlab's own method,
 using a scatter plot:
 
 @code
 x = bicgstab(mat, rhs, 0.001, 100);
 plot(x, sol, '.');
 @endcode
 */
void Meca::dump() const
{
    FILE * file = fopen("ddim.txt", "w");
    fprintf(file, "%i\n", size());
    fclose(file);
    
    FILE* f = fopen("diagonal.txt", "w");
    dumpDiagonal(f);
    fclose(f);

    file = fopen("drhs.bin", "wb");
    fwrite(vRHS, sizeof(real), size(), file);
    fclose(file);
    
    file = fopen("dsol.bin", "wb");
    fwrite(vSOL, sizeof(real), size(), file);
    fclose(file);

    file = fopen("dpts.bin", "wb");
    fwrite(vPTS, sizeof(real), size(), file);
    fclose(file);

    file = fopen("dmat.bin", "wb");
    dumpMatrix(file);
    fclose(file);
    
    std::cerr << " Dumped a system of size " << size() << std::endl;
}
