// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/*
 frametool is an utility to read and manipulate frames in object-files,
 
 It only uses the START and END tags of frames, and does not interpret
 or verify the logical content of the data contained between these tags.

 sieve is another tool to read/write object-files, 
 reader is an interactive tool to read/write object-files
*/

#include <cstdio>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include "iowrapper.h"

enum { SKIP, COPY, LAST };

/**
 read a line, and returns a code indicating if this is the start
 or the end of a cytosim frame
 */
int whatline(FILE* file, bool copy, char * buf, size_t buf_size)
{
    char *const end = buf + buf_size;
    char * ptr = buf;
    
    flockfile(file);

    char c = 0;
    do {
        c = getc_unlocked(file);
        
        if ( feof(file) )
            break;

        if ( ptr < end )
            *ptr++ = c;
        
        if ( copy )
            putchar(c);
        
    } while ( c != '\n' );
 
    funlockfile(file);
    
    while ( ptr < end )
        *ptr++ = 0;
    
    if ( 0 == strncmp(buf, "#frm ", 5) )     return 1;
    if ( 0 == strncmp(buf, "#frame ", 7) )   return 1;
    if ( 0 == strncmp(buf, "#Cytosim ", 9) ) return 1;
    if ( 0 == strncmp(buf, "#end ", 5) )     return 2;
    if ( 0 == strncmp(buf, " #end ", 6) )    return 2;

    return 0;
}


int whatline(FILE* file, bool copy)
{
    char buf[16];
    int res = whatline(file, copy, buf, sizeof(buf));
    //if (res) fprintf(stderr, ">%s<\n", buf);
    return res;
}


//=============================================================================


void countFrame(FILE* file)
{
    int  frm = -1, linecnt = 0, oldcnt = 0;
    
    do {
        ++linecnt;
        int code = whatline(file, false);
        
        if ( code == 2 )
        {
            printf("frame %5i: %7i lines (%+i)\n", frm, linecnt, linecnt-oldcnt);
            oldcnt = linecnt;
        }
        
        if ( code == 1 )
        {
            ++frm;
            linecnt = 0;
        }
        
    } while ( !feof(file) );
}


void extract(FILE* file, const int action[], int max)
{
    int indx = -1;
    bool copy = ( action[0] != SKIP );
    
    while ( !feof(file) )
    {
        int code = whatline(file, copy);
        
        if ( code == 0 )
            continue;
        
        if ( code == 1 )
        {
            ++indx;
            if ( indx < max )
                copy = ( action[indx] != SKIP );
            else
                return;
        }
        if ( code == 2 )
        {
            if ( indx+2 >= max )
                return;
            if ( LAST == action[indx+1] )
                return;
            copy = ( action[indx+1] != SKIP );
        }
    }
}


void extractLast(FILE* file)
{
    fpos_t pos, start;
    
     while ( !feof(file) )
     {
         fgetpos(file, &pos);
         int code = whatline(file, false);
         
         if ( code == 1 )
             start = pos;
    }
    
    clearerr(file);
    fsetpos(file, &start);
    
    while ( 1 )
    {
        char c = getc(file);
        if ( feof(file) )
            break;
        putchar(c);
    }
    
    printf("\n");
}


//=============================================================================

void help()
{
    printf("Synopsis:\n");
    printf("    frametool can list the frames present in a trajectory file,\n");
    printf("    or extract specified frames\n");
    printf("Usage:\n");
    printf("    frametool FILENAME [INDICES]\n\n");
    printf("INDICES can be specified with positive integers as:\n");
    printf("    INDEX\n");
    printf("    START:END\n");
    printf("    START:\n");
    printf("    START:INCREMENT:END\n");
    printf("    START:INCREMENT:\n");
    printf("    last\n");
    printf("Examples:\n");
    printf("    frametool objects.cmo 0:2:\n");
    printf("    frametool objects.cmo 0:10\n");
    printf("    frametool objects.cmo last\n");
}



void HALT(const char* message)
{
    fprintf(stderr, "ERROR: %s\n", message);
    exit(EXIT_FAILURE);
}


int parse(char arg[], int& mode, int action[], int max)
{
    if ( 0 == strncmp(arg, "last", 4) )
    {
        mode = LAST;
        return 0;
    }
    
    if ( isdigit(arg[0]) )
    {
        int s = 0, i = 1, e = max-1;
        int c = sscanf(arg, "%i:%i:%i", &s, &i, &e);
        bool open = ( arg[strlen(arg)-1] == ':' );
        switch( c )
        {
            case 0:
                return 1;
            case 1:
                if ( s < 0 )
                    HALT("frame number must be positive");
                if ( open )
                {
                    for ( int f = s; f < max; ++f )
                        action[f] = COPY;
                }
                else
                {
                    if ( s < max )
                        action[s] = COPY;
                }
                break;
            case 2:
                if ( s < 0 )
                    HALT("frame number must be positive");
                if ( open )
                {
                    if ( i < 0 )
                        HALT("increment must be positive");
                    for ( int f = s; f < max; f += i )
                        action[f] = COPY;
                }
                else
                {
                    if ( i < 0 )
                        HALT("frame number must be positive");
                    for ( int f = s; f <= i  &&  f < max; ++f )
                        action[f] = COPY;
                }
                break;
            case 3:
                if ( s < 0 )
                    HALT("frame number must be positive");
                if ( i <= 0 )
                    HALT("increment must be strictly positive");
                if ( e < 0 )
                    HALT("frame number must be positive");
                if ( open )
                    HALT("unexpected syntax");
                for ( int f = s; f <= e  &&  f < max; f+= i )
                    action[f] = COPY;
                break;
        }
        mode = COPY;
        return 0;
    }
    return 1;
}


int main(int argc, char* argv[])
{
    const char* filename = "objects.cmo";
    FILE* file;
    
    //a list of frame to collect:
    const unsigned int MAX_FRAME = 16384;
    int action[MAX_FRAME] = { SKIP };
    
    if ( argc < 2 )
    {
        help();
        return EXIT_FAILURE;
    }
    
    if ( 0 == strncmp(argv[1], "help", 4) )
    {
        help();
        printf("! This program has a limit of %i frames\n", MAX_FRAME);
        return EXIT_SUCCESS;
    }
    
    filename = argv[1];
    int mode = SKIP;
    
    //get the list of frame from the command line arguments:
    for ( int ar = 2; ar < argc; ++ar )
    {
        if ( parse(argv[ar], mode, action, MAX_FRAME) )
        {
            printf("Unexpected command line argument `%s'\n", argv[ar]);
            return EXIT_FAILURE;
        }
    }
    
    for ( int f = MAX_FRAME-1; f >= 0 && action[f] == SKIP; --f )
        action[f] = LAST;

#if ( 0 )
    for ( int f = 0; f < 10 ; ++f )
        fprintf(stderr, "%i", action[f]);
    fprintf(stderr, "\n");
#endif
    
    //----------------------------------------------
    
    file=fopen(filename, "r");
    if ( file==0 )
    {
        printf("Could not open file `%s'\n", filename);
        return EXIT_FAILURE;
    }
    if ( ferror(file) )
    {
        fclose(file);
        printf("Error opening file `%s'\n", filename);
        return EXIT_FAILURE;
    }

    //----------------------------------------------
    switch(mode)
    {
        case SKIP:
            countFrame(file);
            break;
            
        case COPY:
            extract(file, action, MAX_FRAME);
            break;
            
        case LAST:
            extractLast(file);
            break;
    }
    
    fclose(file);
    return EXIT_SUCCESS;
}
