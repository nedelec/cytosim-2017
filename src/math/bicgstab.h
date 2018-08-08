// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#ifndef BICGSTAB_H
#define BICGSTAB_H

#include <iostream>
#include <cmath>

#include "real.h"
#include "cblas.h"

#ifdef REAL_IS_FLOAT
#    define DOT blas_dfdot
#else
#    define DOT blas_xdot
#endif

/// Templated iterative solvers for systems of linear equations
/**
 The linear system (and the preconditionner) is defined by class LinearOperator:    
 @code
    class LinearOperator
    {
    public:
        /// size of the matrix
        unsigned int size() const;
        
        /// apply operator to a vector
        void multiply(const real*, real*) const;
        
        /// apply transposed operator to vector
        void transMultiply(const real*, real*) const;
        
        /// apply preconditionning 
        void precondition(const real*, real*) const;
    };
 @endcode
 
 The iterative solver is followed by class Monitor,
 where the desired convergence criteria can be specified.
 Monitor will also keep track of iteration counts.
 An suitable implementation of Monitor is given here.
 
 F. Nedelec, 27.03.2012 - 21.02.2013
*/
namespace Solver
{
    
    /// records the number of iterations, and the convergence
    class Monitor
    {
    private:
        
        int      mFlag;
        
        unsigned mIter,  mIterMax, mIterOld;
        
        real     mResid, mResidMax, mResidOld;
        
    public:
        
        /// set the maximum number of iterations, and the residual threshold
        Monitor(unsigned i, real r) { reset(); mIterMax = i; mResidMax = r; }
        
        /// destructor
        virtual ~Monitor() {};
        
        /// reset interation count and achieved residual
        void reset() { mFlag = 0; mIter = 0; mResid = INFINITY; mIterOld = 0; mResidOld = INFINITY; }
        
        /// increment iteration count
        void operator ++() { ++mIter; }
        
        /// the termination code
        int flag()       const { return mFlag; }
        
        /// iteration count
        int iterations() const { return mIter; }
        
        /// last achieved residual
        real residual()  const { return mResid; }
        
        /// true if achieve residual < residual threshold
        bool converged() const { return mResid < mResidMax; }
        
        /// calculate residual from \a x and return true if threshold is achieved
        virtual bool finished(unsigned size, real const* x);

        /// calculate residual from \a x and return true if threshold is achieved
        virtual bool finished(int f, unsigned size, real const* x) { mFlag = f; return finished(size, x); }
    };
    
    
    /// allocates vectors of real
    class Allocator
    {
    private:
        
        /// type for size
        typedef size_t size_type;
        
        /// size of the vector to be allocated
        size_type siz;
        
        /// number of vectors allocated
        size_type alc;
        
        /// memory
        real    * mem;
        
    public:
        
        Allocator()  { siz = 0; alc = 0; mem = 0; }
        
        ~Allocator() { release(); }
        
        void allocate(size_type s, unsigned int n)
        {
            // Keep memory aligned to 32 bytes:
            const unsigned chunk = 32 / sizeof(real);
            siz = ( s + chunk - 1 ) & -chunk;
            size_t a = siz * n;
            if ( a > alc )
            {
                if ( mem )
                    delete(mem);
                mem = new real[a];
                alc = a;
                //std::cerr << "Allocator::allocate "<<a<<std::endl;
            }
        }
        
        void release()
        {
            if ( mem )
            {
                delete(mem);
                //std::cerr << "Allocator::release "<<std::endl;
            }
            mem = 0;
            alc = 0;
            siz = 0;
        }
        
        void relax()
        {
            //release();
        }
        
        real * bind(unsigned int i)
        {
            if ( mem == 0 )
                return 0;
            if ( (i+1)*siz > alc )
                return 0;
            //std::cerr << "Allocator::bind " << i << " " << mem+i*siz << std::endl;
            return mem + i * siz;
        }
    };
    
