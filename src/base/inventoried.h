// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef INVENTORIED_H
#define INVENTORIED_H


/// type for the serial-number of an Object
typedef unsigned long Number;


/// Object that can be entered in a Inventory
/**
 Inventoried provides a serial-number of type Number, used to identify objects in the simulation.
 A serial-number is strictly positive, and it is given only once in each class.
 
 Inventoried [and any derived class] can be registered in a Inventory.
 The Inventory keeps track of all assigned serial-numbers, 
 and can be used to find back an object of a given serial-number.
*/
class Inventoried
{
    
    friend class Inventory;
    
protected:
    
    /// object identifier, unique within the class defined by tag()
    Number   nNumber;
    
public:
    
    /// initialize (name=0)
    Inventoried() : nNumber(0) {}
    
    /// destructor
    ~Inventoried() {}
    
    
    /// change the serial number
    void    number(Number n)   { nNumber = n; }
    
    /// serial number : a integer identifier, unique within each class
    Number  number()     const { return nNumber; }
    
};


#endif
