// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#include "vector1.h"

/**
 Read two scalar values equal to zero
 */
void eatTwoZeros(std::istream& is)
{
    if ( is.good() )
    {
        std::streampos isp = is.tellg();
        real Z;
        is >> Z;
        if ( !is.fail() && Z==0 )
        {
            isp = is.tellg();
            is >> Z;
            if ( !is.fail() && Z==0 )
                return;
        }
        // restore initial state:
        is.clear();
        is.seekg(isp);
    }
}


std::istream & operator >> ( std::istream& is, Vector1& v)
{
    is >> v.XX;
    eatTwoZeros(is);
    return is;
}


std::ostream & operator << ( std::ostream& os, Vector1 const& v)
{
    os << v.XX;
    return os;
}


const Vector1 interpolate(const Vector1& a, real x, const Vector1& b)
{
    return Vector1(a.XX+x*b.XX, 0);
}


real distanceSqr(const Vector1& a, const Vector1& b)
{
    return (a.XX-b.XX)*(a.XX-b.XX);
}


real distance(const Vector1& a, const Vector1& b)
{
    return fabs(a.XX-b.XX);
}

