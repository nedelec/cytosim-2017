// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef BUDDY_H
#define BUDDY_H

#include <vector>
#include <algorithm>


/// Maintains mutual relationship between objects.
/**
 Buddy implements mutual relationship between objects.
 
 Relationship is established with hello().
 Then goodbye() will be called for all buddies, when the object is destroyed.
 
 This is the only thing that the class does.
 This class can be used if an object needs to know when another object is destroyed,
 and vice-versa.
 
 F. Nedelec 11 Aug. 2012
 */
class Buddy
{

private:
    
    /// type for a list of buddies
    typedef std::vector<Buddy *> BuddyList;
    
    /// list of buddies
    BuddyList buddies;
    
    
    /// this will add \a b into the list of buddies, if not already present
    void hello1(Buddy * b)
    {
        // do nothing if buddy is know already:
        BuddyList::iterator bi = std::find(buddies.begin(), buddies.end(), b);
        if ( bi != buddies.end() )
            return;
        
        bi = std::find(buddies.begin(), buddies.end(), (Buddy*)0);
        if ( bi != buddies.end() )
        {
            *bi = b;
            return;
        }
        buddies.push_back(b);
    }
        
    /// removes \a b from the list of known buddy, and call goodbye()
    bool goodbye1(Buddy * b)
    {
        BuddyList::iterator bi = std::find(buddies.begin(), buddies.end(), b);
        if ( bi != buddies.end() )
        {
            *bi = 0;
            goodbye(b);
            return true;
        }
        return false;
    }
    
public:
    
    Buddy() {}
    
    /// upon destruction, goodbye is called for all buddies
    virtual ~Buddy()
    {
        for ( BuddyList::iterator bi = buddies.begin(); bi < buddies.end(); ++bi )
            if ( *bi )
                (*bi)->goodbye1(this);
    }

    /// will make \a this and \a b mutual buddies
    void hello(Buddy * b)
    {
        if ( b )
        {
            hello1(b);
            b->hello1(this);
        }
    }
    
    /// this is called everytime a known buddy is destroyed
    virtual void goodbye(Buddy *)
    {
    }
    
    /// return buddy at index \a ix
    Buddy * buddy(unsigned int ix) const
    {
        if ( ix < buddies.size() )
            return buddies[ix];
        return 0;
    }
    
    /// replace the buddy that may have been at index \a ix
    void buddy(Buddy * b, unsigned int ix)
    {
        if ( ix < buddies.size() )
        {
            if ( buddies[ix] )
                buddies[ix]->goodbye1(this);
        }
        else
            buddies.resize(ix+1, 0);
        
        buddies[ix] = b;
    }
    
};

#endif

