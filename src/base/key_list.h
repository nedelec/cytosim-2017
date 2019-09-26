// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef KEY_LIST_H
#define KEY_LIST_H

#include "assert_macro.h"
#include <string>
#include <vector>
#include <iostream>
#include <sstream>



/// stores a set of pairs ( string, values ). Used by Glossary::set()
template <typename val_type>
class KeyList
{
public:
    
    /// type for a key in KeyList
    typedef std::string key_type;
    
    /// type for a key-value pair
    struct key_value 
    {
        /// key
        key_type   key;
        /// value
        val_type   val;
        /// constructor with initialization
        key_value(key_type const& k, val_type v)
        {
            key=k;
            val=v;
        }
    };
    
private:
    
    /// list of keys-value pairs
    std::vector<key_value> map;
    
public:
    
    /// constructor
    KeyList() { }
    
    /// constructor
    KeyList(key_type k0, val_type v0)
    {
        push_back(k0, v0);
    }
    
    /// constructor
    KeyList(key_type k0, val_type v0, key_type k1, val_type v1)
    {
        push_back(k0, v0); push_back(k1, v1);
    }
    
    /// constructor
    KeyList(key_type k0, val_type v0, key_type k1, val_type v1, key_type k2, val_type v2)
    {
        push_back(k0, v0); push_back(k1, v1); push_back(k2, v2);
    }

    /// constructor
    KeyList(key_type k0, val_type v0, key_type k1, val_type v1, key_type k2, val_type v2,
            key_type k3, val_type v3)
    {
        push_back(k0, v0); push_back(k1, v1); push_back(k2, v2);
        push_back(k3, v3);
    }
    
    /// constructor
    KeyList(key_type k0, val_type v0, key_type k1, val_type v1, key_type k2, val_type v2,
            key_type k3, val_type v3, key_type k4, val_type v4)
    {
        push_back(k0, v0); push_back(k1, v1); push_back(k2, v2);
        push_back(k3, v3); push_back(k4, v4);
    }
    
    /// constructor
    KeyList(key_type k0, val_type v0, key_type k1, val_type v1, key_type k2, val_type v2,
            key_type k3, val_type v3, key_type k4, val_type v4, key_type k5, val_type v5)
    {
        push_back(k0, v0); push_back(k1, v1); push_back(k2, v2);
        push_back(k3, v3); push_back(k4, v4); push_back(k5, v5);
    }
    
    /// constructor
    KeyList(key_type k0, val_type v0, key_type k1, val_type v1, key_type k2, val_type v2,
            key_type k3, val_type v3, key_type k4, val_type v4, key_type k5, val_type v5,
            key_type k6, val_type v6)
    {
        push_back(k0, v0); push_back(k1, v1); push_back(k2, v2);
        push_back(k3, v3); push_back(k4, v4); push_back(k5, v5);
        push_back(k6, v6);
    }

    /// constructor
    KeyList(key_type k0, val_type v0, key_type k1, val_type v1, key_type k2, val_type v2,
            key_type k3, val_type v3, key_type k4, val_type v4, key_type k5, val_type v5,
            key_type k6, val_type v6, key_type k7, val_type v7)
    {
        push_back(k0, v0); push_back(k1, v1); push_back(k2, v2);
        push_back(k3, v3); push_back(k4, v4); push_back(k5, v5);
        push_back(k6, v6); push_back(k7, v7);
    }

    /// constructor
    KeyList(key_type k0, val_type v0, key_type k1, val_type v1, key_type k2, val_type v2,
            key_type k3, val_type v3, key_type k4, val_type v4, key_type k5, val_type v5,
            key_type k6, val_type v6, key_type k7, val_type v7, key_type k8, val_type v8)
    {
        push_back(k0, v0); push_back(k1, v1); push_back(k2, v2);
        push_back(k3, v3); push_back(k4, v4); push_back(k5, v5);
        push_back(k6, v6); push_back(k7, v7); push_back(k8, v8);
    }

    /// constructor
    KeyList(key_type k0, val_type v0, key_type k1, val_type v1, key_type k2, val_type v2,
            key_type k3, val_type v3, key_type k4, val_type v4, key_type k5, val_type v5,
            key_type k6, val_type v6, key_type k7, val_type v7, key_type k8, val_type v8,
            key_type k9, val_type v9)
    {
        push_back(k0, v0); push_back(k1, v1); push_back(k2, v2);
        push_back(k3, v3); push_back(k4, v4); push_back(k5, v5);
        push_back(k6, v6); push_back(k7, v7); push_back(k8, v8);
        push_back(k9, v9);
    }

    /// constructor
    KeyList(key_type k0, val_type v0, key_type k1, val_type v1, key_type k2, val_type v2,
            key_type k3, val_type v3, key_type k4, val_type v4, key_type k5, val_type v5,
            key_type k6, val_type v6, key_type k7, val_type v7, key_type k8, val_type v8,
            key_type k9, val_type v9, key_type kA, val_type vA)
    {
        push_back(k0, v0); push_back(k1, v1); push_back(k2, v2);
        push_back(k3, v3); push_back(k4, v4); push_back(k5, v5);
        push_back(k6, v6); push_back(k7, v7); push_back(k8, v8);
        push_back(k9, v9); push_back(kA, vA);
    }
    
    /// constructor
    KeyList(key_type k0, val_type v0, key_type k1, val_type v1, key_type k2, val_type v2,
            key_type k3, val_type v3, key_type k4, val_type v4, key_type k5, val_type v5,
            key_type k6, val_type v6, key_type k7, val_type v7, key_type k8, val_type v8,
            key_type k9, val_type v9, key_type kA, val_type vA, key_type kB, val_type vB)
    {
        push_back(k0, v0); push_back(k1, v1); push_back(k2, v2);
        push_back(k3, v3); push_back(k4, v4); push_back(k5, v5);
        push_back(k6, v6); push_back(k7, v7); push_back(k8, v8);
        push_back(k9, v9); push_back(kA, vA); push_back(kB, vB);
    }
    
    /// number of entries
    size_t size()                       const { return map.size(); }
    
    /// return n-th entry
    const key_value & operator[](int n) const { return map[n]; }
    
    /// add new entry (k, v)
    void push_back(key_type const& k, val_type v) { map.push_back(key_value(k, v)); }

    /// this will accept the key or the ascii representation of its value
    bool set(val_type& val, key_type const& key) const
    {        
        std::ostringstream oss;
        for ( unsigned int n = 0; n < map.size(); ++n )
        {
            // write in 'oss' the ascii representation of the value
            oss.str("");
            oss << map[n].val;
            // accept both the string key or the ascii-value
            if ( key == map[n].key  ||  key == oss.str() )
            {
                val = map[n].val;
                //std::cerr << "KeyList::set  " << key << " -> " << val << std::endl;
                return true;
            }
        }
        return false;
    }
};



extern const char PREF[];


/// output operator
template <typename T>
std::ostream & operator << (std::ostream & os, const KeyList<T> & list)
{
    os << "Known values are:" << std::endl;
    for ( unsigned int n = 0; n < list.size(); ++n )
        os << PREF << list[n].key << " = " << list[n].val << std::endl;
    return os;
}



#endif


