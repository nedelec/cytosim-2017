// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/*
 This contains C-wrappers around some of the Fortran routines of LAPACK.
 see http://www.netlib.org/lapack
 
 LAPACK contains more than 1000 linear algebra functions in FORTRAN,
 but here we just propagate the functions needed in Cytosim,
 allowing direct link with the fortran LAPACK library
 */


#ifndef LAPACK_H
#define LAPACK_H

#include "cblas.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    void FORTRAN(ptsv)(int*, int*, real*, real*, real*, int*, int* );
    inline void lapack_xptsv(int N, int NHRS, real* D, real* E, real* B, int LDB, int* INFO)
    {
        FORTRAN(ptsv)(&N, &NHRS, D, E, B, &LDB, INFO );
    }
    
    
    void FORTRAN(ptsvx)(char *, int*, int*, const real*, const real*, real*, real*,
                        const real*, int*, real*, int*, real*, real*, real*, real*, int* );
    inline void lapack_xptsvx(char fact, int N, int NHRS, const real* D, const real* E, real* DF, real* EF,
                              const real* B, int LDB, real* X, int LDX, real* RCOND, real* FERR, real* BERR, real* work, int* INFO)
    {
        FORTRAN(ptsvx)(&fact, &N, &NHRS, D, E, DF, EF, B, &LDB, X, &LDX, RCOND, FERR, BERR, work, INFO );
    }
    
    
    void FORTRAN(pttrf)(int*, real*, real*, int*);
    inline void lapack_xpttrf(int N, real* D, real* E, int* INFO)
    {
        FORTRAN(pttrf)(&N, D, E, INFO );
    }
    
    
    void FORTRAN(pttrs)(int*, int*, const real*, const real*, real*, int*, int*);
    inline void lapack_xpttrs(int N, int NHRS, const real* D, const real* E, real* B, int LDB, int* INFO )
    {
        FORTRAN(pttrs)(&N, &NHRS, D, E, B, &LDB, INFO );
    }
    
    void FORTRAN(ptts2)(int*, int*, const real*, const real*, real*, int*);
    inline void lapack_xptts2(int N, int NHRS, const real* D, const real* E, real* B, int LDB )
    {
        FORTRAN(ptts2)( &N, &NHRS, D, E, B, &LDB );
    }
    
    
    void FORTRAN(posv)(char *, int*, int*, real*, int*, real*, int*, int*);
    inline void lapack_xposv(char UPLO, int N, int NHRS, real* A, int LDA, real* B, int LDB, int* INFO)
    {
        FORTRAN(posv)(&UPLO, &N, &NHRS, A, &LDA, B, &LDB, INFO );
    }
    
    
    void FORTRAN(potrf)(char *, int*, real*, int*, int*);
    inline void lapack_xpotrf(char UPLO, int N, real* A, int LDA, int* INFO)
    {
        FORTRAN(potrf)(&UPLO, &N, A, &LDA, INFO );
    }
    
    
    void FORTRAN(potrs)(char *, int*, int*, const real*, int*, real*, int*, int*);
    inline void lapack_xpotrs(char UPLO, int N, int NHRS, const real* A, int LDA, real* B, int LDB, int* INFO)
    {
        FORTRAN(potrs)(&UPLO, &N, &NHRS, A, &LDA, B, &LDB, INFO );
    }
    
    
    void FORTRAN(potf2)(char *, int*, real*, int*, int*);
    inline void lapack_xpotf2(char UPLO, int N, real* A, int LDA, int* INFO)
    {
        FORTRAN(potf2)(&UPLO, &N, A, &LDA, INFO );
    }
    
    
    void FORTRAN(potri)(char *, int*, real*, int*, int*);
    inline void lapack_xpotri(char UPLO, int N, real* A, int LDA, int* INFO)
    {
        FORTRAN(potri)(&UPLO, &N, A, &LDA, INFO );
    }
    
    
    void FORTRAN(pptrf)(char *, int*, real*, int*);
    inline void lapack_xpptrf(char UPLO, int N, real* A, int* INFO)
    {
        FORTRAN(pptrf)(&UPLO, &N, A, INFO );
    }
    
    
    void FORTRAN(pptrs)(char *, int*, int*, const real*, real*, int*, int*);
    inline void lapack_xpptrs(char UPLO, int N, int NHRS, const real* A, real* B, int LDB, int* INFO)
    {
        FORTRAN(pptrs)(&UPLO, &N, &NHRS, A, B, &LDB, INFO );
    }
    
    
    void FORTRAN(pptri)(char *, int*, real*, int*);
    inline void lapack_xpptri(char UPLO, int N, real* A, int* INFO)
    {
        FORTRAN(pptri)(&UPLO, &N, A, INFO );
    }
    
    
    void FORTRAN(trtrs)(char *, char *, char *, int*, int*, const real*, int*, real*, int*, int*);
    inline void lapack_xtrtrs(char UPLO, char trans, char diag, int N, int NHRS, const real* A, int LDA, real* B, int LDB, int* INFO)
    {
        FORTRAN(trtrs)(&UPLO, &trans, &diag, &N, &NHRS, A, &LDA, B, &LDB, INFO );
    }
    
    
    void FORTRAN(gesv)(int*, int*, real*, int*, int*, real*, int*, int*);
    inline void lapack_xgesv(int N, int NRHS, real* A, int LDA, int* IPIV, real* B, int LDB, int* INFO)
    {
        FORTRAN(gesv)(&N, &NRHS, A, &LDA, IPIV, B, &LDB, INFO);
    }
    
    
    void FORTRAN(getf2)(int*, int*, real*, int*, int*, int*);
    inline void lapack_xgetf2(int M, int N, real* A, int LDA, int* IPIV, int* INFO)
    {
        FORTRAN(getf2)(&M, &N, A, &LDA, IPIV, INFO);
    }
    
    
    void FORTRAN(getrf)(int*, int*, real*, int*, int*, int*);
    inline void lapack_xgetrf(int M, int N, real* A, int LDA, int* IPIV, int* INFO)
    {
        FORTRAN(getrf)(&M, &N, A, &LDA, IPIV, INFO);
    }
    
    
    void FORTRAN(getri)(int*, real*, int*, const int*, real*, const int*, int*);
    inline void lapack_xgetri(int N, real* A, int LDA, const int* IPIV, real* WORK, int LWORK, int* INFO)
    {
        FORTRAN(getri)(&N, A, &LDA, IPIV, WORK, &LWORK, INFO);
    }
    
    
    void FORTRAN(getrs)(char *, int*, int*, const real*, int*, const int*, real*, int*, int*);
    inline void lapack_xgetrs(char trans, int N, int NRHS, const real* A, int LDA, const int* IPIV, real* B, int LDB, int* INFO)
    {
        FORTRAN(getrs)(&trans, &N, &NRHS, A, &LDA, IPIV, B, &LDB, INFO);
    }
    
    
    void FORTRAN(laswp)(int*, const real*, int*, int*, int*, const int*, int*);
    inline void lapack_xlaswp(int N, const real* A, int LDA, int K1, int K2, const int* IPIV, int INCX)
    {
        FORTRAN(laswp)(&N, A, &LDA, &K1, &K2, IPIV, &INCX);
    }
    
    
    
    void FORTRAN(sysv)(char *, int*, int*, real*, int*, int*, real*, int*, real*, int*, int*);
    inline void lapack_xsysv(char UPLO, int N, int NRHS, real* A, int LDA, int* IPIV, real*B, int LDB, real* WORK, int LWORK, int* INFO)
    {
        FORTRAN(sysv)(&UPLO, &N, &NRHS, A, &LDA, IPIV, B, &LDB, WORK, &LWORK, INFO);
    }
    
    
    void FORTRAN(sytrf)(char *, int*, real*, int*, int*, real*, int*, int*);
    inline void lapack_xsytrf(char UPLO, int N, real* A, int LDA, int* IPIV, real* WORK, int LWORK, int* INFO)
    {
        FORTRAN(sytrf)(&UPLO, &N, A, &LDA, IPIV, WORK, &LWORK, INFO);
    }
    
    
    void FORTRAN(sytrs)(char *, int*, int*, real*, int*, int*, real*, int*, int*);
    inline void lapack_xsytrs( char UPLO, int N, int NRHS, real* A, int LDA, int* IPIV, real* B, int LDB, int* INFO)
    {
        FORTRAN(sytrs)(&UPLO, &N, &NRHS, A, &LDA, IPIV, B, &LDB, INFO);
    }
    
    
    
    
    
    void FORTRAN(syev)(char*, char*, int*, real*, int*, real*, real*, int*, int*);
    inline void lapack_xsyev(char JOBZ, char UPLO, int N, real* A, int LDA, real* W, real* WORK, int LWORK, int* INFO)
    {
        FORTRAN(syev)(&JOBZ, &UPLO, &N, A, &LDA, W, WORK, &LWORK, INFO);
    }
    
    
    void FORTRAN(syevd)(char*, char*, int*, real*, int*, real*, real*, int*, int*, int*, int*);
    inline void lapack_xsyevd(char JOBZ, char UPLO, int N, real* A, int LDA, real* W, real* WORK, int LWORK, int* IWORK, int LIWORK, int* INFO)
    {
        FORTRAN(syevd)(&JOBZ, &UPLO, &N, A, &LDA, W, WORK, &LWORK, IWORK, &LIWORK, INFO);
    }

    
    void FORTRAN(syevx)(char *, char *, char *, int*, real*, int*, real*, real*, int*, int*,
                        real*, int*, real*, real*, int*, real*, int*, const int*, int*, int*);
    inline void lapack_xsyevx(char JOBZ, char RANGE, char UPLO, int N, real* A, int LDA, real VL, real VU, int IL, int IU,
                              real ABSTOL, int* M, real* W, real* Z, int LDZ, real* WORK, int LWORK, const int* IWORK, int* IFAIL, int* INFO)
    {
        FORTRAN(syevx)(&JOBZ, &RANGE, &UPLO, &N, A, &LDA, &VL, &VU, &IL, &IU, &ABSTOL, M, W, Z, &LDZ, WORK, &LWORK, IWORK, IFAIL, INFO);
    }
    
    
    
    
    
    
    void FORTRAN(gtsv)(int*, int*, real*, real*, real*, real*, int*, int*);
    inline void lapack_xgtsv(int N, int NRHS, real* DL, real* D, real* DU, real* B, int LDB, int* INFO)
    {
        FORTRAN(gtsv)(&N, &NRHS, DL, D, DU, B, &LDB, INFO);
    }
    
    void FORTRAN(gttrf)(int*, real*, real*, real*, real*, int*, int*);
    inline void lapack_xgttrf(int N, real* DL, real* D, real* DU, real* DU2, int* IPIV, int* INFO)
    {
        FORTRAN(gttrf)(&N, DL, D, DU, DU2, IPIV, INFO);
    }
    
    
    void FORTRAN(gttrs)(char *, int*, int*, real*, real*, real*, real*, int*, real*, int*, int*);
    inline void lapack_xgttrs(char TRANS, int N, int NRHS, real* DL, real* D, real* DU, real* DU2, int* IPIV, real* B, int LDB, int* INFO)
    {
        FORTRAN(gttrs)(&TRANS, &N, &NRHS, DL, D, DU, DU2, IPIV, B, &LDB, INFO);
    }
    
    
    
    
    void FORTRAN(geqrf)( int*, int*, real*, int*, real*, real*, const int*, int*);
    inline void lapack_xgeqrf(int M, int N, real* A, int LDA,  real* TAU, real* WORK, int LWORK, int* INFO)
    {
        FORTRAN(geqrf)(&M, &N, A, &LDA, TAU, WORK, &LWORK, INFO);
    }
    
    void FORTRAN(ormqr)( char *, char *, int*, int*, int*, const real*, int*, const real*, real*, int*, real*, const int*, int*);
    inline void lapack_xormqr( char side, char trans, int M, int N, int K, const real* A, int LDA, const real* TAU,
                              real* C, int LDC, real* WORK, int LWORK, int* INFO)
    {
        FORTRAN(ormqr)(&side, &trans, &M, &N, &K, A, &LDA, TAU, C, &LDC, WORK, &LWORK, INFO);
    }
    
    void FORTRAN(gels)(char *, int*, int*, int*, const real*, int*, const real*, int*, real*, const int*, int*);
    inline void lapack_xgels(char trans, int M, int N, int NRHS, const real* A, int LDA, const real* B, int LDB, real* WORK, int LWORK, int* INFO)
    {
        FORTRAN(gels)(&trans, &M, &N, &NRHS, A, &LDA, B, &LDB, WORK, &LWORK, INFO);
    }
    
#ifdef __cplusplus
} //extern "C"
#endif

#endif //LAPACK_H
