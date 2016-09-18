// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <stdio.h>

#include "real.h"
#include "cblas.h"
#include "clapack.h"


void test_blas(const int size)
{
    real * x = new real[size];
    real * y = new real[size];
    real * z = new real[size];
    
    for ( int i = 0; i < size; ++i )
        x[i] = i;
    
    blas_xzero(size, y);
    blas_xcopy(size, x, 1, y, 1);
    blas_xcopy(size, x, 1, z, 1);
    blas_xscal(size, +3.14, z, 1);
    blas_xaxpy(size, -3.14, x, 1, z, 1);
    
    real sum = blas_xasum(size, z, 1);
    printf("zero = %f\n", sum);
    
    real nrm = blas_xdot(size, x, 1, y, 1);
    printf("nrm^2 = %f\n", nrm);
    
    nrm = blas_xnrm2(size, x, 1);
    printf("nrm^2 = %f\n", nrm*nrm);
    
    delete[] z;
    delete[] y;
    delete[] x;
}


void test_lapack(const int size)
{
    real* mat = new real[size*size];
    int* ipiv = new int[size];
    
    for ( int ii = 0; ii < size; ++ii )
    {
        for ( int jj = 0; jj < size; ++jj )
            mat[ii+size*jj] = (ii+1) * (ii==jj);
    }
    
    int info = 0;
    real w;
    lapack_xgetri( size, 0, size, ipiv, &w, -1, &info );
    int workspace = (int)w;
    printf("getri workspace %i\n", workspace);
    real* work  = new real[workspace];


    lapack_xgetrf( size, size, mat, size, ipiv, &info );
    printf("getrf returned %i\n", info);
    
    lapack_xgetri( size, mat, size, ipiv, work, workspace, &info );
    printf("getri returned %i\n", info);
    
    for ( int ii = 0; ii < size; ++ii )
    {
        for ( int jj = 0; jj < size; ++jj )
            printf("%f ", mat[ii+size*jj]);
        printf("\n");
    }
    
    delete[] work;
    delete[] ipiv;
    delete[] mat;
}


int main(int argc, char* argv[])
{
    printf("\nBLAS:\n");
    test_blas(10);
    
    printf("\nLAPACK:\n");
    test_lapack(10);

    printf("\ndone!\n");
    return 0;
}
