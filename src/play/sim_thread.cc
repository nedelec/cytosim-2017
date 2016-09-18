// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <sys/time.h>
#include <time.h>
#include "sim_thread.h"
#include "exceptions.h"
#include "picket.h"


//------------------------------------------------------------------------------

/**
 This uses a Parser that cannot write to disc.
 The function callback is called when Parser::hold() is reached.
 */
SimThread::SimThread(void (*hold_callback)(void))
: Parser(simul, 1, 1, 1, 1, 0), holding(hold_callback)
{
    mStatus = 0;
    mHold   = 0;
    mPeriod = 1;
    pthread_mutex_init(&mMutex, 0);
    pthread_cond_init(&mCondition, 0);
}

/**
 Known issue:
 When quitting the application, this destructor might be called after the
 destructor of Simul(), in which case it will access non-existent data,
 most likely causing a crash().
 */
SimThread::~SimThread()
{
    stop();
    pthread_detach(mThread);
}

//------------------------------------------------------------------------------
#pragma mark -

void SimThread::hold()
{
    //assert_true( pthread_equal(pthread_self(), mThread) );

    if ( ++mHold >= mPeriod )
    {
        holding();
        mHold = 0;
        if ( mStatus > 0 )
            pthread_cond_wait(&mCondition, &mMutex);
        else
        {
            // if ( mStatus < 0 ), we exit
            mStatus = 0;
            unlock();
            pthread_exit(0);
        }
    }
}


void SimThread::run()
{
    //assert_true( pthread_equal(pthread_self(), mThread) );
    
    try {
        Parser::readConfig(simul.prop->config);
    }
    catch( Exception & e ) {
        std::cerr << std::endl << "Error: " << e.what() << std::endl;
        //flashText("Error: simulation died");
    }
    mStatus = 0;
    unlock();
}


void* run_thread(void * arg)
{
    SimThread * lt = static_cast<SimThread*>(arg);
    lt->run();
    return 0;
}


void SimThread::run_more()
{
    //assert_true( pthread_equal(pthread_self(), mThread) );
    
    try {
        simul.prepare();
        while ( 1 )
        {
            simul.step();
            simul.solve();
            hold();
        }
    }
    catch( Exception & e ) {
        std::cerr << std::endl << "Error: " << e.what() << std::endl;
        simul.relax();
        mStatus = 0;
        unlock();
        //flashText("Error: %s", e.what());
    }
}


