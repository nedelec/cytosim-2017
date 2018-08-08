// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "random.h"
#include <cstring>
#include "tictoc.h"

extern Random RNG;




void printBits(FILE * f, const void * v, const int size)
{
    for ( int ii=0; ii < size; ++ii )
    {
        char c = ((char*)v)[ii];
        for ( int jj=7; jj >=0; --jj )
            fprintf(f, "%d", ( c >> jj ) & 1 );
        fprintf(f, ".");
    }
    fprintf(f, "\n");
}



void speed_test()
{
    const unsigned int cnt = 1 << 30;
    TicToc::tic();
    uint32_t u = 10;
    for (uint32_t j=0; j<cnt; ++j)
    {
        u = RNG.pint_inc(1024);
        RNG.pint_inc(u);
    }
    TicToc::toc("int");
}


void test_int()
{
    int j;
    
    for (j=0; j<21; j++)
        printf(" %10u%s", RNG.pint(), (j%7)==6 ? "\n" : "");
    
    printf("\n");
    
    for (j=0; j<90; j++)
        printf(" %2u%s", RNG.pint_inc(99), (j%30)==29 ? "\n" : "");
    
    printf("\n");
    
    for (j=0; j<90; j++)
        printf(" %2u%s", RNG.pint_inc2(99), (j%30)==29 ? "\n" : "");
    
    printf("\n");
    
    for (j=0; j<100; j++)
        printf(" %3i%s", RNG.sint_inc(99), (j%20)==19 ? "\n" : "");
    
    printf("\n");
    
    for (j=0; j<42; j++)
        printf(" %10.7f%s", RNG.sreal(), (j%7)==6 ? "\n" : "");
    
    printf("\n");
    
    for (j=0; j<42; j++)
        printf(" %8f%s", RNG.preal(), (j%7)==6 ? "\n" : "");
    
    printf("\n");
}


float convertFix(uint32_t x)
{
    //This assumes IEEE Standard 754 Floating point numbers
    //32 bits: 1 for sign, 8 for exponents, 23 for fraction
    const uint32_t FRAC     = 0x7FFFFFU;
    const uint32_t EXPON    = 127 << 23;
    uint32_t result = EXPON | ( x & FRAC );
    return *(float*)&result - 1.0;
}



void testbits()
{
    const int SCALE=2;
    float x;
    for ( int ii=0; ii <= SCALE; ++ii )
    {
        x = ii / float(SCALE);
        printf(" %f :", x);
        printBits(stdout, &x, 4);
        // x = -ii / float(SCALE);
        // printf("%f :", x);
        // printBits(stdout, &x, 4);
    }
    
    double y;
    for ( int ii=0; ii <= 20; ++ii )
    {
        y = convertFix( RNG.pint() );
        printf(" %f :", y);
        printBits(stdout, &y,8);
    }
}


#define TEST test
void test_test( const real prob, const int MAX )
{
    int cnt = 0, a, b, c;
    for ( int jj=0; jj < MAX; ++jj )
    {
        a = RNG.TEST(prob) + RNG.TEST(prob) + RNG.TEST(prob) + RNG.TEST(prob);
        b = RNG.TEST(prob) + RNG.TEST(prob) + RNG.TEST(prob) + RNG.TEST(prob);
        c = RNG.TEST(prob) + RNG.TEST(prob) + RNG.TEST(prob) + RNG.TEST(prob);
        cnt += a + b + c;
    }
    printf("prob = %f measured = %f cnt = %i\n", prob, cnt / double(12*MAX), cnt);
}

void test_RNG(const int MAX)
{
    for ( int jj=0; jj < MAX; ++jj )
    {
        RNG.preal();RNG.preal();RNG.preal();RNG.preal();RNG.preal();
        RNG.preal();RNG.preal();RNG.preal();RNG.preal();RNG.preal();
    }
}


void test_float()
{
    double d;
    printf("preal:      ");
    for ( int kk=0; kk < 10; ++kk )
    {
        d = RNG.preal();
        printf(" %+f", d);
    }
    printf("\n");
    printf("sreal:      ");
    for ( int kk=0; kk < 10; ++kk )
    {
        d = RNG.sreal();
        printf(" %+f", d);
    }
    printf("\n");
}

