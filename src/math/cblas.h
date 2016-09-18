// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/**
 This contains C front-ends to some functions of BLAS
 see http://www.netlib.org/blas
  
 Functions are renamed : 
 
 blas_xcopy calls scopy if ( real is float ),
               or dcopy if ( real is double ).
*/

#ifndef CBLAS_H 
#define CBLAS_H

#include "real.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif
    
#undef FORTRAN
    
#ifdef REAL_IS_FLOAT
#define FORTRAN(x) s##x##_
#define iFORTRAN(x) is##x##_
#else
#define FORTRAN(x) d##x##_
#define iFORTRAN(x) id##x##_
#endif

/*
 * ===========================================================================
 * Prototypes for level 1 BLAS routines
 * ===========================================================================
 */
#pragma mark -


real FORTRAN(dot)(int*, const real*, int*, const real*, int*);
inline real blas_xdot(int N, const real* X, int incX, const real* Y, int incY)
{
    return FORTRAN(dot)(&N, X, &incX, Y, &incY);
}

real FORTRAN(nrm2)(int*, const real*, int*);
inline real blas_xnrm2(int N, const real*X, int incX)
{
    return FORTRAN(nrm2)(&N, X, &incX);
}

real FORTRAN(asum)(int*, const real*, int*);
inline real blas_xasum(int N, const real*X, int incX)
{
    return FORTRAN(asum)(&N, X, &incX);
}

/// This is not in the standard BLAS
real FORTRAN(sum)(int*, const real*, int*);
inline real blas_xsum(int N, const real*X, int incX)
{
    return FORTRAN(sum)(&N, X, &incX);
}

int iFORTRAN(amax)(int*, const real*, int*);
inline int blas_ixamax(int N, const real*X, int incX)
{
    return (iFORTRAN(amax)(&N, X, &incX) - 1);
}

int iFORTRAN(max)(int*, const real*, int*);
inline int blas_ixmax(int N, const real*X, int incX)
{
    return (iFORTRAN(max)(&N, X, &incX) - 1);
}

int iFORTRAN(amin)(int*, const real*, int*);
inline int blas_ixamin(int N, const real*X, int incX)
{
    return (iFORTRAN(amin)(&N, X, &incX) - 1);
}

int iFORTRAN(min)(int*, const real*, int*);
inline int blas_ixmin(int N, const real*X, int incX)
{
    return (iFORTRAN(min)(&N, X, &incX) - 1);
}

void FORTRAN(swap)(int*, real*, int*, real*, int*);
inline void blas_xswap(int N, real*X, int incX, real*Y, int incY)
{
    FORTRAN(swap)(&N, X, &incX, Y, &incY);
}

void FORTRAN(copy)(int*, const real*, int*, real*, int*);
inline void blas_xcopy(int N, const real*X, int incX, real*Y, int incY)
{
    FORTRAN(copy)(&N, X, &incX, Y, &incY);
}

void FORTRAN(axpy)(int*, real*, const real*, int*, real*, int*);
inline void blas_xaxpy(int N, real alpha, const real*X, int incX, real*Y, int incY)
{
    FORTRAN(axpy)(&N, &alpha, X, &incX, Y, &incY );
}

#ifdef __INTEL_MKL__
/**
 axpby() is an addition of Intel to the blas routines,
 but it does not exist in the BLAS standard implementation
 */
void FORTRAN(axpby)(int*, real*, const real*, int*, real*, real*, int*);
inline void blas_xaxpby(int N, real alpha, const real*X, int incX, real beta, real*Y, int incY)
{
    FORTRAN(axpby)(&N, &alpha, X, &incX, &beta, Y, &incY );
}
#endif
    
void FORTRAN(rotg)(real*, real*, real*, real*);
inline void blas_xrotg(real*a, real*b, real*c, real*s)
{
    FORTRAN(rotg)(a, b, c, s);
}

void FORTRAN(rotmg)(const real*, const real*, const real*, real*, real*);
inline void blas_xrotmg(const real*d1, const real*d2, const real*b1, real b2, real*P)
{
    FORTRAN(rotmg)(d1, d2, b1, &b2, P);
}

void FORTRAN(rot)(int*, real*, int*, real*, int*, real*, real*);
inline void blas_xrot( int N, real*X, int incX, real*Y, int incY, real c, real s)
{
    FORTRAN(rot)(&N, X, &incX, Y, &incY, &c, &s);
}