void* run_more_thread(void * arg)
{
    SimThread * lt = static_cast<SimThread*>(arg);
    lt->run_more();
    return 0;
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 This attempts to start the live simulation
 */
int SimThread::start()
{
    if ( mStatus == 0 )
    {
        if ( 0 == trylock() )
        {
            mStatus = 1;
            pthread_create(&mThread, 0, run_thread, this);
            return 0;
        }
    }
    return 1;
}


int SimThread::persist()
{
    if ( mStatus == 0 )
    {
        if ( 0 == trylock() )
        {
            mStatus = 2;
            pthread_create(&mThread, 0, run_more_thread, this);
            return 0;
        }
    }
    return 1;
}


/**
 ask the live-thread to exit at the next spontaneous halt
*/ 
void SimThread::stop()
{
    if ( mStatus > 0 )
    {
        // request termination:
        mStatus = -1;
        release();
        // wait for termination:
        pthread_join(mThread, 0);
    }
}

/**
 kill the live-thread immediately
 */
void SimThread::cancel()
{
    if ( mStatus > 0 )
    {
        // request termination:
        if ( 0 == pthread_cancel(mThread) )
        {
            // wait for termination:
            pthread_join(mThread, 0);
            unlock();
            mStatus = 0;
        }
    }
}

//------------------------------------------------------------------------------
#pragma mark -


SingleProp * SimThread::getHandleProperty(real range)
{
    Property * p = simul.properties.find("single", "mouse");
    
    if ( p == 0 )
    {
        HandProp * hp = new HandProp("mouse");
        
        // Attach fast and never detach:
        hp->binding_range   = range;
        hp->binding_rate    = 1000;
        hp->unbinding_rate  = 0;
        hp->unbinding_force = INFINITY;
        hp->complete(simul.prop, &simul.properties);
        simul.properties.deposit(hp);
        
        SingleProp * sp = new SingleProp("mouse");
        sp->hand = "mouse";
        sp->stiffness = 1000;
        sp->complete(simul.prop, &simul.properties);
        simul.properties.deposit(sp);
        return sp;
    }
    
    return static_cast<SingleProp*>(p);
}


Single * SimThread::createHandle(const Vector & pos, real range)
{
    SimThread::Lock lck(this);
    SingleProp * sp = getHandleProperty(range);
    Single * res = new Picket(sp, pos);
    mHandles.push_back(res);
    simul.singles.add(res);
    mHandle = res;
    return res;
}


bool SimThread::selectClosestHandle(Vector const& pos, real range)
{
    Single * res = 0;
    real dsm = 0;
    for ( Single ** oi = mHandles.begin(); oi < mHandles.end(); ++oi )
    {
        real d = ( (*oi)->posFoot() - pos).norm();
        if ( res == 0  ||  d < dsm )
        {
            res = *oi;
            dsm = d;
        }
    }
    if ( res && dsm < range )
    {
        mHandle = res;
        return 1;
    }
    return 0;
}


void SimThread::detachHandle()
{
    if ( mHandle )
    {
        SimThread::Lock lck(this);
        if ( mHandle->attached() )
            mHandle->detach();
    }
}

void SimThread::moveHandle(const Vector & pos)
{
    if ( mHandle )
    {
        SimThread::Lock lck(this);
        mHandle->setPosition(pos);
    }
}


void SimThread::moveHandles(const Vector & vec)
{
    SimThread::Lock lck(this);
    for ( Single ** oi = mHandles.begin(); oi < mHandles.end(); ++oi )
        (*oi)->translate(vec);
}


void SimThread::deleteHandles()
{
    SimThread::Lock lck(this);
    for ( unsigned int ii = 0; ii < mHandles.size(); ++ii )
    {
        if ( mHandles[ii] )
            simul.erase( mHandles[ii] );
    }
    mHandles.clear();
    mHandle = 0;
}


void SimThread::clear()
{
    simul.erase();
    mHandles.clear();
    mHandle = 0;
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 Read config file from the start, allowing parameters to be changed, 
 while simulation objects remain as they are.
 
 If the simulation is running live, this will pause it,
 read the config file, and allow it to proceed.
 */
void SimThread::reloadConfig()
{
    Lock lck(this);
    // the parser can only change properties:
    Parser(simul, 0, 1, 0, 0, 0).readConfig(simul.prop->config);
    simul.prop->display_fresh = false;
}


/**
 This will execute the code specified in \a iss.
 
 If the simulation is running live, the SimThread is paused,
 and the code is executed with another Parser.
 When that is completed, the SimThread is released.
 
 The parser has full rights during the execution
 */
int  SimThread::execute(std::istream& iss)
{
    SimThread::Lock lck(this);
    try {
        Parser(simul, 1, 1, 1, 1, 1).parse(iss, "executing magic code");
    }
    catch( Exception & e ) {
        std::cerr << "Error : " << e.what();
    }
    return 0;
}

/**
 Save current state in two files
 */
void SimThread::writeState()
{
    Lock lck(this);
    try {
 
        std::ostringstream oss;
        oss << std::setw(4) << std::setfill('0') << reader.frame() << ".cmo";
        
        std::string pfile = "properties"+oss.str();
        simul.writeProperties(pfile, true);
        
        std::string ofile = "objects"+oss.str();
        OutputWrapper out(ofile.c_str(), false, false);
        simul.writeObjects(out);
    }
    catch( Exception & e ) {
        std::cerr << "Error in Simul::writeState(): " << e.what();
    }
}


void SimThread::writeProperties(std::ostream& os, bool prune)
{
    Lock lck(this);
    try {
        simul.writeProperties(os, prune);
    }
    catch( Exception & e ) {
        std::cerr << "Error in Simul::writeState(): " << e.what();
    }
}


