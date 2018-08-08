// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef COUPLE_SET_H
#define COUPLE_SET_H

#include "object_set.h"
#include "couple.h"
#include "couple_prop.h"
#include <stack>

/// Set for Couple
/**
 A Couple is stored in one of 4 NodeList, depending on its state:
 - ffList = free.
 - afList = hand1 attached, hand2 free.
 - faList = hand1 free, hand2 attached.
 - aaList = hand1 and hand2 attached [also called bridge].
 .
 The lists are accessible via firstFF() and firstFA(), firstAF() and firstAA.
 This way, when scanning the Couple, the states of both hands are known,
 and we can save the corresponding tests. This is very efficient.
 
 A Couple is automatically transfered to the appropriate list,
 if one of its Hand binds or unbind. This is one role of HandMonitor:
 HandMonitor::afterAttachment() and HandMonitor::afterDetachment() 
 are called by the Hand, and call CoupleSet::relink().
 */
class CoupleSet: public ObjectSet
{
    
private:
    
    /// list of Couple which are not attached (f=free)
    NodeList    ffList;
    
    /// list of Couple with only one side attached (a=attached, f=free)
    NodeList    afList, faList;
    
    /// list of Couple with both sides attached (a=attached)
    NodeList    aaList;
    
    /// frozen list
    NodeList    ffIce, faIce, afIce, aaIce;
    
    
    /// a list to hold Couples of one class
    typedef std::stack<Couple*> CoupleList;
    
    /// an array of CoupleList
    typedef std::vector<CoupleList> CoupleReserve;
    
    /// uniLists[p] contains the Couples with ( property()->index() == p ) that are diffusing
    CoupleReserve      uniLists;
    
    /// flag to enable couple:fast_diffusion attachment algorithm
    bool               uni;
    
    /// initialize couple:fast_diffusion attachment algorithm
    bool         uniPrepare(PropertyList& properties);
    
    /// implements couple:fast_diffusion attachment algorithm for one class of Couple
    void         uniAttach(FiberSet const&, std::stack<Couple*>&);
    
    /// couple:fast_diffusion attachment algorithm; assumes free Couples are uniformly distributed
    void         uniAttach(FiberSet const&);
    
    /// return Couples in uniLists to the normal lists
    void         uniRelax();

public:
    
    ///creator
    CoupleSet(Simul& s) : ObjectSet(s), ffList(this), afList(this), faList(this), aaList(this), uni(false) {}
    
    ///destructor
    virtual ~CoupleSet() {}
    
    //--------------------------
    
    /// identifies the class
    std::string  kind() const { return "couple"; }
    
    /// create a new property for class \a kind with given name
    Property *   newProperty(const std::string& kind, const std::string& name, Glossary&) const;
    
    //--------------------------
    
    /// create new objects
    ObjectList   newObjects(const std::string& kd, const std::string& nm, Glossary& opt);

    /// construct object
    Object *     newObjectT(const Tag tag, int prop_index);

    /// first free
    Couple *     firstFF()     const { return static_cast<Couple*>(ffList.first()); }
    
    /// last free
    Couple *     lastFF()      const { return static_cast<Couple*>(ffList.last()); }
    
    /// first Couple attached by cHand1
    Couple *     firstAF()     const { return static_cast<Couple*>(afList.first()); }
    
    /// first Couple attached by cHand2
    Couple *     firstFA()     const { return static_cast<Couple*>(faList.first()); }
    
    /// first Couple attached by both hands
    Couple *     firstAA()     const { return static_cast<Couple*>(aaList.first()); }
    
    /// number of free Couples
    unsigned int sizeFF()      const { return ffList.size(); }
    /// number of Couples attached by cHand1
    unsigned int sizeAF()      const { return afList.size(); }
    /// number of Couples attached by cHand2
    unsigned int sizeFA()      const { return faList.size(); }
    /// number of Couples attached by both hands
    unsigned int sizeAA()      const { return aaList.size(); }
    /// total number of elements
    unsigned int size()        const { return ffList.size() + faList.size() + afList.size() + aaList.size(); }
    
    /// return pointer to the Object of given Number, or zero if not found
    Couple *     find(Number n) const { return static_cast<Couple*>(inventory.get(n)); }
    
    /// register into the list
    void         link(Object *);
    
    /// collect Object for which func(this, val) == true
    ObjectList   collect(bool (*func)(Object const*, void*), void*) const;

    /// erase all Object and all Property
    void         erase();
    
    /// mix order of elements
    void         mix();
    
    
    
    /// prepare for step()
    void         prepare(PropertyList& properties);
    
    /// Monte-Carlo step
    void         step(FiberSet const&, FiberGrid const&);
    
    /// return all reserves to the normal lists
    void         relax();
     
    
    /// transfer all object to ice
    void         freeze();
    
    /// delete objects, or put them back in normal list
    void         thaw(bool erase);
    
    /// write
    void         write(OutputWrapper&) const;
    
    /// modulo the position (periodic boundary conditions)
    void         foldPosition(const Modulo *) const;
    
    ///debug function
    int          bad() const;
};


#endif

