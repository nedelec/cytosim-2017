// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SIMUL_H
#define SIMUL_H

#include "assert_macro.h"
#include <iostream>
#include <stack>
#include <map>

#include "single_set.h"
#include "couple_set.h"
#include "fiber_set.h"
#include "bead_set.h"
#include "solid_set.h"
#include "sphere_set.h"
#include "organizer_set.h"
#include "field_set.h"
#include "space_set.h"
#include "fiber_grid.h"
#include "point_grid.h"
#include "simul_prop.h"
#include "property_list.h"
#include "field_values.h"
#include "field.h"
#include "meca.h"



/// the string that defines the start of a frame
const static char FRAME_TAG[] = "#Cytosim ";


/// Simulator class
class Simul
{    
private:
    
    /// time in the simulated world
    real               sTime;
    
    /// True if the simulation is ready to do a step
    bool               sReady;
    
    /// the last Space defined in the simulation
    Space *            sSpace;
    
    /// The Meca used to set and integrate the equations of motion
    mutable Meca       sMeca;
    
    /// grid used for attachment to Fiber
    mutable FiberGrid  fiberGrid;
    
    /// grid used for steric interaction of Solid fat-points and Sphere
    mutable PointGrid  stericGrid;
    
    //-------------------------------------------------------------------------------
    
    /// a copy of the properties that were stored to file
    mutable std::string properties_saved;
   
public:

    /// Global cytosim parameters
    SimulProp *  prop;
    
    /// holds all Property, except the SimulProp
    PropertyList properties;
    
    
    /// list of Space
    SpaceSet     spaces;
    
    /// list of Field
    FieldSet     fields;
    
    /// list of Fiber
    FiberSet     fibers;
    
    /// list of Sphere
    SphereSet    spheres;
    
    /// list of Bead
    BeadSet      beads;
    
    /// list of Solid
    SolidSet     solids;
    
    /// list of Single
    SingleSet    singles;
    
    /// list of Couple
    CoupleSet    couples;
    
    /// list of organizers
    OrganizerSet organizers;
    
    //-------------------------------------------------------------------------------
    
    /// constructor
    Simul();
    
    /// destructor
    virtual ~Simul();
        
    //-------------------------------------------------------------------------------
    
    /// link Object
    void      add(Object *);

    /// link Objects
    int       add(ObjectList const&);

    /// unlink Object
    void      remove(Object *);

    /// unlink Objects
    void      remove(ObjectList const&);
    
    /// unlink and delete object
    void      erase(Object *);
    
    /// unlink and delete all objects in list
    void      erase(ObjectList const&);

    /// mark objects in list
    static void mark(ObjectList const&, int);

    /// erase simulation world (all sub-lists and variables)
    void      erase();
    
    //-------------------------------------------------------------------------------

    /// time in the simulated world
    real      simTime()   const { return sTime; }
   
    /// set frame index
    void      setTime(real t)   { sTime = t; }
    
    //-------------------------------------------------------------------------------
   
    /// perform basic initialization; register callbacks
    void      initialize(Glossary&);
    
    /// initialize the simulation engine
    void      prepare(Space * spc);
  
    /// initialize the simulation engine using the current space
    void      prepare() { prepare(space()); }
    
    /// returns true is the simulation is ready to run (or appears so)
    bool      isReady() const { return sReady; }
    
    /// call after a sequence of step() have been done
    void      relax() { couples.relax(); }
    
    /// set current Space
    void      space(Space * spc);

    /// get current Space
    Space *   space() const { return sSpace; }
    
    /// return first Space with this name
    Space *   findSpace(const std::string& name) const;

    /// call foldPosition() for all objects
    void      foldPosition() const;
    
    //-------------------------------------------------------------------------------
    
    /// perform the Monte-Carlo part of the simulation for one time_step
    void      step();

    /// calculate the mechanics of the system and move objects accordingly, for one time_step
    void      solve();
    
    /// calculate Forces and Lagrange multipliers, but do not move objects
    void      computeForces() const;

    /// dump matrix and vector from Meca
    void      dump() const { sMeca.dump(); }
    
    //-------------------------------------------------------------------------------
    
    /// call setInteractions(meca) for all objects
    void      setInteractions(Meca&) const;
    
private:
    
    /// give an estimate for simul:binding_grid_step
    real      estimateFiberGridStep() const;
    
    /// set fiberGrid and stericGrid over the given space
    void      setFiberGrid(Space *) const;
    
    /// calculate the range of the steric grid automatically
    real      estimateStericRange() const;
    
    /// initialize the grid for steric interaction (stericGrid)
    void      setStericGrid(Space const*) const;
    
    /// add steric interactions between spheres, solids and fibers
    void      setStericInteractions(Meca&) const;
    
    //-------------------------------------------------------------------------------
    /// Function used to parse the config file, and to read state from a file:
    //-------------------------------------------------------------------------------

    /// return the ObjectSet corresponding to this Tag in the simulation (used for IO)
    ObjectSet* findSet(const Tag);

    /// read sim-state from IO (low-level function)
    int        readObjects(InputWrapper&);
    
public:
    
    /// return the ObjectSet corresponding to a class
    ObjectSet* findSet(const std::string& kind);
    
    /// return the ObjectSet corresponding to a class
    ObjectSet const* findSet(const std::string& kind) const { return const_cast<Simul*>(this)->findSet(kind); }
    
    /// find an object from the class-TAG and the inventory number
    Object*   findObject(const Tag, long);
    
    /// find an object from the name of its class and the inventory number
    Object*   findObject(const std::string&, long) const;
    
