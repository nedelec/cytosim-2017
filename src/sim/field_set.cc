// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "field_set.h"
#include "field_prop.h"
#include "iowrapper.h"
#include "glossary.h"
#include "simul.h"

//------------------------------------------------------------------------------

void FieldSet::prepare()
{
    for ( Field * f=first(); f; f=static_cast<Field*>(f->next()) )
    {
        assert_true( f->hasField() );
        f->prepare();
    }
}


void FieldSet::step()
{
    for ( Field * f=first(); f; f=static_cast<Field*>(f->next()) )
    {
        if ( f->hasField() )
        {
            MSG_ONCE("!!!! Field is active\n");
            f->step(simul.fibers, simul.prop->time_step);
        }
    }
}

//------------------------------------------------------------------------------
#pragma mark -

Property* FieldSet::newProperty(const std::string& kd, const std::string& nm, Glossary&) const
{
    if ( kd == kind() )
        return new FieldProp(nm);
    return 0;
}

//------------------------------------------------------------------------------
Object * FieldSet::newObjectT(const Tag tag, int idx)
{
    Field * obj = 0;
    if ( tag == Field::TAG )
    {
        Property * p = simul.properties.find_or_die(kind(), idx);
        obj = new Field(static_cast<FieldProp*>(p));
        //the field is not initialized, because it should be set by FieldBase::read
    }
    return obj;
}


/**
 @defgroup NewField How to create a Field
 @ingroup NewObject

 Specify the initial value of the Field:
 
 @code
 new field NAME
 {
    value = 0
 }
 @endcode
 
 \todo: read the value of the field from a file, at initialization
 */
ObjectList FieldSet::newObjects(const std::string& kd, const std::string& nm, Glossary& opt)
{
    Field * obj = 0;
    if ( kd == kind() )
    {
        Property * p = simul.properties.find_or_die(kd, nm);
        FieldProp * fp = static_cast<FieldProp*>(p);
        
        obj = new Field(fp);
        
        // initialize field:
        obj->setField();
        
        // an initial concentration can be specified:
        real val = 0;
        if ( opt.set(val, "value") || opt.set(val, "initial_value") )
            obj->setConcentration(val);
    }
    
    ObjectList res;
    if ( obj )
        res.push_back(obj);
    
    return res;

}


//------------------------------------------------------------------------------
void FieldSet::add(Object * obj)
{
    assert_true(obj->tag() == Field::TAG);
    ObjectSet::add(obj);
}

void FieldSet::remove(Object * obj)
{
    assert_true(obj->tag() == Field::TAG);
    ObjectSet::remove(obj);
}

