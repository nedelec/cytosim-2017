// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#include "vector3.h"



std::istream & operator >> ( std::istream& is, Vector3& v)
{
    return is >> v.XX >> v.YY >> v.ZZ;
}


std::ostream & operator << ( std::ostream& os, Vector3 const& v)
{
    std::streamsize w = os.width();
    os << v.XX << " ";
    os.width(w);
    os << v.YY << " ";
    os.width(w);
    os << v.ZZ;
    return os;
}


const Vector3 interpolate(const Vector3& a, real x, const Vector3& b)
{
    return Vector3(a.XX+x*b.XX, a.YY+x*b.YY, a.ZZ+x*b.ZZ);
}


real distanceSqr(const Vector3& a, const Vector3& b)
{
    return (a.XX-b.XX)*(a.XX-b.XX) + (a.YY-b.YY)*(a.YY-b.YY) + (a.ZZ-b.ZZ)*(a.ZZ-b.ZZ);
}


real distance(const Vector3& a, const Vector3& b)
{
    return sqrt((a.XX-b.XX)*(a.XX-b.XX) + (a.YY-b.YY)*(a.YY-b.YY) + (a.ZZ-b.ZZ)*(a.ZZ-b.ZZ));
}