    /// find a fiber from the name of the fiber
    Fiber*    findFiber(long num) const { return static_cast<Fiber*>(fibers.findObject(num)); }
    
    /// read an Object reference and return the corresponding Object (\c tag is set)
    Object*   readReference(InputWrapper&, char& tag);

    /// check if the name corresponds to a property class
    bool      isProperty(const std::string&) const;
    
    /// get an existing property, or return zero
    Property* findProperty(const std::string&, const std::string&) const;
    
    /// get an existing property, or return zero
    Property* findProperty(const std::string&, const int index) const;

    /// find a SingleProp
    SingleProp* findSingleProp(const std::string& nm) const;

    /// return all existing propertis of required class
    PropertyList findProperties(const std::string&) const;

    /// create a new property
    Property* newProperty(const std::string&, const std::string&, Glossary&);
    
    /// write all properties
    void      writeProperties(std::ostream&, bool prune) const;
    
    /// write sim-state to a named file
    void      writeProperties(std::string& file, bool prune) const;

    //-------------------------------------------------------------------------------

    /// read sim-state from InputWrapper
    int       reloadObjects(InputWrapper&);
    
    /// read sim-state from a named file
    int       loadObjects(InputWrapper&);
    
    /// write simulation-state to specified file
    void      writeObjects(OutputWrapper&) const;
    
    /// write simulation-state in binary or text mode, appending to the file or not
    void      writeObjects(std::string const& file, bool binary, bool append) const;
    
    //-------------------------------------------------------------------------------
    
    /// call `Simul::report0`, adding lines before and after with 'start' and 'end' tags.
    void      report(std::ostream&, std::string const&, Glossary&) const;
    
    /// call one of the report function
    void      report0(std::ostream&, std::string const&, Glossary&) const;
    
    /// print time
    void      reportTime(std::ostream&) const;
   
    /// analyse the network connectivity to identify isolated sub-networks
    void      analyzeClusters() const;
    
    /// print size of clusters defined by connections with Couples
    void      reportClusters(std::ostream&, bool) const;
    
    /// print the length and the points of each fiber
    void      reportFiber(std::ostream&) const;
    
    /// print the length and the points of each fiber
    void      reportFiber(std::ostream&, FiberProp const*) const;
    
    /// print the coordinates of the model-points of each fiber
    void      reportFiberPoints(std::ostream&) const;
    
    /// print the mean and standard deviation of model-points of all fibers
    void      reportFiberMoments(std::ostream&) const;

    /// print the coordinates and forces on the model-points of each fiber
    void      reportFiberForces(std::ostream&) const;
    
    /// print the positions and the states of the two ends of each fiber
    void      reportFiberEnds(std::ostream&) const;
    
    /// print average length and standard deviation for each class of fiber
    void      reportFiberLengths(std::ostream&) const;
    
    /// print length distribution for each class of fiber
    void      reportFiberLengthDistribution(std::ostream&, real delta, real max) const;
    
    /// print number of kinks in each class of Fiber
    void      reportFiberSegments(std::ostream&) const;
    
    /// print number of fibers according to dynamic state of end
    void      reportFiberDynamic(std::ostream&, FiberEnd) const;
    
    /// print number of fibers according to their dynamic states
    void      reportFiberDynamic(std::ostream&) const;
    
    /// print coordinates of points along the length of all Fiber
    void      reportFiberSpeckles(std::ostream&, real) const;
    
    /// print dynamic states of Fiber
    void      reportFiberStates(std::ostream&) const;
    
    /// print Fiber tensions along certain planes defined in `opt`
    void      reportFiberTension(std::ostream&, Glossary& opt) const;
    
    /// print Organizer positions
    void      reportOrganizer(std::ostream&) const;
    
    /// print Aster positions
    void      reportAster(std::ostream&) const;
    
    /// print Bead positions
    void      reportBeadSingles(std::ostream&) const;
    
    /// print Bead positions
    void      reportBeadPosition(std::ostream&) const;
    
    /// print Solid positions
    void      reportSolid(std::ostream&) const;
    
    /// print state of Couples
    void      reportCouple(std::ostream&) const;
    
    /// print position of Couples
    void      reportCouplePosition(std::ostream&) const;
    
    /// print position of Couples of a certain kind
    void      reportCouplePosition(std::ostream&, std::string const&) const;
    
    /// print info on doubly bound Couples 
    void      reportCoupleLink(std::ostream&, std::string const&) const;
    
    /// print numbers of different types of link made by each couple
    void    reportCoupleType(std::ostream&, std::string const&) const;
    
    /// print state of Singles
    void      reportSingle(std::ostream&) const;
    
    /// print position of Singles
    void      reportSinglePosition(std::ostream&) const;
    
    /// print position of Singles
    void      reportSingleForce(std::ostream&) const;
    
    /// print position of Singles of a certain kind
    void      reportSinglePosition(std::ostream&, std::string const&) const;
    
    /// print state of Couples
    void      reportSphere(std::ostream&) const;
    
    /// print something about Spaces
    void      reportSpace(std::ostream&) const;
    
    /// print something
    void      reportCustom(std::ostream&) const;

    //-------------------------------------------------------------------------------
    
    /// custom function
    void      custom0();
    /// custom function
    void      custom1();
    /// custom function
    void      custom2();
    /// custom function
    void      custom3();
    /// custom function
    void      custom4();
    /// custom function
    void      custom5();
    /// custom function
    void      custom6();
    /// custom function
    void      custom7();
    /// custom function
    void      custom8();
    /// custom function
    void      custom9();
};

#endif