    /// Bi-Conjugate Gradient Stabilized without Preconditionning
    template < typename LinearOperator, typename Monitor, typename Allocator >
    void BCGS(const LinearOperator& mat, const real* rhs, real* x, Monitor& monitor, Allocator& allocator)
    {
        double rho_1 = 1, rho_2, alpha = 0, beta = 0, omega = 1;
        
        const unsigned int size = mat.size();
        allocator.allocate(size, 5);
        real * r      = allocator.bind(0);
        real * rtilde = allocator.bind(1);
        real * p      = allocator.bind(2);
        real * t      = allocator.bind(3);
        real * v      = allocator.bind(4);
        
        blas_xcopy(size, rhs, 1, r, 1);
        mat.multiply(x, rtilde);
        blas_xaxpy(size, -1.0, rtilde, 1, r, 1);      // r = rhs - A * x
        blas_xcopy(size, r, 1, rtilde, 1);
        
        while ( ! monitor.finished(size, r) )
        {
            rho_2 = rho_1;
            rho_1 = DOT(size, rtilde, 1, r, 1);
            
            if ( rho_1 == 0.0 )
            {
                monitor.finished(2, size, r);
                break;
            }
            
            beta = ( rho_1 / rho_2 ) * ( alpha / omega );
            if ( beta == 0.0 )
            {
                // p = r;
                blas_xcopy(size, r, 1, p, 1);
            }
            else {
                // p = r + beta * ( p - omega * v )
                blas_xaxpy(size, -omega, v, 1, p, 1);
#ifdef __INTEL_MKL__
                blas_xaxpby(size, 1.0, r, 1, beta, p, 1);
#else
                blas_xscal(size, beta, p, 1);
                blas_xaxpy(size, 1.0, r, 1, p, 1);
#endif
            }
            
            mat.multiply( p, v );                     // v = A * p;
            alpha = rho_1 / DOT(size, rtilde, 1,  v, 1);
            
            blas_xaxpy(size, -alpha, v, 1, r, 1);     // r = r - alpha * v;
            blas_xaxpy(size,  alpha, p, 1, x, 1);     // x = x + alpha * p;
            
            //if ( monitor.finished(size, r) )
            //    break;
            
            mat.multiply( r, t );                     // t = A * r;
            
            real tdt = DOT(size, t, 1, t, 1);
            
            if ( tdt == 0.0 )
            {
                monitor.finished(0, size, r);
                break;
            }

            omega = DOT(size, t, 1, r, 1) / tdt;
            
            if ( omega == 0.0 )
            {
                monitor.finished(3, size, r);
                break;
            }
            
            blas_xaxpy(size,  omega, r, 1, x, 1);     // x = x + omega * r;
            blas_xaxpy(size, -omega, t, 1, r, 1);     // r = r - omega * t;
            
            ++monitor;
         }
        
        allocator.relax();
    }
    
    
    /// Bi-Conjugate Gradient Stabilized with Preconditionning
    template < typename LinearOperator, typename Monitor, typename Allocator >
    void BCGSP(const LinearOperator& mat, const real* rhs, real* x, Monitor& monitor, Allocator& allocator)
    {
        double rho_1 = 1, rho_2, alpha = 0, beta = 0, omega = 1.0, delta;
        
        const unsigned int size = mat.size();
        allocator.allocate(size, 7);
        real * r      = allocator.bind(0);
        real * rtilde = allocator.bind(1);
        real * p      = allocator.bind(2);
        real * t      = allocator.bind(3);
        real * v      = allocator.bind(4);
        real * phat   = allocator.bind(5);
        real * shat   = allocator.bind(6);
        
        blas_xcopy(size, rhs, 1, r, 1);
        mat.multiply(x, rtilde);
        blas_xaxpy(size, -1.0, rtilde, 1, r, 1);        // r = rhs - A * x
        blas_xcopy(size, r, 1, rtilde, 1);              // r_tilde = r
        
        while ( ! monitor.finished(size, r) )
        {
            rho_2 = rho_1;
            rho_1 = DOT(size, rtilde, 1, r, 1);
            
            if ( rho_1 == 0.0 )
            {
                monitor.finished(2, size, r);
                break;
            }
            
            beta = ( rho_1 / rho_2 ) * ( alpha / omega );
            if ( beta == 0.0 )
            {
                // p = r;
                blas_xcopy(size, r, 1, p, 1);
            }
            else {
                // p = r + beta * ( p - omega * v )
                blas_xaxpy(size, -omega, v, 1, p, 1);
#ifdef __INTEL_MKL__
                blas_xaxpby(size, 1.0, r, 1, beta, p, 1);
#else
                blas_xscal(size, beta, p, 1);
                blas_xaxpy(size, 1.0, r, 1, p, 1);
#endif
            }
            
            mat.precondition( p, phat );                // phat = PC * p;
            mat.multiply( phat, v );                    // v = M * phat;
            
            delta = DOT(size, rtilde, 1,  v, 1);
            if ( delta == 0.0 )
            {
                monitor.finished(4, size, r);
                break;
            }
            
            alpha = rho_1 / delta;
            blas_xaxpy(size, -alpha, v, 1, r, 1);       // r = r - alpha * v;
            blas_xaxpy(size,  alpha, phat, 1, x, 1);    // x = x + alpha * phat;

            //if ( monitor.finished(size, r) )
            //    break;

            mat.precondition( r, shat );                // shat = PC * r
            mat.multiply( shat, t );                    // t = M * shat
            
            real tdt = DOT(size, t, 1, t, 1);
            
            if ( tdt == 0.0 )
            {
                monitor.finished(0, size, r);
                break;
            }
            
            omega = DOT(size, t, 1, r, 1) / tdt;
            
            if ( omega == 0.0 )
            {
                monitor.finished(3, size, r);
                break;
            }
            
            blas_xaxpy(size,  omega, shat, 1, x, 1);    // x = x + omega * shat
            blas_xaxpy(size, -omega, t, 1, r, 1);       // r = r - omega * t
            
            ++monitor;
        }
        
        allocator.relax();
    }
};

#endif

