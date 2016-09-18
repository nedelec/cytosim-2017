// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "field_base.h"
#include "field_values.h"


/// type of Field used in Cytosim
typedef FieldBase<FieldScalar> Field;


/// initialize diffusion matrix (only for FieldScalar)
template < > 
void FieldBase<FieldScalar>::prepare();

/// diffusion step
template < > 
void FieldBase<FieldScalar>::step(FiberSet&, real);

