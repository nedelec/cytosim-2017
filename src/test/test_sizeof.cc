// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <stdio.h>
#include <sys/types.h>

#include <iostream>
#include <iomanip>
#include <typeinfo>
#include <cmath>

#define KNRM  "\x1B[0m"
#define KBLD  "\x1B[1m"
#define KUND  "\x1B[4m"
#define KREV  "\x1B[7m"

#define KBLK  "\x1B[30m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define KBLDRED  "\x1B[1m\x1B[31m"
#define KBLDGRN  "\x1B[1m\x1B[32m"
#define KBLDYEL  "\x1B[1m\x1B[33m"
#define KBLDBLU  "\x1B[1m\x1B[34m"
#define KBLDMAG  "\x1B[1m\x1B[35m"
#define KBLDCYN  "\x1B[1m\x1B[36m"
#define KBLDWHT  "\x1B[1m\x1B[37m"

template <typename T>
void print_sizeof(char const* msg)
{
    std::cout << std::setw(16) << msg << " : " <<
    typeid(T).name() << "   sizeof = " <<
    sizeof(T) << std::endl;
}

void print_types()
{
    typeof(int) x = 1;
    
    if ( x == 1 )
        printf("typeof exists\n");
    
    print_sizeof<bool>("bool");
    print_sizeof<char>("char");
    print_sizeof<short>("short");
    print_sizeof<int>("int");
    print_sizeof<long>("long int");
    print_sizeof<long long int>("long long int");
    
    print_sizeof<float>("float");
    print_sizeof<double>("double");
    print_sizeof<long double>("long double");
}

void print_sizes()
{
    printf("Size of Data Types:\n");

    printf( "bool          = %lu bytes\n", sizeof(bool) );
    printf( "char          = %lu bytes\n", sizeof(char) );
    printf( "short         = %lu bytes\n", sizeof(short) );
    printf( "int           = %lu bytes\n", sizeof(int) );
    printf( "long int      = %lu bytes\n", sizeof(long int) );
    printf( "long long int = %lu bytes\n", sizeof(long long int) );
    printf( "float         = %lu bytes\n", sizeof(float) );
    printf( "double        = %lu bytes\n", sizeof(double) );
    printf( "long double   = %lu bytes\n", sizeof(long double) );
    printf( "void *        = %lu bytes\n", sizeof(void *) );
    printf( "fpos_t        = %lu bytes\n", sizeof(fpos_t) );
    printf( "off_t         = %lu bytes\n", sizeof(off_t) );
    
}

void print_colors()
{
    printf("%sred\n", KRED);
    printf("%sgreen\n", KGRN);
    printf("%syellow\n", KYEL);
    printf("%sblue\n", KBLU);
    printf("%smagenta\n", KMAG);
    printf("%scyan\n", KCYN);
    printf("%swhite\n", KWHT);
    printf("%snormal\n", KNRM);
}


int main ()
{
    print_types();
    printf("\n");
    print_sizes();
    printf("\n");
    print_colors();
    return 0;
}
