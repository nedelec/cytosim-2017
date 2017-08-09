// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef ALLOT_H
#define ALLOT_H

#include <cstdio>
#include <cstdlib>


/// Allot manages a piece of memory
/**
 Allot holds an array of type T and remembers the size of the array.
 The destructor will release this memory by calling delete[]
 */

template <typename VAL>
class Allot
{
public:
    
    /// copy of the first template argument
    typedef VAL value_type;

private:
    
    /// size currently allocated
    unsigned int alc;
    
    /// array allocated
    VAL * val;
    
    /// copy old memory to new one
    bool  mCopy;

    /// chunk size (a power of 2)
    unsigned int mChunk;
    
private:
    
    /// the integer above s that is a multiple of mChunk
    unsigned int chunked(unsigned int s)
    {
        return ( s + mChunk - 1 ) & -mChunk;
    }
    
    unsigned int min(unsigned int x, unsigned int y)
    {
        return ( x < y ? x : y );
    }
    
    inline void copy(VAL* to, VAL const* from, const unsigned int cnt)
    {
        for ( unsigned int ii=0; ii<cnt; ++ii )
            to[ii] = from[ii];
    }

public:
    
    /// constructor
    Allot() : alc(0), val(0), mCopy(0), mChunk(8) { }
    
    
    /// Allocate size `s`, and set copy flag to `cop`
    Allot(unsigned int s, bool cop) : alc(0), val(0), mCopy(cop), mChunk(8)
    {
        allocate(s);
    }
    
    /// Allocate size `s`, set copy flag to `cop`, and chunk size to `chk`
    Allot(unsigned int s, bool cop, unsigned int chk) : alc(0), val(0), mCopy(cop)
    {
        //chunk must be a multiple of 2:
        if ( chk & chk-1 )
        {
            do
                chk = chk & chk-1;
            while ( chk & chk-1 );
            
            chk <<= 1;
        }
        
        if ( !chk )
        {
            fprintf(stderr, "Allot::chunk must be a positive power of 2");
            exit(1);
        }
        
        mChunk = chk;

        allocate(s);
    }
    
    /// copy constructor
    Allot(Allot<VAL> const& o) : alc(0), val(0), mCopy(o.mCopy), mChunk(o.mChunk)
    {
        if ( o.alc )
        {
            allocate(o.alc);
            copy(val, o.val, min(alc, o.alc));
        }
    }
    
    /// release memory
    void deallocate()
    {
        if ( val )
        {
            delete[] val;
            val = 0;
        }
        alc = 0;
    }
    
    /// destructor
    ~Allot()
    {
        deallocate();
    }
    
    /// copy assignment operator
    Allot& operator = (Allot<VAL> const& o)
    {
        deallocate();
        if ( o.alc )
        {
            allocate(o.alc);
            copy(val, o.val, min(alc, o.alc));
        }
        return *this;
    }
    
    /// change size of allocated memory
    void reallocate(unsigned int alc_new)
    {
        //std::cerr << "Allot " << this << ":reallocate(" << alc_new << ")"<<std::endl;
        VAL * val_new = new VAL[alc_new];
        if ( val )
        {
            if ( mCopy )
                copy(val_new, val, min(alc, alc_new));
            delete[] val;
        }
        alc = alc_new;
        val = val_new;
    }
    
    /// allocate, but only if size increases
    int allocate(unsigned int s)
    {
        if ( s > alc )
        {
            reallocate(chunked(s));
            return s;
        }
        return 0;
    }
    
    /// forget current allocation
    VAL * release()
    {
        VAL * res = val;
        val = 0;
        alc = 0;
        return res;
    }
    
    /// exchange the data between `this` and `o`
    void swap(Allot<VAL> o)
    {
        VAL * v = val;
        unsigned int a = alc;
        val = o.val;
        alc = o.alc;
        o.val = v;
        o.alc = a;
    }
    
    /// allocated size
    unsigned int capacity() const
    {
        return alc;
    }

    /// pointer to data array
    VAL const* addr() const
    {
        return val;
    }

    /// pointer to data array
    VAL* addr()
    {
        return val;
    }
    
    /// conversion to array
    operator VAL * () { return val; }
    
    /// conversion to const array
    operator VAL const* () const { return val; }

    /// access to array
    VAL& operator [] (size_t i) { return val[i]; }

};

#endif

