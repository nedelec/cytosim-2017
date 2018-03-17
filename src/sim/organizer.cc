// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "organizer.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"


//------------------------------------------------------------------------------
Organizer::~Organizer()
{
    //Cytosim::MSG(31, "destroying Organizer %p\n", this);
}


void Organizer::grasp(Mecable * m)
{
    hello(m);    
    objs.push_back(m);
}


void Organizer::grasp(Mecable * m, unsigned int ix)
{
    if ( ix >= objs.size() )
        objs.resize(ix+1, 0);

    if ( m != objs[ix] )
    {
        Buddy::goodbye(objs[ix]);
        hello(m);
    }
    
    objs[ix] = m;
}


void Organizer::goodbye(Buddy * b)
{
    if ( b )
    {
        MecableList::iterator oi = std::find(objs.begin(), objs.end(), b);
        if ( oi != objs.end() )
            *oi = 0;
    }
}


//------------------------------------------------------------------------------
/**
 \return The centroid from all the object positions
 */
Vector Organizer::position() const
{
    Vector res(0,0,0);
    for ( MecableList::const_iterator oi = objs.begin(); oi < objs.end(); ++oi )
        res += (*oi)->position();
    return res / objs.size();
}


Vector Organizer::positionP(unsigned int ix) const
{
    Vector res(0,0,0);
    for ( MecableList::const_iterator oi = objs.begin(); oi < objs.end(); ++oi )
        res += (*oi)->posPoint(ix);
    return res / objs.size();
}


void Organizer::translate( Vector const& T )
{
    for ( MecableList::iterator oi = objs.begin(); oi < objs.end(); ++oi )
    {
        Movable * mv = *oi;
        if ( mv ) mv->translate(T);
    }
}


void Organizer::rotate( Rotation const& T )
{
    for ( MecableList::iterator oi = objs.begin(); oi < objs.end(); ++oi )
    {
        Movable * mv = *oi;
        if ( mv ) mv->rotate(T);
    }
}


//------------------------------------------------------------------------------
void Organizer::write(OutputWrapper& out) const
{
    out.writeUInt16(objs.size());
    out.writeSoftNewLine();
    for ( MecableList::const_iterator oi = objs.begin(); oi < objs.end(); ++oi )
    {
        out.writeSoftSpace();
        if ( *oi )
            (*oi)->writeReference(out);
        else
            Object::writeNullReference(out);
    }
}


void Organizer::read(InputWrapper & in, Simul& sim)
{
    try
    {
        unsigned int nb = in.readUInt16();
        
        //std::cerr << " Organizer::read with " << nb << " objects" << std::endl;
        for ( unsigned int mi = 0; mi < nb; ++mi )
        {
            Tag tag = 0;
            Object * w = sim.readReference(in, tag);
            ///@todo check that tag corresponds to a Mecable
            grasp(static_cast<Mecable*>(w), mi);
        }
    }
    catch( Exception & e )
    {
        e << ", in Organizer::read()";
        throw;
    }
}
