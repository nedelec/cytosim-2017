// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FIBER_SET_H
#define FIBER_SET_H

#include "dim.h"
#include "object_set.h"
#include "fiber.h"
#include "field_set.h"

class FiberProp;


/// a list of Fiber
/**
 The FiberSet stores Fiber, and derived classes.
 Algorithms that specifically deal with Fibers should be contained here.
 */
class FiberSet : public ObjectSet
{
private:
    
    FiberSet();

public:
    
    /// creator
    FiberSet(Simul& s) : ObjectSet(s) {}
    
    /// destructor
    virtual ~FiberSet() { }
    
    //--------------------------
    
    /// identifies the class
    std::string kind() const { return "fiber"; }
    
    /// create a new property for class `kind` with given name
    Property* newProperty(const std::string& kind, const std::string& name, Glossary&) const;

    //--------------------------
 
    /// create new Fibers
    ObjectList newObjects(const std::string& kd, const std::string& nm, Glossary& opt);
    
    /// construct Fiber
    Object * newObjectT(const Tag tag, int prop_index);
    
    /// first Fiber
    Fiber * first() const
    {
        return static_cast<Fiber*>(nodes.first());
    }
    
    /// last Fiber
    Fiber * last() const
    {
        return static_cast<Fiber*>(nodes.last());
    }
    
    /// return pointer to the Object of given inventory Number, or zero if not found
    Fiber * find(Number n) const
    {
        return static_cast<Fiber*>(inventory.get(n));
    }

    /// Cut all segments intersecting the plane defined by <em> n.x + a = 0 </em>
    void cutAlongPlane(Vector const& n, real a, bool (*func)(Object const*, void*), void*);
    
    /// Monte-Carlo step for every Fiber
    void step();
    
    /// modulo the position (periodic boundary conditions)
    void foldPosition(const Modulo *) const;
    
    /// set random sites along the fibers, separated on average by `spread`
    void uniFiberSites(Array<FiberBinder>&, const real spread) const;

    //--------------------------------------------------------------------------

    /// number of Fiber that do not belong to an Organizer
    int   count(bool (*func)(Fiber const*, void*), void*) const;
    
    /// total length of Fiber 
    real  totalLength() const;

    /// total length of Fiber that have ( FiberProp == p ) 
    real  totalLength(FiberProp const *) const;
    
    /// calculate: number of fibers, mean and standard-deviation of fiber length 
    void  infoLength(unsigned int& cnt, real& avg, real& dev, bool (*func)(Fiber const*, void*) = 0, void* = 0) const;
    
    /// calculate: number of fibers, number of joints and number of kinks
    void  infoSegments(unsigned& cnt, unsigned& joints, unsigned& kinks, real&, real&, bool (*func)(Fiber const*, void*) = 0, void* = 0) const;
    
    /// calculate: center of gravity G, average direction D, average nematic direction N
    real  infoDirection(Vector& G, Vector& D, Vector& N, bool (*func)(Fiber const*, void*) = 0, void* = 0) const;
    
    /// Count Fibers intersecting the plane defined by <em> n.x + a = 0 </em>
    void  infoIntersections(int& np, int& na, Vector const& n, real a) const;

    /// Calculate spindle indices
    void  infoSpindle(real& ixa, real& ixs, Vector const& n, real a, real m, real da) const;
    
    /// sum Lagrange multipliers for segments that intersect the plane <em> n.x + a = 0 </em>
    void  infoTension(unsigned int&, real& hten, Vector const& n, real a) const;
    
    /// sum Lagrange multipliers for all fibers
    void  infoTension(unsigned int&, real& hten) const;

    /// Calculate averaged distance from origin - for all model points
    void  infoRadius(unsigned int&, real& rad) const;
    
    /// Calculate averaged distance from origin - for fiber ends
    void  infoRadius(unsigned int&, real& rad, FiberEnd) const;

};


#endif

