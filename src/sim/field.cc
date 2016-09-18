// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Created by Francois Nedelec on 18/12/07.

#include "field.h"
#include "fiber_binder.h"
#include "fiber_set.h"
#include "cblas.h"
#include "sim.h"

extern Random RNG;

template < > 
void FieldBase<FieldScalar>::prepare()
{
    const Space * spc = prop->confine_space_ptr;
    if ( spc == 0 )
        throw InvalidParameter("A space must be defined to prepare a field");

    const unsigned int nbc = FieldGrid::nbCells();
    assert_true( nbc > 0 );
    
    if ( fiMirror )
        delete [] fiMirror;    
    fiMirror = new FieldScalar[nbc];
    fiMirrorSize = nbc;
    
    fiDiffusionMatrix.allocate(nbc);
    fiDiffusionMatrix.makeZero();
    
    // find out which cell is inside the space:
    Vector pos(0,0,0);
    unsigned char * inside = new unsigned char[nbc];
#if ( 1 )
    for ( unsigned int c = 0; c < nbc; ++c )
    {
        setPositionFromIndex(pos, c, 0.5);
        inside[c] = spc->inside(pos);
    }
#else
    // extended covered area:
    const real range = 2 * cellWidth();
    for ( unsigned int c = 0; c < nbc; ++c )
    {
        setPositionFromIndex(pos, c, 0.5);
        inside[c] = ! spc->allOutside(pos, range);
    }
#endif
    // build an array of offsets to right-side neighbor in each dimension
    unsigned int off = 1, offset[DIM];
    for ( int d = 0; d < DIM; ++d )
    {
        offset[d] = off;
        off *= FieldGrid::nbCells(d);
    }
    
    // build the matrix for diffusion,
    // keeping only cells which are inside the space:
    for ( unsigned int c = 0; c < nbc; ++c )
    {
        if ( inside[c] )
        {
            for ( int d = 0; d < DIM; ++d )
            {
                unsigned int oc = c + offset[d];
                if ( oc < nbc  &&  inside[oc] )
                {
                    fiDiffusionMatrix(c,  oc) += 1.0;
                    fiDiffusionMatrix(c,  c ) -= 1.0;
                    fiDiffusionMatrix(oc, oc) -= 1.0;
                }
            }
        }
    }
    delete[] inside;
    if ( 0 && nbc < 100 ) {
        std::cerr << std::endl;
        fiDiffusionMatrix.printFull(std::cerr);
    }
    fiDiffusionMatrix.prepareForMultiply();
    
    std::cerr << "Field::prepare() diffusion using ";
    std::cerr << fiDiffusionMatrix.nbNonZeroElements() << " matrix elements" << std::endl;
}


/**
 //\todo implement Crank-Nicholson for diffusion
 */
template < >
void FieldBase<FieldScalar>::step(FiberSet&, real time_step)
{
    assert_true( prop );
    
    // we cast FieldScalar to floating-point type :
    assert_true( sizeof(FieldScalar) == sizeof(real) );
    real * field = (real*)( FieldGrid::cell_addr() );
    
    if ( prop->diffusion > 0 )
    {
        assert_true( fiMirror );
        assert_true( fiMirrorSize == FieldGrid::nbCells() );
        real * dfield = (real*)( fiMirror );
        
        // dfield = field * prop->diffusion_theta:
        blas_xcopy(FieldGrid::nbCells(), field, 1, dfield, 1);
        blas_xscal(FieldGrid::nbCells(), prop->diffusion_theta, dfield, 1);
        
        
        // decay:
        if ( prop->decay_rate > 0 )
        {
            // field = field * ( 1 - decay_rate * dt ):
            blas_xscal(FieldGrid::nbCells(), 1.0 - prop->decay_rate_dt, field, 1);
        }
        
        // diffusion
        assert_true( fiDiffusionMatrix.size() == FieldGrid::nbCells() );
        
        // field = field + fiDiffusionMatrix * dfield:
        fiDiffusionMatrix.vecMulAdd(dfield, field);
    }
}

