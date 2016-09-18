// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <cstdio>
#include "real.h"
#include "random.h"
#include "random_vector.h"
#include "quaternion.h"
#include "matrix3.h"
#include "vecprint.h"

extern Random RNG;


void testRotation(Vector3 vec, real angle)
{
    Matrix3 mat;
    Quaternion<real> q, p;
    Vector3 V, W, T;

    q.setFromAxis(vec, angle);
    real a = q.getAngle(T);
    
    printf("\nQ = ");
    q.print();
    printf(" ,   norm = %.2f\n", q.norm() );
    
    printf(" %.2f %.2f %.2f  angle %.3f", vec[0],vec[1],vec[2], angle);
    printf(" ?=? %.2f %.2f %.2f  angle %.3f\n", T[0],T[1],T[2], a);
    
    q.setMatrix3(mat);
    mat.write(std::cout);
    printf("rotation error = %e", mat.maxDeviationFromRotation());
    
    real err = ( Matrix3::rotationAroundAxis( vec, angle ) - mat ).maxNorm();
    printf(", deviation = %e\n", err);
    
    if ( 0 )
    {
        real m16[16];
        q.setOpenGLMatrix(m16);
        VecPrint::matPrint(std::cout, 4, 4, m16);
    }
    
    V = Vector3::randBox();
    W = mat*V;

    printf("   MATRIX*V            : ");
    W.println();
    
    p = q * Quaternion<real>(0, V.XX, V.YY, V.ZZ) * q.conjugated();
    printf("   q * (0, V) * inv(q) : ");
    p.println();
    
    q.rotateVector(W,V);
    printf("   Q.rotateVector(V)   : ");
    W.println();
}


void test1()
{
    Matrix3 mat;
    Vector3 V, W;
    
    Quaternion<real> q, p;
    
    const real angle = M_PI/6.0;
    Vector3 vec(0,0,0);
    
    printf("------------------- rotations of PI/6 -----------------\n");
    
    for ( int ii = 0; ii<3; ++ii )
    {
        vec.set(0,0,0);
        vec[ii] = 1;
        testRotation(vec, angle);
    }
    
    printf("------------------- identity ---------------------------\n");
    
    mat.makeIdentity();
    mat.write(std::cout);
    q.setFromMatrix3(mat);
    q.println(stdout);
    
    printf("-------------- quat-quat multiplication ----------------\n");
    
    for ( int ii = 0; ii < 4; ++ii )
    {
        for ( int jj = 0; jj < 4; ++jj )
        {
            p = Quaternion<real>(0,0,0,0);
            p[ii] = 1;
            q = Quaternion<real>(0,0,0,0);
            q[jj] = 1;
            
            p.print();
            printf("  * ");
            q.print();
            printf("  = ");
            (p*q).println();
        }
        printf("\n");
    }
    
    printf("----------------- conversion quat-mat-quat --------------\n");
    
    real error = 0, e;
    for ( int ii = 0; ii < 1000; ++ii )
    {
        vec = Vector3::randUnit();
        real a = RNG.sreal() * M_PI;
        p.setFromAxis(vec, a);
        p.setMatrix3(mat);
        q.setFromMatrix3(mat);
        //p.print(); q.print();
        if ( q[0] * p[0] < 0 ) q = -q;
        e = (q-p).norm();
        if ( e > error ) error = e;
    }
    printf("  max error = %e\n", error);
    
    printf("------------ rotation mult. is not commutative -----------\n");
    
    for ( int ii = 0; ii<3; ++ii )
    {
        for ( int jj = 0; jj<3; ++jj )
        {
            q.setFromPrincipalAxis( ii, angle );
            p.setFromPrincipalAxis( jj, angle );
            
            (q*p).print(stdout);
            (p*q).println(stdout);
        }
    }
            
    printf("------------ rotation around principal axes -------------\n");

    for ( int ii = 0; ii<3; ++ii )
    {
        q.setFromPrincipalAxis(ii, angle);
        q.setMatrix3(mat);
        mat.write(std::cout);
        printf("\n");
    }
}

void test2(const int max)
{
    //this test a way to generate a random matrix:
    Quaternion<real> Q;
    Quaternion<real> pos;
    Matrix3 rot;
    Vector3 vec;
    
    for(int s = 0; s < max; ++s)
    {
        Q = Quaternion<real>::randomRotation(RNG);
        pos = Q * Quaternion<real>(0,1,0,0) * Q.conjugated();
        rot = Matrix3::randomRotation(RNG);
        vec = rot * Vector3(0,0,1);
        vec.println();
    }
}


int main(int argc, char* argv[])
{
    test1();
    //test2(10000);
    return 0;
}
