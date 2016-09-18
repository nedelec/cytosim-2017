// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <stdio.h>
#include <sys/types.h>

#include "array.h"
#include "random.h"
extern Random RNG;


int comp_ints(const void * ap, const void * bp)
{
    int a = *((int*)ap);
    int b = *((int*)bp);
    if ( a < b ) return -1;
    if ( b > a ) return 1;
    return 0;
}


int main(int argc, char* argv[])
{
    RNG.seedTimer();
    Array<int> a;
    
    for( int cnt = 0; cnt < 10; ++cnt )
    {
        a.clear();
        for( int i=0; i < 10; ++i )
            if ( RNG.flip() )
                a.push_back(RNG.flip());
        
        printf("size = %i", a.size());
        {
            Array<int> b;
            b = a;
            a.deallocate();
            
            printf(" = %i", b.size());
            b.sort(comp_ints);
            
            printf(" -> %i\n", b.size());
            for( unsigned int i=0; i < b.size(); ++i )
                assert_true( b[i] == 1 );
        }
    }
    
    printf("test done\n");
    return 0;
}
