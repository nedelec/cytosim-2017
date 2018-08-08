// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "sim.h"
#include <fstream>
#include "tictoc.h"
#include "iowrapper.h"
#include "messages.h"

/// Current format version number used for writing object-files.
/**
 This is related to InputWrapper::formatID()
 */
const int currentFormatID = 41;

//History of changes of currentFormatID:

/*
     08/12/2012 FRAME_TAG was changed from "#frame " to "#Cytosim "
 41: 17/11/2012 Space stores its shape in objects.cmo
 40: 11/9/2012  Aster format simplified
 39: 11/7/2012  Object::mark is stored on 32 bits instead of 16 previously
 38: 3/05/2012  FiberNaked stores length instead of segment-length
 37: 30/04/2012 Couple::Duo stores its activity state
 36: 22/02/2012 All Spaces store their dimensions in objects.cmo
 35: 15/09/2011 Some Spaces stores their dimensions in objects.cmo  
 34: 20/12/2010 Moved Fiber::mark to Object::mark
 33: 29/04/2010 Added Fiber::mark
 32: 15/04/2010 Space became an Object
 31: 01/04/2010 Fiber became a Mecable
 30: The Tag were reduced to 1 char: saves 20% space & simplifies code
     26/05/2009: started cytosim-PI: Power to the user!
 27: 22/03/2008 new Fiber::write(), called in Tubule::write()
 26: 03/11/2007 Hand do not record haEnd flag
 24: 14/12/2006 started cytosim 3, lots of changes
 23: 10/12/2005 new Solid
 22: modified Sphere
 21: modified Sphere
 20: 12/07/2004
 19: introduced different kind of Single
*/

//============================================================================
//=======================      INPUT FROM A FILE     =========================
//============================================================================
//------------------------------------------------------------------------------

/**
 We do not allow property()->index() of an Object to change during import from a file.
 However, there is no structural reason that prevent this in the code.
 If necessary, it should be possible to remove this limitation.
 
 The Object is not modified
 */
Object * Simul::readReference(InputWrapper & in, char & tag)
{
    tag = in.getUL();

    if ( !in.binary() )
    {
        while ( isspace(tag) )
            tag = in.getUL();
    }
    
    // Object::TAG is the 'void' reference
    if ( tag == Object::TAG )
        return 0;
    
#ifdef BACKWARD_COMPATIBILITY
    if ( in.formatID() < 32 )
    {
        Number n = isupper(tag) ? in.readUInt32() : in.readUInt16();
        if ( n == 0 )
            return 0;
        Object * w = findObject(tolower(tag), n);
        if ( w == 0 )
            throw InvalidIO("Unknown object referenced (old style)");
        return w;
    }
#endif
    
    char pretag = 0;
    if ( tag == '$' )
    {
        pretag = tag;
        tag = in.getUL();
    }

    const ObjectSet * set = findSet(tag);
    
    if ( set == 0 )
        throw InvalidIO("`"+std::string(1,tag)+"' is not a recognized class tag");
    
    int ix = 0, mk = 0;
    Number nb = 0;
    
    Object::readReference(in, ix, nb, mk, pretag);
    
    if ( nb == 0 )
        return 0;
    
    Object * res = set->find(nb);
    
    if ( res == 0 )
        throw InvalidIO("Unknown object referenced");
    
    assert_true( res->number() == nb );
    
    if ( res->property() == 0 || res->property()->index() != ix )
        throw InvalidIO("The property of a `"+res->property()->kind()+"' should not change!");
    
    return res;
}


//------------------------------------------------------------------------------

int Simul::loadObjects(InputWrapper & in)
{
    if ( in.eof() )
        return 1;
    
    if ( ! in.good() )
        throw InvalidIO("invalid file in Simul::loadObjects()");
    
    int res = 0;
    
    in.lock();
    try
    {
        res = readObjects(in);
        //std::clog << "loadObjects returns " << res << std::endl;
    }
    catch(Exception & e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        in.unlock();
        throw;
    }
    
    in.unlock();
    return res;
}

/**
 Before reading, all objects are transfered to a secondary list called 'ice'.
 Every object encountered in the file is updated, and then transferred back to the normal list.
 
 When the read is complete, the objects that are still on 'ice' are deleted.
 In this way the new state reflects exactly the system that was read from file.
 
 @returns
 - 0 = success
 - 1 = EOF
 */

