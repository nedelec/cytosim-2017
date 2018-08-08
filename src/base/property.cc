// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "property.h"
#include "property_list.h"
#include "glossary.h"
#include "tokenizer.h"
#include "stream_func.h"
#include <sstream>
#include <fstream>

//------------------------------------------------------------------------------

Property::Property(const std::string& n, const int ix) : mName(n), mIndex(ix)
{
    //std::cerr << "new Property `" << mName << "'" << std::endl;
}


Property::~Property()
{
    //std::cerr << "del Property `" << mName << "'" << std::endl;
}


//------------------------------------------------------------------------------
/**
 parse string \a str to set values of the property.
 
 the stamp feature limits parsing to only one pass:
 - parsing is done only if \a stamp is not found.
 - after parsing, \a stamp is added at the end of the string
 .
 If \a stamp is zero, parsing is always done.
/
*/
void Property::readString(std::string& str, const char stamp)
{
    if ( str.size() <= 0 )
        return;

    // only read if stamp is not already present:
    if ( stamp  &&  str[str.size()-1] == stamp )
        return;
    
    Glossary glos;
    try
    {
        glos.read(str);
        read(glos);
    }
    catch ( Exception& )
    {
        if ( stamp )
            str.push_back(stamp);
        throw;
    }
    
    if ( stamp )
        str.push_back(stamp);
}


void Property::readFile(char const* file)
{
    Glossary glos;
    std::ifstream is(file);
    glos.read(is);
    read(glos);
}


//------------------------------------------------------------------------------

void Property::write_diff(std::ostream & os, const Property* def) const
{
    if ( def )
    {
        std::stringstream val, ref;
        def->write_data(ref);
        write_data(val);
        StreamFunc::diff_stream(os, val, ref);
    }
    else
        write_data(os);
}


void Property::write_diff(std::ostream & os, const bool prune) const
{
    if ( prune )
    {
        Property * def = clone();
        if ( def )
        {
            def->clear();
            write_diff(os, def);
            delete(def);
            return;
        }
    }
    write_data(os);
}


bool Property::modified() const
{
    std::ostringstream ssr;
    
    Property * def = clone();
    if ( def )
    {
        def->clear();
        def->write_data(ssr);
        std::string str = ssr.str();
        delete(def);
        ssr.str("");
        write_data(ssr);
        return str.compare(ssr.str());
    }
    return true;
}


//------------------------------------------------------------------------------

/**
 This writes:
 @code
 set kind [index] name 
 {
   key = values
   ...
 }
 @endcode
 */
void Property::write(std::ostream & os, const bool prune) const
{
    os << "set " << kind();
    if ( index() >= 0 )
        os << " " << index();
    os << " " << mName << std::endl;
    os << "{" << std::endl;
    write_diff(os, prune);
    os << "}" << std::endl;
}

std::ostream& operator << (std::ostream& os, const Property& p)
{
    p.write(os, 0);
    return os;
}


