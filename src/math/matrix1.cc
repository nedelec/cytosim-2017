// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "matrix1.h"
#include "random.h"


real Matrix1::rotationAngle() const
{
    if ( val[0] > 0 )
        return 0;
    else
        return M_PI;
}


Matrix1 Matrix1::rotationFromEulerAngles(const real x)
{
    Matrix1 res;
    res.val[0] = ( cos(x) > 0 ) ? 1 : -1;
    return res;
}



Matrix1 Matrix1::randomRotation(Random& rng)
{
    Matrix1 res;
    res.val[0] = rng.sflip();
    return res;
}


Matrix1 Matrix1::rotationToVector(const real vec[1])
{
    Matrix1 res;
    if ( vec[0] > 0 )
        res.val[0] = 1;
    else
        res.val[0] = -1;
    return res;
}