int Simul::reloadObjects(InputWrapper & in)
{
    if ( in.eof() )
        return 1;
    
    if ( ! in.good() )
        throw InvalidIO("Simul::read(): invalid file");
 
    couples.freeze();
    singles.freeze();
    fibers.freeze();
    beads.freeze();
    solids.freeze();
    spheres.freeze();
    organizers.freeze();
    fields.freeze();
    spaces.freeze();
    
    in.lock();
    
    // flag to erase the older objects
    bool erase = true;
    
    try
    {
        if ( readObjects(in) == 0 )
            erase = false;
        
        in.unlock();
        
        /*
         Attention: The order of the thaw() below is important:
         destroying a Fiber will detach any motor attached to it,
         and thus automatically move them to the 'unattached' list,
         as if they had been updated from reading the file.
         Destroying couples and singles before the fibers avoid this problem.
         */
        organizers.thaw(erase);
        couples.thaw(erase);
        singles.thaw(erase);
        beads.thaw(erase);
        solids.thaw(erase);
        spheres.thaw(erase);
        fibers.thaw(erase);
        spaces.thaw(erase);
        fields.thaw(erase);
    }
    catch(Exception & e)
    {
        in.unlock();
        std::cerr << "Error: " << e.what() << std::endl;
        
        /*
         Attention: The order of the thaw() below is important:
         destroying a Fiber will detach any motor attached to it,
         and thus automatically move them to the 'unattached' list,
         as if they had been updated from reading the file.
         Destroying couples and singles before the fibers avoid this problem.
         */
        organizers.thaw(erase);
        couples.thaw(erase);
        singles.thaw(erase);
        beads.thaw(erase);
        solids.thaw(erase);
        spheres.thaw(erase);
        fibers.thaw(erase);
        spaces.thaw(erase);
        fields.thaw(erase);
        throw;
    }
    
    return in.eof();
}

//------------------------------------------------------------------------------

/**
 
 @returns
 0 : no sign of a cytosim frame was found
 1 : a frame starting tag (FRAME_TAG) was found, but not the end
 2 : the frame starting and end tags were found
 */
int Simul::readObjects(InputWrapper & in)
{
    int res = 0;
    char c = '\n', tag, pretag;
    std::string line;
    
    while ( in.good() )
    {
        // read the next tag
        tag = in.getUL();
        while ( isspace(tag) )
        {
            c = tag;
            tag = in.getUL();
        }
        
        if ( in.eof() ) break;
        
        if ( c != '\n' )
        {
            in.get_line(line);
            c = '\n';
            std::cerr << " skipped `" << line;
            continue;
        }
        
#ifdef BACKWARD_COMPATIBILITY
        // Compatibility with older format
        if ( in.formatID() < 32 )
        {
            ObjectSet * set = findSet(tolower(tag));
            if ( set )
            {
                Number n = isupper(tag) ? in.readUInt32() : in.readUInt16();
                if ( n == 0 )
                    throw InvalidIO("invalid (null) Object reference");
                Object * w = set->find(n);
                int pi = 0;
                if ( w ) {
                    if ( tag!='i'  &&  ( tag!='m' || in.formatID()!=31 ))
                        pi = in.readUInt16();
                    w->read(in, *this);
                    set->relink(w);
                }
                else {
                    if ( tag!='i'  &&  ( tag!='m' || in.formatID()!=31 ))
                        pi = in.readUInt16();
                    w = set->newObjectT(tolower(tag), pi);
                    w->number(n);
                    w->read(in, *this);
                    set->add(w);
                }
                continue;
            }
        }
#endif
        
        if ( tag == '$' )
        {
            pretag = tag;
            tag = in.getUL();
        }
        else
            pretag = 0;
        
        // find the corresponding ObjectSet:
        ObjectSet * set = findSet(tag);
        
        if ( set )
        {
            set->readObject(in, tag, pretag);
            continue;
        }

        //check for meta-data, contained in lines starting with '#'
        if ( tag == '#' )
        {
            in.get_line(line);

            //detect frame start
            if ( 0 == line.compare(0, sizeof(FRAME_TAG)-2, 1+FRAME_TAG) )
            {
                res = 1;
                continue;
            }
            
#ifdef BACKWARD_COMPATIBILITY
            //detect frame start
            if ( 0 == line.compare(0, 6, "frame ") )
            {
                res = 1;
                continue;
            }
#endif
                        
            //detect frame sections
            if ( 0 == line.compare(0, 8, "section ") )
            {
                //std::cerr << "at " << line << std::endl;
#if ( 0 )
                if ( 0 == line.compare(8, 6, "couple") )
                {
                    in.skip_until("#section ");
                    //std::cerr << "skipped " << line << std::endl;
                }
#endif
                continue;
            }
            

            //detect info line
            if ( 0 == line.compare(0, 5, "time ") )
            {
                int f = currentFormatID, d = 3;
                double t = simTime();
                
                sscanf(line.c_str(), "time %lf, dim %i, format %i", &t, &d, &f);
                
                // set time of simulation:
                setTime(t);
                
                in.inputDIM(d);
                if ( d != DIM )
                    Cytosim::warning("Mismatch between file (%iD) and executable (%iD)\n", d, DIM);

                in.formatID(f);
                continue;
            }
            
            //binary signature
            if ( 0 == line.compare(0, 7, "binary ") )
            {
                in.setBinarySwap(line.substr(7).c_str());
                continue;
            }
           
            //detect the mark at the end of the frame
            if ( 0 == line.compare(0, 10, "end frame ") )
                return 2;
            
            //detect the mark at the end of the frame
            if ( 0 == line.compare(0, 12, "end cytosim ") )
                return 2;
            
            continue;
        }
        else {
            //finally, we just skip the line
            in.get_line(line);
            std::cerr << "skipped `" << tag << line << std::endl;
        }
    }
    return res;
}


