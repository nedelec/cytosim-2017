// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef ARRAY_H
#define ARRAY_H

#include "assert_macro.h"
#include "random.h"

/** 
 Array<typename VAL> stores instatiations of the class VAL.
 VAL need to have a public constructor without argument.
 
 This class resembles std::vector<VAL>, with similar functionalities.
 Many functions of std::vector are missing, and some are new: remove(), sort() and mix().
 
 Some functions:
 - allocate(int s) ensures that array[s-1] can then be accessed, as with C-arrays. 
 - operator[](int index) returns the object stored at that index.
 - mix() permutes the values to produce a random ordering.
 - addr() allows direct access to the underlying C-array.
 .
 
 New memory is allocated if necessary by allocate(), and the values
 from the old array are copied to the new memory space.
 
 Allocation when it is done exceeds what is necessary by a bit,
 to ensure that allocation only occurs from time-to-time,
 even if one adds objects one by one to the array.
 
 Destruction of objects is not handled by the Array.
 */

/// Dynamic array of VAL
template <typename VAL>
class Array
{
public:
    
    /// typedef for type used as array index
    typedef unsigned index_type;

    /// typedef for the template argument
    typedef VAL value_type;

    /// iterator class type
    typedef VAL * iterator;
    
private:
    
    /// C-array holding the VAL
    VAL * val_;
    
    /// size of memory that was allocated for val_[]
    unsigned alc_;
    
    /// number of objects currently present in the array
    unsigned nbo_;
    
    /// size of the chunk used for memory allocation (a power of 2)
    unsigned chk_;
    
#pragma mark -
private:
    
    /// the integer above s that is a multiple of chk_
    unsigned chunked(unsigned s)
    {
        return ( s + chk_ - 1 ) & -chk_;
    }
    
    unsigned min(unsigned x, unsigned y)
    {
        return ( x < y ? x : y );
    }
    
    inline void copy(VAL* to, VAL const* from, const unsigned cnt)
    {
        for ( index_type ii=0; ii<cnt; ++ii )
            to[ii] = from[ii];
    }
    
#pragma mark -
public:
        
    /// Default creator without allocation
    Array() : val_(0), alc_(0), nbo_(0), chk_(8)
    {
    }

    /// Allocate size `s`, and set chunk size to `chk`
    Array(unsigned s, unsigned k) : val_(0), alc_(0), nbo_(0)
    {
        //chunk must be a power of 2:
        if ( k & k-1 )
        {
            do
                k = k & (k-1);
            while ( k & (k-1) );
            
            k <<= 1;
        }
        
        if ( !k )
        {
            fprintf(stderr, "Array::chunk must be a positive power of 2");
            exit(1);
        }

        chk_ = k;

        //printf("Array %p new of size %i\n", this, max);
        allocate(s);
    }
    
    /// Copy constructor
    Array(Array<VAL> const & o) : val_(0), alc_(0), nbo_(o.nbo_), chk_(o.chk_)
    {
        if ( o.alc_ )
        {
            //printf("Array %p copy constructor size %i\n", this, nbo_);
            allocate(o.alc_);
            copy(val_, o.val_, min(alc_, o.alc_));
        }
    }

    /// Destructor
    virtual ~Array()
    {
        deallocate();
    }
    
    /// Assignment operator
    Array& operator =(Array<VAL> const & o)
    {
        if ( o.nbo_ > alc_ )
        {
            //printf("Array %p allocated %i = from size %i\n", this, alc_, o.nbo_);
            deallocate();
            allocate(o.nbo_);
        }
        nbo_ = o.nbo_;
        copy(val_, o.val_, nbo_);
        return *this;
    }
    
    
#pragma mark -
    /// Number of objects
    unsigned size() const
    {
        return nbo_;
    }

    /// true if this Array holds no value
    bool  empty() const
    {
        return ( nbo_ == 0 );
    }
    
    /// Currently allocated size
    unsigned capacity() const
    {
        return alc_;
    }
    
    /// Address of the underlying C-array
    VAL * addr() const
    {
        return val_;
    }
    
    /// pointer to first element
    iterator begin() const
    {
        return val_;
    }
    
    /// pointer to a position just past the last element
    iterator end() const
    {
        return val_+nbo_;
    }
    
    /// reference to Object at index ii (val_[ii])
    VAL & at(const index_type ii) const
    {
        assert_true( ii < nbo_ );
        return val_[ii];
    }
    
    /// reference to Object at index ii (val_[ii])
    VAL & operator[](const index_type ii) const
    {
        assert_true( ii < nbo_ );
        return val_[ii];
    }
    
    
#pragma mark -
    /// Allocate to hold `s` objects: valid indices are 0 <= indx < max
    void reallocate(const unsigned alc_new)
    {
        VAL * val_new = new VAL[alc_new];
        if ( val_ )
        {
            copy(val_new, val_, min(alc_, alc_new));
            delete[] val_;
        }
        alc_ = alc_new;
        val_ = val_new;
    }
    
