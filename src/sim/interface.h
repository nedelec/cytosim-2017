// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef INTERFACE_H
#define INTERFACE_H


#include "simul.h"
#include <iostream>


/// Cytosim Application Programming Interface
/*
 A reduced set of commands to control and simulate
 a system of objects within cytosim.
 */
class Interface
{
private:
    
    /// disabled default constructor
    Interface();

protected:
    
    /// associated Simul
    Simul& simul;
    
public:
    
    /// associates with given Simul
    Interface(Simul& s);
    
    /// destructor
    virtual ~Interface() {}
    
    //-------------------------------------------------------------------------------
    
    /// this is called between commands during the execution process
    /**
     The overwritten version should call simul.relax() to make sure that
     the simulation data structures are coherent.
     It can perform additional things, for example display the simulation world
     */
    virtual void hold() {}
    
    /// Parse a text containing cytosim commands
    /**
     This is necessary for the 'event code' in parse_run(),
     The function must be defined in the derived class Parser
     */
    virtual void parse(std::istream&, std::string const& msg) = 0;
    
    //-------------------------------------------------------------------------------
    
    /// create a new Property of kind \a k from options set in Glossary
    Property*  execute_set(std::string const& k, std::string const& n, Glossary&);

    /// change values in Property according to Glossary
    void       execute_change(Property *, Glossary&);

    /// change values in Property of kind \a k following options set in Glossary
    Property*  execute_change(std::string const& k, std::string const& n, Glossary&);
    
    /// change 'display' (and only this) in corresponding Property
    void       change_display(std::string const& k, std::string const& n, Glossary&);

    /// create 1 Object of kind \a k with name \a n, following options in Glossary
    ObjectList execute_new(std::string const& k, std::string const& n, Glossary&);
    
    /// create 'cnt' objects of kind \a k with name \a n, randomly placed in space (no option)
    int        execute_new(std::string const& k, std::string const& n, unsigned cnt);
    
    /// delete \c cnt objects of kind \a k with name \a n, following options in Glossary
    void       execute_delete(std::string const& k, std::string const& n, Glossary&, int cnt);
    
    /// mark \c cnt objects of kind \a k with name \a n, following options in Glossary
    void       execute_mark(std::string const& k, std::string const& n, Glossary&, int cnt);

    /// cut fibers, following different options in Glossary
    void       execute_cut(std::string const& k, std::string const& n, Glossary&);
    
    /// import objects from another file
    void       execute_import(std::string const& file, Glossary&);
    
    /// export objects from another file
    void       execute_export(std::string& file, std::string const& what, Glossary&);
    
    /// write output file with object coordinates or information on objects
    void       execute_report(std::string& file, std::string const& what, Glossary&);
    
    /// perform simulation steps
    void       execute_run(Glossary& opt, unsigned cnt, bool do_write);

};

#endif

