// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

class Prop
{

public:
    
    char          string[256];
    int           an_int;
    int           an_array[6];
    long          a_long;
    unsigned long a_ulong;
    
    float         a_float;
    double        a_double;
    
    Prop() {}
    
    virtual ~Prop() { }
};
