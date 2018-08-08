// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
/*
 A test for the Floating Point Exceptions (Signal)
*/

#include <cstdlib>
#include <iostream>
#include <csignal>
#include "smath.h"

/*
 icpc --help
 
 -fp-trap=<arg>[,<arg>,...]
 control floating point traps at program start.  <arg> can be of the
 following values
 [no]divzero   - [Do not] trap on division by zero
 [no]inexact   - [Do not] trap on inexact result
 [no]invalid   - [Do not] trap on invalid operation
 [no]overflow  - [Do not] trap on overflow
 [no]underflow - [Do not] trap on underflow
 [no]denormal  - [Do not] trap on denormal
 all           - enable trap on all of the above
 none          - trap on none of the above
 common        - trap on most commonly used IEEE traps
 (invalid, division by zero, overflow)
 -fp-trap-all=<arg>[,<arg>,...]
 control floating point traps in every routine.  <arg> can be of the
 values specified in -fp-trap
 */

typedef double real;

void fpe_handler(int sig)
{
    psignal(sig, "Cytosim Error ");
    std::cerr << "Cytosim : Floating point exception" << std::endl;
    exit(sig);
}

void infinities()
{
    real z = 0;
    real y = 0.0 / z;
    real x = 1.0 / z;
    std::cerr << " 1/0 = " << x << std::endl; 
    std::cerr << " 0/0 = " << y << std::endl;
    std::cerr << "0   < inf = " << ( 0 < INFINITY ) << std::endl;
    std::cerr << "inf < inf = " << ( INFINITY < INFINITY ) << std::endl;
}

void print_numbers()
{
    std::cerr << " 1.0 / 0 = " <<  1.0 / 0 << std::endl;
    std::cerr << "-1.0 / 0 = " << -1.0 / 0 << std::endl;
    std::cerr << " 0.0 / 0 = " <<  0.0 / 0 << std::endl;
    std::cerr << "-log(0)  = " << -log(0.0) << std::endl;
#if ( 0 )
    std::cerr << "absf(-2) = " << sMath::absf(-2.0) << std::endl;
    std::cerr << "absf(-1) = " << sMath::absf(-1.) << std::endl;
    std::cerr << "absf(+1) = " << sMath::absf(+1.) << std::endl;
    std::cerr << "absf(+2) = " << sMath::absf(+2.) << std::endl;
#endif
}


int main ()
{
    print_numbers();
    if ( signal(SIGFPE, fpe_handler) == SIG_ERR )
    {
        std::cerr << "Could not register SIGFPE handler\n";
        return EXIT_FAILURE;
    }
    infinities();
    std::cerr << "test completed" << std::endl;
    return 0;
}
