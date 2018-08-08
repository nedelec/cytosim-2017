// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef FIELD_H
#define FIELD_H

#include "dim.h"
#include "real.h"
#include "grid.h"
#include "space.h"
#include "object.h"
#include "iowrapper.h"
#include "messages.h"
#include "exceptions.h"
#include "matsparsesym1.h"
#include "field_prop.h"
class FiberSet;

#ifdef DISPLAY
   #include "gle.h"
#endif

/// value of type VAL defined as a function of position over the simulation Space
/**
The field is build on a Grid with square cells, because diffusion/reaction are then
 easier to implement. The Grid does not necessarily match the edges of the Space exactly,
 but instead extends outside, such as to cover the Space::inside entirely.
 
 Each cell holds the amount of molecules.
 The local concentration can be obtained by dividing by the cell volume:
 @code
 concentration = FieldGrid::cell(position) / FieldGrid::cellVolume();
 @endcode
 */
template < typename VAL >
class FieldBase : public Grid<DIM, VAL, unsigned int>, public Object
{
public:
    
    /// the type of Grid from which the Field is derived
    typedef Grid<DIM, VAL, unsigned int> FieldGrid;
    
    /// property
    FieldProp const* prop;
    
private:
    
    /// disabled default constructor
    FieldBase();

    /// duplicate field
    VAL*     fiMirror;
    
    /// allocated size of fiMirror
    unsigned fiMirrorSize;
    
    /// matrix for diffusion
    MatrixSparseSymmetric1 fiDiffusionMatrix;
    
    
    /// initialize the field
    void createGrid()
    {
        assert_true( FieldGrid::hasDimensions() );
        // delete preexisting grid if necessary:
        FieldGrid::destroy();
        // create the grid using the calculated dimensions:    
        FieldGrid::createCells();
        // set all values to zero:
        FieldGrid::clear();
    }
    
    /// initialize to cover the given Space with squares of size 'step'
    void setGrid(Vector& range)
    {
        assert_true( prop && prop->step > REAL_EPSILON );
        // we add a safety border (in micro-meters)
        const real border = 1;
        
        int size[3] = { 0, 0, 0 };
        // we use square cells:
        for ( int d = 0; d < DIM; ++d )
        {
            size[d] = (int)(ceil( 2*(range[d]+border) / prop->step ));
            range[d] = 0.5 * prop->step * size[d];
        }
        
        FieldGrid::setDimensions(-range, range, size);
        createGrid();
        
        //verify the cell size:
        for ( int d = 0; d < DIM; ++d )
        {
            real dif = FieldGrid::delta(d)*prop->step - 1.0;
            if ( fabs(dif) > 1e-6 )
            {
                Cytosim::warning("Field:step[%i] is not as expected:\n", d);
                Cytosim::warning("  field: %f  prop: %f\n", FieldGrid::cellWidth(d), prop->step);
            }
        }            
    }
    
public:
    #pragma mark -
    
    /// constructor
    FieldBase(FieldProp const* p)
    {
        prop=p;
        fiMirror=0;
        fiMirrorSize=0;
    }
    
    /// destructor
    ~FieldBase()
    {
        if ( fiMirror )
            delete[] fiMirror;
    }
    
    /// initialize with squares of size 'step'
    void setField()
    {
        assert_true( prop );

        if ( ! FieldGrid::hasCells() )
        {            
            if ( prop->confine_space_ptr == 0 )
                throw InvalidParameter("A space must be defined to set a field");
            
            Vector range = prop->confine_space_ptr->extension();
            setGrid(range);
            
            //std::cerr << "setField() step="<< prop->step<< " nCells="<< FieldGrid::nbCells()<<std::endl;
            Cytosim::MSG(4, "Field %lx set with %i cells of size %.3f um\n", this, FieldGrid::nbCells(), prop->step);
        }
    }
    
        
    /// true if field was set
    bool hasField()  const { return FieldGrid::hasCells(); }
    
    /// size of cell
    real cellWidth() const { return FieldGrid::cellWidth(0); }
    
    //------------------------------ simulation --------------------------------
    #pragma mark -

    /// set all cells to value = volume * conc
    void setConcentration(real conc)
    {
        FieldGrid::setValues( conc * FieldGrid::cellVolume() );
    }
    
    /// total value
    real sumValues() { return FieldGrid::sumValues(); }
    
    /// simulation step 
    void step(FiberSet&, real) {}
    
    /// initialize diffusion matrix (only for FieldScalar)
    void prepare() {}
    
    //------------------------------ read/write --------------------------------
    #pragma mark -

    /// a unique character identifying the class
    static const Tag TAG = 'i';
    
    /// return unique character identifying the class
    Tag    tag() const { return TAG; }
    
    /// return index of 'prop' in corresponding PropertyList
    const Property* property() const { return prop; }
    
