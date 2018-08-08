// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef DYNAMIC_FIBER_H
#define DYNAMIC_FIBER_H

#include "sim.h"
#include "vector.h"
#include "node_list.h"
#include "fiber.h"

class DynamicFiberProp;


/// A Fiber with discrete growth and dynamic instability at the PLUS_END
/**

 This implements the 1D-model of microtubule dynamic instability proposed by Brun, Rupp et al.
 with a coupling parameter N=2 that cannot be changed.
 
 Assembly and disassembly follow discrete steps of size \a prop->unit_length.
 The model keeps track of the discrete state of the two terminal units of tubulin.
 This leads to 4 different states, which are mapped to [STATE_GREEN, STATE_RED].
 
 
 The growth speed is reduced under antagonistic force by an exponential factor:\n
 <em>
 <b>Measurement of the Force-Velocity Relation for Growing Microtubules</b>\n
 Marileen Dogterom and Bernard Yurke\n
 Science Vol 278 pp 856-860; 1997\n
 http://www.sciencemag.org/content/278/5339/856.abstract
 </em>
 
 ...and this increases the catastrophe rate:\n
 <em>
 <b>Dynamic instability of MTs is regulated by force</b>\n
 M.Janson, M. de Dood, M. Dogterom.\n
 Journal of Cell Biology Vol 161, Nb 6, 2003\n
 Figure 2 C\n
 </em>
 http://www.jcb.org/cgi/doi/10.1083/jcb.200301147
 
 If you use this model, please cite:\n
 <em>
 <b>A theory of microtubule catastrophes and their regulation</b>\n
 Brun L, Rupp B, Ward J, Nedelec F\n
 PNAS 106 (50) 21173-21178; 2009\n
 http://www.pnas.org/content/106/50/21173
 </em>
 
 This is not implemented:
 - the MINUS_END is not dynamic,
 - assembly is not limited by free tubulin pool.
 - rescues are not included.
 .
 
 See the @ref DynamicFiberPar.

 Note: A Gillespie simulation method is used.
 This class is not fully tested (17. Feb 2011).
 @ingroup FiberGroup
 */
class DynamicFiber : public Fiber
{
public:
    
    /// the Property of this object
    DynamicFiberProp const* prop;
   
private:
    
    /// assembly during last time-step
    real       mGrowthP;
    real       mGrowthM;
    
    /// Gillespie times:
    real       nextGrowthP;
    real       nextHydrolP;
    
    real       nextGrowthM;
    real       nextHydrolM;
    
    
    /// state of units near the MinusEnd
    int        unitM[2];
    
    int        stateP;
    
    /// state of units near the PlusEnd: [0] is terminal, [1] is penultimate unit
    int        unitP[2];
    
    int        stateM;
    
    int        calculateStateP() const;
    int        calculateStateM() const;
    
public:
  
    /// constructor
    DynamicFiber(DynamicFiberProp const*);

    /// destructor
    virtual ~DynamicFiber();
        
    //--------------------------------------------------------------------------
    
    /// return assembly/disassembly state of the end \a which
    int         dynamicState(FiberEnd which) const;
    
    /// set state of FiberEnd \a which to \a new_state
    void        setDynamicState(FiberEnd which, int new_state);
    
    /// the amount of freshly assembled polymer during the last time step
    real        freshAssembly(FiberEnd which) const;
    
    //--------------------------------------------------------------------------
    
    /// cut fiber at distance \a abs from MINUS_END
    Fiber *     severM(real abs);
    
    /// join two fibers
    void        join(Fiber * fib);
    
    /// simulate dynamic instability of PLUS_END
    int         stepPlusEnd(real growth_rate_dt);
    
    /// simulate dynamic instability of MINUS_END
    int         stepMinusEnd(real growth_rate_dt);
    
    /// monte-carlo step
    void        step();
    
    //--------------------------------------------------------------------------
    
    /// write to OutputWrapper
    void        write(OutputWrapper&) const;

    /// read from InputWrapper
    void        read(InputWrapper&, Simul&);
    
};


#endif