//==========================================================================

void testPoisson(real E, unsigned int N)
{
    real x = 0;
    for ( unsigned int i = 0; i < N; ++i )
        x += RNG.poisson(E);
    x /= N;
    printf("%f - %f = %f\n", x, E, x-E);
}


//==========================================================================
//test 3 methods to generate a random event time, when the rate varies in time
// F. Nedelec, Oct 2005

//this is our standard method: 64s CPU
int method1(const int maxTime, const real rate[])
{
    for ( int ii=0; ii<maxTime; ++ii )
    {
        if (RNG.test(rate[ii])) return ii;
    }
    return maxTime;
}

//this is 'exact' and very slow: 370s CPU (an exponential at each step!)
int method2(const int maxTime, const real rate[])
{
    for ( int ii=0; ii<maxTime; ++ii )
    {
        if ( RNG.preal() < (1.-exp(-rate[ii])) )
            return ii;
    }
    return maxTime;
}

//this is exact, and the fastest method: 10s CPU!
int method3(const int maxTime, const real rate[])
{
    real T = -log( RNG.preal() );
    for ( int ii=0; ii<maxTime; ++ii )
    {
        T -= rate[ii];
        if ( T < 0 ) return ii;
    }
    return maxTime;
}


int testGillespie(const int method)
{
    //test new idea for gillespie with changing rate (Oct 2005)
    const int maxTime = 200;
    real rate[maxTime];
    for ( int ii=0; ii<maxTime; ++ii )
        rate[ii] = ( ii % 10 ) / 30.0;
    
    int bins[3][maxTime+1];
    for ( int ii=0; ii<=maxTime; ++ii )
    {
        bins[0][ii] = 0;
        bins[1][ii] = 0;
        bins[2][ii] = 0;
    }
    
    const int nbSamples = 1000000;
    const int subSamples = 10;
    int result;
    switch( method )
    {
        case 0:
            for ( int ii=0; ii<nbSamples; ++ii )
            {
                bins[0][ method1(maxTime, rate) ]++;
                bins[1][ method2(maxTime, rate) ]++;
                bins[2][ method3(maxTime, rate) ]++;
            }
            break;
            
        case 1:
            printf("method 1:");
            for ( int ii=0; ii<nbSamples; ++ii )
                for ( int jj=0; jj<subSamples; ++jj )
                    result = method1(maxTime, rate);
            return result;
            
        case 2:
            printf("method 2:");
            for ( int ii=0; ii<nbSamples; ++ii )
                for ( int jj=0; jj<subSamples; ++jj )
                    result = method2(maxTime, rate);
            return result;
            
        case 3:
            printf("method 3:");
            for ( int ii=0; ii<nbSamples; ++ii )
                for ( int jj=0; jj<subSamples; ++jj )
                    result = method3(maxTime, rate);
            return result;
    }
    
    
    FILE* file = fopen("test.out", "w");
    for ( int ii=0; ii<=maxTime; ++ii )
        fprintf(file, "%4i   %6i %6i %6i\n", ii, bins[0][ii], bins[1][ii], bins[2][ii]);
    fclose(file);
    return 0;
}


//==========================================================================
int main(int argc, char* argv[])
{
    
    
#if ( 0 )  // 1 = test modified Gillespie method
    if ( argc == 1 )
        testGillespie(0);
    else
        testGillespie(atoi(argv[1]));
    
    printf(" done Gillespie with variable rate\n");
    return EXIT_SUCCESS;
#endif
    
#if ( 0 )
    for ( int i = 0; i < 20; ++i )
        testPoisson(RNG.preal_exc(), 1000000);
    return EXIT_SUCCESS;
#endif
    
    printf("sizeof(uint32_t) = %lu\n", sizeof(uint32_t));
    if ( argc == 1 )
    {
        for ( int kk=0; kk < 11; ++kk )
        {
            real rate = kk/10.0;
            test_test(rate, 5000000);
            //test_RNG(50000);
        }
    }
    else
    {
        //seed with the given number (hopefully a number!)
        RNG.seed( atoi(argv[1]) );
        test_int();
        speed_test();
        test_float();
    }
    
    
    printf("done\n");
    return EXIT_SUCCESS;
}