    /// write Field to file using VAL::write()
    /** Some of this should be moved to Grid */
    void   write(OutputWrapper& out) const
    {
        if ( FieldGrid::hasCells() && prop->save )
        {        
            out.writeUInt16(DIM);
            for ( int d = 0; d < DIM; ++d )
            {
                out.writeSoftSpace();
                out.writeUInt32(FieldGrid::dim(d));
                out.writeFloat(FieldGrid::inf(d));
                out.writeFloat(FieldGrid::sup(d));
            }
            out.writeSoftSpace();
            out.writeUInt32(FieldGrid::nbCells());
            for ( typename FieldGrid::index_type c = 0; c < FieldGrid::nbCells(); ++c ) 
                FieldGrid::cell(c).write(out);
            out.writeSoftNewLine();
        }
        
        if ( prop->positive )
        {
            if ( FieldGrid::hasNegativeValue() )
                throw Exception("Aborting because Field has negative values");
        }
    }
    
    
    /// read Field from file using VAL::read()
    void   read_data(InputWrapper& in, Simul&)
    {
        int  size[DIM] = { 0 };
        real minB[DIM] = { 0 }, maxB[DIM] = { 0 };
        
        try {
            unsigned int dim = in.readUInt16();
            if ( dim != DIM )
                throw InvalidIO("field::dimensionality missmatch");
            
            for ( unsigned int d = 0; d < dim; ++d )
            {
                size[d] = in.readUInt32();
                minB[d] = in.readFloat();
                maxB[d] = in.readFloat();
            }
            
            FieldGrid::setDimensions(minB, maxB, size);
            createGrid();
            
            typename FieldGrid::index_type nbc = in.readUInt32();
            if ( nbc != FieldGrid::nbCells() )
            {
                printf("file: %u field:%u\n", nbc, FieldGrid::nbCells());
                throw InvalidIO("missmatch in Field::size");
            }
            //std::cerr << "Field::read() nb_cells=" << nbc << std::endl;

            for ( typename FieldGrid::index_type c = 0; c < nbc; ++c ) 
                FieldGrid::cell(c).read(in);
        }
        catch( Exception & e ) {
            e << ", in Field::read()";
            throw;
        }
    }
    
    /// read Field and checks that the Grid::step has not changed
    void   read(InputWrapper& in, Simul& sim)
    {
        read_data(in, sim);
        
        if ( prop )
        {
            for ( unsigned int d = 0; d < DIM; ++d )
            {
                real dif = FieldGrid::delta(d)*prop->step - 1.0;
                if ( fabs(dif) > 1e-6 )
                {
                    Cytosim::warning("Field:step[%i] has changed:\n", d);
                    Cytosim::warning("  file: %f  prop: %f\n", FieldGrid::cellWidth(d), prop->step);
                }
            }
            
            /*
             we should extrapolate the data that were read to a grid with the
             resolution specified by prop->step
             */
            
        }
    }
    
    //------------------------------- display ----------------------------------

#ifdef DISPLAY
    
    /// openGL display function
    /**
     display all cells for which mask(center) == true
     */
    bool display(real max_value, bool inside) const
    {
        if ( !FieldGrid::hasCells() )
            return false;
        
        glPushAttrib(GL_ENABLE_BIT|GL_POLYGON_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
        Vector dx(0.5*FieldGrid::cellWidth(0), 0, 0);
        Vector dy(0, 0.5*FieldGrid::cellWidth(1), 0);
        
#if ( DIM > 1 )
        Vector dmm = -dx - dy;
        Vector dpm =  dx - dy;
        Vector dpp =  dx + dy;
        Vector dmp = -dx + dy;
        const Space * spc = prop->confine_space_ptr;
        real amp = 1.0 / ( max_value * FieldGrid::cellVolume() );
#endif
#if ( DIM == 2 )
        // paint all cells one by one
        for ( typename FieldGrid::index_type c = 0; c < FieldGrid::nbCells(); ++c )
        {
            Vector w;
            FieldGrid::setPositionFromIndex(w, c, 0.5);
            if ( !inside || spc->inside(w) )
            {
                FieldGrid::cell(c).setColor(amp);
                glBegin(GL_TRIANGLE_STRIP);
                gle::gleVertex(w+dmm);
                gle::gleVertex(w+dpm);
                gle::gleVertex(w+dmp);
                gle::gleVertex(w+dpp);
                glEnd();
            }
        }
#elif ( DIM == 3 )
        // paint only one XY-plane in the middle of the grid
        typename FieldGrid::index_type z = FieldGrid::nbCells(2) / 2;
        for ( typename FieldGrid::index_type x = 0; x < FieldGrid::nbCells(0); ++x )
        for ( typename FieldGrid::index_type y = 0; y < FieldGrid::nbCells(1); ++y )
        {
            Vector w;
            int coord[] = { x, y, z };
            FieldGrid::setPositionFromCoordinates(w, coord, 0.5);
            if ( !inside || spc->inside(w) )
            {
                FieldGrid::cell(coord).setColor(amp);
                glBegin(GL_TRIANGLE_STRIP);
                gle::gleVertex(w+dmm);
                gle::gleVertex(w+dpm);
                gle::gleVertex(w+dmp);
                gle::gleVertex(w+dpp);
                glEnd();
            }
        }
#endif
        glPopAttrib();
        if ( 0 ) 
        {
            glPushAttrib(GL_LIGHTING_BIT);
            glDisable(GL_LIGHTING);
            glColor4f(1, 0, 1, 1);
            glLineWidth(0.5);
            drawEdges(*this);
            glPopAttrib();
        }
        
        return true;
    }
    
#endif
};


#endif