    /// Allocate to hold at least `s` objects: valid indices are 0 <= indx < max
    unsigned allocate(const unsigned s)
    {
        if ( s > alc_ )
        {
            reallocate(chunked(s));
            assert_true( alc_ >= s );
            return s;
        }
        return 0;
    }
    
    /// Allocate and set new values to `zero`
    int allocate_zero(const unsigned size, VAL const& zero)
    {
        int res = allocate(size);
        if ( res )
        {
            //set the newly allocated memory to zero
            for ( index_type ii = nbo_; ii < alc_; ++ii )
                val_[ii] = zero;
        }
        return res;
    }
    
    /// truncate Array to a smaller size
    void truncate(const unsigned size)
    {
        if ( size < nbo_ )
            nbo_ = size;
    }
    
    /// Set the size of this Array to `size` (allocate or truncate if necessary)
    void resize(const unsigned size)
    {
        if ( size < nbo_ )
            nbo_ = size;
        else if ( size > nbo_ )
        {
            allocate(size);
            nbo_ = size;
        }
    }
    
    /// Release occupied memory
    void deallocate()
    {
        if ( val_ )
        {
            //printf("Array %p deallocate %i\n", this, allocated);
            delete[] val_;
            val_ = 0;
        }
        alc_ = 0;
        nbo_ = 0;
    }
    
    /// Set the number of objects to zero
    inline void clear()
    {
        nbo_ = 0;
    }
    
    /// Delete all values as if they were pointers to Object
    void destroy()
    {
        for ( index_type ii=0; ii < nbo_; ++ii )
        {
            if ( val_[ii] )
            {
                //std::cerr << " delete " << val_[ii] << std::endl;
                delete( val_[ii] );
                val_[ii] = 0;
            }
        }
        nbo_ = 0;
    }
    
    /// Set all values to `zero`
    void zero(VAL const& zero)
    {
        for ( index_type ii=0; ii < alc_; ++ii )
            val_[ii] = zero;
    }
    
    
#pragma mark -
    
    /// Increment the size of the array, and return new value at end of it
    VAL & new_val()
    {
        if ( nbo_ >= alc_ )
            allocate(nbo_+1);
        nbo_++;
        return val_[nbo_-1];
    }
    
    /// Add `np` at the end of this Array
    void push_back(const VAL np)
    {
        unsigned n = nbo_ + 1;
        allocate(n);
        val_[nbo_] = np;
        nbo_ = n;
    }
    
    /// Add the elements of `array` at the end of this Array
    void append(const Array<VAL> array)
    {
        allocate(nbo_+array.nbo_);
        for ( index_type ii = 0; ii < array.nbo_; ++ii )
            val_[nbo_++] = array.val_[ii];
    }
    
    /// Return index of `obj`, or -1 if not found in the list (slow scan search)
    int find(const VAL obj) const
    {
        for ( index_type ii = 0; ii < nbo_; ++ii )
            if ( val_[ii] == obj )
                return ii;
        return -1;
    }
    
#pragma mark -
    
    /// Number of times the value `zero` occurs in the array
    unsigned count(VAL const& zero = 0) const
    {
        unsigned res = 0;
        for ( index_type ii = 0; ii < nbo_; ++ii )
            res += ( val_[ii] == zero );
        return res;
    }

    /// sort array using standard C-library function qsort()
    void sort(int (*comp)(const void *, const void *))
    {
        qsort(val_, nbo_, sizeof(VAL), comp);
    }
    
    /// Swap two random values in the array
    VAL& pick_one(Random& rng)
    {
        return val_[rng.pint_exc(nbo_)];
    }

    /// Move the last Object on top, push all other values down by one slot
    void turn()
    {
        if ( nbo_ < 2 ) return;
        
        VAL * tmp = val_[0];
        for ( index_type ii = 0; ii < nbo_-1; ++ii )
            val_[ii] = val_[ii+1];
        val_[nbo_-1] = tmp;
    }
    
    /// Swap two random values in the array
    void permute(Random& rng)
    {
        index_type ii = rng.pint() % nbo_;
        index_type jj = rng.pint() % nbo_;
        if ( ii != jj )
        {
            VAL  tmp = val_[ii];
            val_[ii] = val_[jj];
            val_[jj] = tmp;
        }
    }
    
    
    /// Randomly permutes all objects in the array
    /**
     This produces uniform shuffling in linear time.
     see Knuth's The Art of Programming, Vol 2 chp. 3.4.2 
     */
    void mix(Random& rng)
    {
        index_type jj = nbo_, kk;
        while ( jj > 1 )
        {
            kk = rng.pint() % jj;  //between 0 and j-1
            --jj;
            VAL  tmp = val_[jj];
            val_[jj] = val_[kk];
            val_[kk] = tmp;
        }
    }
};




#endif
