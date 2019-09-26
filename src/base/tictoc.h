// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#include <ctime>

/// A set of functions related to time
/**
 Functions to get wall-time, and processor-time,
 from the C-standard library.
 */
namespace TicToc
{    
    
    /// current date in short format, `buf` should be 26 character long or more
    void    date(char * buf, size_t buf_size);
    
    /// current date in short format, `buf` should be 26 character long or more
    void    date(char * buf, size_t buf_size, bool no_year);

    /// this is approximately the number of days after Jan 1 2000
    int     date_hack(const char *);
    
    /// day of the year (0-365)
    int     day_of_the_year();
    
    /// hour of the day (0-23)
    int     hours_today();

    /// number of second since midnight
    long    seconds_today();
    
    /// number of micro-second since midnight
    long    milli_seconds_today();
    
    /// call to start timer
    void    tic();
    
    /// return number of milli-seconds elapsed since last call to `tic()`
    double  toc();

    /// call to stop timing. Will print a message if msg!=0 
    double  toc(const char * msg);
    
};

