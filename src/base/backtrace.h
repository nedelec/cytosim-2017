// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
/*
 *
 * backtrace is useful to identify the cause of a crash,
 * by printing the current call-stack using functions from the GNU C Library:
 * - backtrace()
 * - backtrace_symbols()
 * .
 * On Mac, this is available since OSX 10.5
 *
 * Backtrace is however not needed to run Cytosim, and this can be disabled
 */



#include <cstdio>


/// print the stack of function calls for the current thread
void print_backtrace(FILE * file);
