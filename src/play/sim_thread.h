// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef LIVE_THREAD_H
#define LIVE_THREAD_H

#include <pthread.h>
#include "parser.h"
#include "simul.h"
#include "frame_reader.h"


/// SimThread is used to run a simulation in a dedicated thread
class SimThread : private Parser
{
    
public:
    
    /**
     A Lock should be created to access the simulation state if it is running live.
     The constructor will stop the simulation at the next opportunity.
     
     The thread will hold indefinitely until the lock is acquired.
     
     The lock will be released when the Lock goes out of scope.
     */
    class Lock
    {
        SimThread * mlt;
        
    public:
        
        /// Stop the simulation, create a lock and call Simul::relax()
        Lock(SimThread * p) : mlt(p) { p->lock(); }
        
        /// Release the pthread lock
        ~Lock() { mlt->unlock(); }
    };
    
    
    /**
     A TryLock can be created to access the simulation state if it is running live.
     The constructor will try to lock the simulation, but this may fail.
     
     TryLock::status() will return 0 if the lock was acquired.
     
     Any acquired lock will be released when the TryLock goes out of scope.
     */
    class TryLock
    {
        SimThread * mlt;
        int          ecode;
        
    public:
        
        /// Stop the simulation, create a pthread lock
        TryLock(SimThread * p) : mlt(p) { ecode = p->trylock(); }
        
        /// return 0 if the lock was acquired, and pthread_mutex_trylock error code otherwise
        int status() const { return ecode; }
        
        /// Release the pthread lock
        ~TryLock() { if ( ecode == 0 ) mlt->unlock(); }
    };

private:
    
    /// Simulation object
    Simul           simul;

    /// reader used to access frames in trajectory file
    FrameReader     reader;

    
    /// disabled default constructor
    SimThread();

    /// callback invoked when the thread is halted
    void           (*holding)(void);
    
    /// a flag reflecting if mThread is running or not
    int             mStatus;
    
    /// slave thread
    pthread_t       mThread;
    
    /// mutex to access simulation state
    pthread_mutex_t mMutex;

    /// condition variable used to control the thread execution
    pthread_cond_t  mCondition;
    
    /// counter for hold()
    unsigned int    mHold;
    
    /// period for hold()
    unsigned int    mPeriod;

    
    /// the current Single being controlled with the mouse
    Single *        mHandle;
    
    /// list of Single used to grab microtubules with the mouse
    Array<Single *> mHandles;
    
    
    /// create or make a new SingleProp for the handles
    SingleProp *  getHandleProperty(real range);

    
    /// lock Mutex guarding access to the Simulation data
    void          lock()   { pthread_mutex_lock(&mMutex); }
    
    /// unlock Mutex guarding access to the Simulation data
    void          unlock() { pthread_mutex_unlock(&mMutex); }
    
    /// trylock Mutex guarding access to the Simulation data
    int           trylock() { return pthread_mutex_trylock(&mMutex); }

    
    /// redefines Interface::hold(), will be called between commands
    void          hold();
    
    /// run the simulation
    void          run();
    
    /// continue to run the simulation beyond its normal termination
    void          run_more();
    
    /// accessory function to call SimThread::run()
    friend void*  run_thread(void * arg);
    
    /// accessory function to call SimThread::persist()
    friend void*  run_more_thread(void * arg);

public:
    
    /// create a SimThread with given Parser permissions
    SimThread(void (*hold_callback)(void));
    
    /// destructor
    ~SimThread();
    
    /// Simul reference
    Simul&     sim() { return simul; }
    
    /// set how many 'hold()' are necessary to halt the thread
    void       period(unsigned int c) { mPeriod = c; }
    
    /// allow the holding-thread to continue
    void       release() { pthread_cond_signal(&mCondition); }

    
    /// start the thread to run a simulation
    int        start();
    
    /// continue to run the simulation after its normal termination
    int        persist();
    
    /// gently stop the simulation
    void       stop();

    /// stop the simulation
    void       cancel();

    /// clear the simulation world
    void       clear();
    
    /// halt the live simulation, read the config file and change the object parameters
    void       reloadConfig();
    
    /// execute given code
    int        execute(std::istream&);
    
    /// save state to file
    void       writeState();
    
    /// save properties to file
    void       writeProperties(std::ostream&, bool prune);

    
    /// open file for input
    void       openFile()       { reader.openFile(simul.prop->trajectory_file); }

    /// true if ready to read from file
    bool       goodFile()       { return reader.good(); }
    
    /// index of current frame
    int        eof()            { return reader.eof(); }
    
    /// index of current frame
    int        frame()          { return reader.frame(); }
    
    /// attempt to load specified frame
    int        readFrame(int f) { Lock lck(this); return reader.readFrame(simul, f); }

    /// load next frame in file
    int        nextFrame()      { Lock lck(this); return reader.readNextFrame(simul); }

    
    ///
    Vector     extension()      { Lock lck(this); if ( simul.space() ) return simul.space()->extension(); return Vector(0,0,0); }
    
    /// return current handle
    Single *   handle()   const { return mHandle; }

    /// make a new Single that can be controlled by the user
    Single *   createHandle(const Vector&, real range);
    
    /// swith current handle
    bool       selectClosestHandle(const Vector&, real range);
    
    /// detach current handle
    void       detachHandle();
    
    /// move the current handle
    void       moveHandle(const Vector&);
    
    /// move all handles
    void       moveHandles(const Vector&);
    
    /// delete all handles
    void       deleteHandles();
    
    /// detach current handle from mouse control
    void       releaseHandle() { mHandle = 0; }
    
};


#endif

