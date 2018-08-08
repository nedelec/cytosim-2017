// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "object.h"
#include "iowrapper.h"
#include "exceptions.h"
#include "property.h"
#include "sim.h"


//------------------------------------------------------------------------------
/**
 The ASCII reference has the format XP:N or XP:N:M, where:
 - X=tag is one ascii character.
 - P=pi is the index of the property (P>=0).
 - N=nb is a serial-number (N>0).
 - M=mk is an integer (the mark is added only if it is non-zero)
 .
 
 For example 'f0:21' is the fiber of property 0, number 21
*/ 
std::string Object::strReference(char tag, int pi, Number nb, int mk)
{
    assert_true( pi >= 0 );
    char tmp[32];
    if ( mk == 0 )
        snprintf(tmp, sizeof(tmp), "%c%i:%lu", tag, pi, nb);
    else
        snprintf(tmp, sizeof(tmp), "%c%i:%lu:%i", tag, pi, nb, mk);
    return std::string(tmp);
}


//------------------------------------------------------------------------------
/**
 The ASCII reference has the format XP:N or XP:N:M, where:
 - X=tag() is one ascii character.
 - P=property->index() is the index of the property (P>=0).
 - N=number() is a serial-number (N>0).
 - M=mark() is an integer (the mark is added only if it is not zero)
 .
 
 For example 'f0:21' is the fiber of property 0, number 21
 */ 
std::string Object::reference() const
{
    return strReference(tag(), property()->index(), number(), mark());
}

//------------------------------------------------------------------------------
/**
 Two binary formats are used:
 - A short format:
     - 1 byte for the tag()
     - 1 byte for the index of the property
     - 2 bytes for the Number
     .
 - A long format:
     - the character '$'
     - 1 byte for the tag()
     - 2 bytes for the index of the property
     - 4 bytes for the Number
     - 4 bytes for the mark
     .
 .
 There is only one ascii based format, as returned by reference().
 All formats are read by ObjectSet::readReference()
 */
void Object::writeReference(OutputWrapper & out) const
{
    writeReference(out, tag());
}

/**
 Same as writeReference(OutputWrapper&), excepts that tag() is replaced by 'g' given as argument.
 */
void Object::writeReference(OutputWrapper & out, Tag g) const
{
    assert_true( number() > 0 );
    assert_true( property() );
    assert_true( property()->index() >= 0 );
    
    if ( number() <= 65535  &&  property()->index() <= 255  &&  mark() == 0 )
    {
        // short format
        out.write(g);
        out.writeUInt8(property()->index(), 0);
        out.writeUInt16(number(), ':');
    }
    else
    {
        // long format with a pretag = '$'
        out.write('$');
        out.write(g);
        out.writeUInt16(property()->index(), 0);
        out.writeUInt32(number(), ':');
        out.writeInt32(mark(), ':');
    }
}

//------------------------------------------------------------------------------
void Object::writeNullReference(OutputWrapper & out)
{
    out.write(TAG);
}

/**
 This must be able to read the formats written by Object::writeReference()
 */
void Object::readReference(InputWrapper& in, int& pi, Number& nb, int& mk, const char pretag)
{
    pi = 0; 
    nb = 0;
    mk = 0;

    if ( pretag == '$' )
    {
        // long format
        pi = in.readUInt16();
        if ( ! in.binary() )
        {
            if ( in.getUL() != ':' )
                throw InvalidSyntax("missing ':'");
        }
        nb = in.readUInt32();
#ifdef BACKWARD_COMPATIBILITY
        if ( in.formatID() < 34 )
            return;
#endif
        if ( ! in.binary() )
        {
            if ( in.getUL() != ':' )
                throw InvalidSyntax("missing ':'");
        }
#ifdef BACKWARD_COMPATIBILITY
        if ( in.formatID() < 39 )
            mk = in.readUInt16();
        else
#endif
        mk = in.readInt32();
    }
    else
    {
        // short format
        pi = in.readUInt8();
        if ( ! in.binary() )
        {
            if ( in.getUL() != ':' )
                throw InvalidSyntax("missing ':'");
        }
        nb = in.readUInt16();
    }
}