void FORTRAN(rotm)(int*, real*, int*, real*, int*, real*);
inline void blas_xrotm( int N, real*X, int incX, real*Y, int incY, real*P)
{
    FORTRAN(rotm)(&N, X, &incX, Y, &incY, P);
}

void FORTRAN(scal)(int*, real*, real*, int*);
inline void blas_xscal(int N, real alpha, real*X, int incX)
{
    FORTRAN(scal)( &N, &alpha, X, &incX);
}

/*
 * ===========================================================================
 * Prototypes for level 2 BLAS
 * ===========================================================================
 */
#pragma mark -


void FORTRAN(gemv)(char*, int*, int*, real*, const real*, int*, const real*, int*, real*, real*, int*);
inline void blas_xgemv(char TransA, int M, int N, real alpha, const real*A, int lda,
                       const real*X, int incX, real beta, real*Y, int incY)
{
    FORTRAN(gemv)(&TransA, &M, &N, &alpha, A, &lda, X, &incX, &beta, Y, &incY);
}

void FORTRAN(trmv)( char*, char*, char*, int*, const real*, int*, real*, int*);
inline void blas_xtrmv( char Uplo, char TransA, char Diag, int N, const real*A, int lda, real*X, int incX)
{
    FORTRAN(trmv)(&Uplo, &TransA, &Diag, &N, A, &lda, X, &incX);
    
}

inline void blas_xtrsv(char Uplo, char TransA, char Diag, int N, const real*A, int lda, real*X, int incX);

inline void blas_xgbmv(char TransA, int M, int N, int Kl,  int Ku, real alpha, const real*A, int lda, const real*X, int incX, real beta, real*Y, int incY);

inline void blas_xtbmv(char Uplo, char TransA, char Diag, int N, int K, const real*A, int lda, real*X, int incX);

inline void blas_xtbsv(char Uplo, char TransA, char Diag, int N, int K, const real*A, int lda, real*X, int incX);

inline void blas_xtpsv(char Uplo, char TransA, char Diag, int N, const real*Ap, real*X, int incX);

inline void blas_xtpmv(char Uplo, char TransA, char Diag, int N, const real*Ap, real*X, int incX);

void FORTRAN(ger)(int*, int*, real* alpha, const real*, int*, const real*, int*, real*, int*);
inline void blas_xger(int M, int N, real alpha, const real*X, int incX, const real*Y, int incY, real*A, int lda)
{
    FORTRAN(ger)(&M, &N, &alpha, X, &incX, Y, &incY, A, &lda);
}


void FORTRAN(symv)(char*, int*, real*, const real*, int*, const real*, int*, real*, real*, int*);
inline void blas_xsymv(char Uplo, int N, real alpha,  const real*A, int lda, const real*X, int incX, real beta, real*Y, int incY)
{
    FORTRAN(symv)(&Uplo,&N,&alpha,A,&lda,X,&incX,&beta,Y,&incY);
}

void FORTRAN(sbmv)(char*, int*, int*, real*, const real*, int*, const real*, int*, real*, real*, int*);
inline void blas_xsbmv(char Uplo, int N, int K, real alpha, const real*A, int lda, const real*X, int incX, real beta, real*Y, int incY)
{
    FORTRAN(sbmv)(&Uplo,&N,&K,&alpha,A,&lda,X,&incX,&beta,Y,&incY);
}

void FORTRAN(spmv)(char*, int*, real*, const real*, const real*, int*, real*, real*, int*);
inline void blas_xspmv(char Uplo, int N, real alpha, const real*A, const real*X, int incX, real beta, real*Y, int incY)
{
    FORTRAN(spmv)(&Uplo,&N,&alpha,A,X,&incX,&beta,Y,&incY);
}

void FORTRAN(syr)(char*, int*, real*, const real*, int*, real*, int*);
inline void blas_xsyr(char Uplo, int N, real alpha, const real*X, int incX, real*A, int lda)
{
    FORTRAN(syr)(&Uplo, &N, &alpha, X, &incX, A, &lda);
}

void FORTRAN(syr2)(char*, int*, real*, const real*, int*, const real*, int*, real*, int*);
inline void blas_xsyr2(char Uplo, int N, real alpha, const real*X, int incX, const real*Y, int incY, real* A, int lda)
{
    FORTRAN(syr2)(&Uplo, &N, &alpha, X, &incX, Y, &incY, A, &lda);
}

    
void FORTRAN(spr)(char*, int*, real*, const real*, int*, real*);
inline void blas_xspr(char Uplo, int N, real alpha, const real*X, int incX, real*Ap)
{
    FORTRAN(spr)(&Uplo, &N, &alpha, X, &incX, Ap);
}

