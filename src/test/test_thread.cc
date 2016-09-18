// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <cstdlib>
#include <cstdio>
#include <pthread.h>

/*
 An example where the execution of a thread 'slave',
 is controlled from the current thread (through keyboard input)
 */

pthread_mutex_t mutex;
pthread_cond_t condition;



void* loop(void *arg)
{
    pthread_mutex_lock(&mutex);
    pthread_t self = pthread_self();
    for ( int cnt = 1; cnt <= 10; ++cnt )
    {
        printf("\n-thread %p: step %i\n", self, cnt);
        pthread_cond_wait(&condition, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    return (void*)self;
}


int main(int argc, char *argv[]) 
{    
    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&condition, 0);
    
    pthread_t slave;
    pthread_create(&slave, 0, &loop, 0);
    
    char key[32];
    do {
        printf(">");
        fgets(key, sizeof(key), stdin);
        pthread_cond_signal(&condition);
    } while( key[0] != 'q' );
    
    if ( 1 )
    {
        pthread_cancel(slave);
        pthread_join(slave, 0);
    }
    
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&mutex);
    printf("finished\n");
    return 0;
}