//=============================================================================
//==========================   OUTPUT TO A FILE   =============================
//=============================================================================
#pragma mark -


void Simul::writeObjects(OutputWrapper & out) const
{
    if ( ! out.good() )
        throw InvalidIO("output file is invalid");
    
    //std::cerr << "Writing frame " << frame_index << std::endl;

    char date[26] = { 0 };
    TicToc::date(date, sizeof(date));
    
    // lock file:
    out.lock();
    // write a line identifying a new frame:
    fprintf(out, "\n\n%s %s", FRAME_TAG, date);
    
    // record the simulated time:
    fprintf(out, "\n#time %.6f, dim %i, format %i", simTime(), DIM, currentFormatID);
    
    // record a signature to identify binary file, and endianess:
    if ( out.binary() )
        out.writeBinarySignature("\n#binary ");
    
    /*
     An object should be written after any other objects that it refers to.
     For example, Aster is written after Fiber, Couple after Fiber...
     This makes it easier to reconstruct the state during input.
     */
    
    if ( spaces.size() )
    {
        out.put_line("\n#section space");
        spaces.write(out);
    }
    
    if ( fields.size() )
    {
        out.put_line("\n#section field");
        fields.write(out);
    }
    
    if ( fibers.size() )
    {
        out.put_line("\n#section fiber");
        fibers.write(out);
    }
    
    if ( solids.size() )
    {
        out.put_line("\n#section solid");
        solids.write(out);
    }
    
    if ( beads.size() )
    {
        out.put_line("\n#section bead");
        beads.write(out);
    }
    
    if ( spheres.size() )
    {
        out.put_line("\n#section sphere");
        spheres.write(out);
    }
    
    if ( singles.size() )
    {
        out.put_line("\n#section single");
        singles.write(out);
    }
    
    if ( couples.size() )
    {
        out.put_line("\n#section couple");
        couples.write(out);
    }
    
    if ( organizers.size() )
    {
        out.put_line("\n#section organizer");
        organizers.write(out);
    }
    
    out.put_line("\n#section end");
    fprintf(out, "\n#end cytosim %s\n\n", date);
    out.unlock();
}



/**
 This appends the current state to a trajectory file.
 Normally, this is objects.cmo in the current directory.
 
 If the file does not exist, it is created de novo.
*/
void Simul::writeObjects(std::string const& file, bool binary, bool append) const
{
    try {

        OutputWrapper out(file.c_str(), append, binary);
        writeObjects(out);
        
    }
    catch( InvalidIO & e ) {
        std::cerr << "Error writing trajectory file `"<< file <<"':" << e.what() << std::endl;
    }
}


//------------------------------------------------------------------------------
#pragma mark -


/**
 The order of the output is important, since some properties 
 depend on others (eg. SingleProp and CoupleProp use HandProp). 
 Luckily, there is no circular dependency in Cytosim at the moment.
 
 Thus we simply follow the order in which properties were defined,
 and which is the order in which properties appear in the PropertyList.
 */

void Simul::writeProperties(std::ostream& os, const bool prune) const
{
    //std::cerr << "Writing properties" << std::endl;
    os << "% Cytosim property file" << std::endl << std::endl;
    
    prop->write(os, prune);
    os << std::endl;
    properties.write(os, prune);
    os << std::endl;
}


/**
 At the first call, this will write all properties to file, 
 and save a copy of what was written to a string `properties_saved`.
 
 The next time this is called, the properties will be compared to the string,
 and the file will be rewritten only if there is a difference.
 */
void Simul::writeProperties(std::string& file, bool prune) const
{
    std::ostringstream oss;
    writeProperties(oss, prune);
    if ( oss.str() != properties_saved )
    {
        properties_saved = oss.str();
        std::ofstream os(file.c_str());
        //this should be equivalent to: writeProperties(os, prune);
        os << properties_saved << std::endl;
        os.close();
        //std::cerr << "Writing properties at frame " << frame() << std::endl;
    }
}