inline void blas_xspr2(char Uplo, int N, real alpha, const real*X, int incX, const real*Y, int incY, real*A);

/*
 * ===========================================================================
 * Prototypes for level 3 BLAS
 * ===========================================================================
 */
#pragma mark -


void FORTRAN(gemm)(char*, char*, int*, int*, int*, real*, const real*, int*, const real*, int*, real*, real*, int*);
inline void blas_xgemm(char TransA, char TransB, int M, int N, int K, real alpha, const real*A,
                       int lda, const real*B, int ldb, real beta, real*C, int ldc)
{
    FORTRAN(gemm)(&TransA,&TransB,&M,&N,&K,&alpha,A,&lda,B,&ldb,&beta,C,&ldc);
}

void FORTRAN(symm)(char*, char*, int*, int*, real*, const real*, int*, const real*, int*, real*, real*, int*);
inline void blas_xsymm(char Side, char Uplo, int M, int N, real alpha, const real*A, int lda,
                       const real*B, int ldb, real beta, real*C, int ldc)
{
    FORTRAN(symm)(&Side,&Uplo,&M,&N,&alpha,A,&lda,B,&ldb,&beta,C,&ldc);
}


void FORTRAN(syrk)(char*, char*, int*, int*, real*, const real*, int*, real*, real*, int*);
inline void blas_xsyrk(char Uplo, char Trans, int N, int K, real alpha, const real*A, int lda, real beta, real*C, int ldc)
{
    FORTRAN(syrk)(&Uplo,&Trans,&N,&K,&alpha,A,&lda,&beta,C,&ldc);
}

inline void blas_xsyr2k(char Uplo, char Trans, int N, int K, real alpha, const real*A, int lda, const real*B, int ldb, real beta, real*C, int ldc);

inline void blas_xtrmm(char Uplo, char TransA, char Diag, int M, int N, real alpha, const real*A, int lda, real*B, int ldb);

void FORTRAN(trsm)(char*, char*, char*, char*, int*, int*, real*, const real*, int*, real*, int*);
inline void blas_xtrsm(char side, char uplo, char transA, char diag, int M, int N, real alpha, const real*A, int lda, real*B, int ldb)
{
    FORTRAN(trsm)(&side, &uplo, &transA, &diag, &M, &N, &alpha, A, &lda, B, &ldb);
}

/*
 * ===========================================================================
 * non-standard additions to blas by Francois Nedelec
 * ===========================================================================
 */


/**
 return the infinite norm of the vector
 @code
 int indx = blas_ixamax(N, X, inc);
 return fabs(X[indx]);
 @endcode
 */
inline real blas_xnrm8(const int N, const real* X)
{
#if ( 1 )
    int indx = blas_ixamax(N, X, 1);
    if ( X[indx] > 0 )
        return  X[indx];
    else
        return -X[indx];
#else
    real min = X[0];
    real max = X[0];
    for ( int u = 1; u < N; ++u )
    {
        if ( max < X[u] ) max = X[u];
        if ( min > X[u] ) min = X[u];
    }
    return max > -min ? max : -min;
#endif
}

/**
 set every value of the vector `X` to `value` (by default = zero).
 It would be equivalent to use
 @code
 real value = 0;
 blas_xcopy(N, &value, 0, X, inc);
 @endcode
 */
inline void blas_xzero(const int N, real* X)
{
#if ( 1 )
    memset(X, 0, N*sizeof(real));
#else
    for ( int u = 0; u < N; ++u )
        X[u] = 0;
#endif
}

inline void blas_xset(const int N, real* X, const int inc, real value)
{
    for ( int u = 0; u < N; u+=inc )
        X[u] = value;
}

    
    
/**
 blas_dfdot() uses double precision to calculate the dot-product
 of two single-precision vectors, to increase precision.
*/
inline double blas_dfdot(int size, float * X, int incX, float * Y, int incY)
{
    double res = 0;
        
    if ( incX == 1  &&  incY == 1 )
    {
        if ( X == Y )
        {
            for ( int ii = 0; ii < size; ++ii )
                res += double(X[ii]) * double(X[ii]);
        }
        else
        {
            for ( int ii = 0; ii < size; ++ii )
                res += double(X[ii]) * double(Y[ii]);
        }
    }
    else
    {
        for ( int ii = 0; ii < size; ++ii )
            res +=  double(X[incX*ii])  *  double(Y[incY*ii]);
    }
    return res;
}

#ifdef __cplusplus
} //extern "C"
#endif

#endif
