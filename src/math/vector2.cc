// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#include "vector2.h"

/**
 Read a scalar value equal to zero
 */
void eatOneZero(std::istream& is)
{
    if ( is.good() )
    {
        std::streampos isp = is.tellg();
        real Z;
        is >> Z;
        if ( is.fail() || Z!=0 )
        {
            // restore initial state:
            is.seekg(isp);
            is.clear();
        }
    }
}


std::istream & operator >> ( std::istream& is, Vector2& v)
{
    is >> v.XX >> v.YY;
    eatOneZero(is);
    return is;
}


std::ostream & operator << ( std::ostream& os, Vector2 const& v)
{
    std::streamsize w = os.width();
    os << v.XX << " ";
    os.width(w);
    os << v.YY;
    return os;
}


const Vector2 interpolate(const Vector2& a, real x, const Vector2& b)
{
    return Vector2(a.XX+x*b.XX, a.YY+x*b.YY);
}


real distanceSqr(const Vector2& a, const Vector2& b)
{
    return (a.XX-b.XX)*(a.XX-b.XX) + (a.YY-b.YY)*(a.YY-b.YY);
}


real distance(const Vector2& a, const Vector2& b)
{
    return sqrt((a.XX-b.XX)*(a.XX-b.XX) + (a.YY-b.YY)*(a.YY-b.YY));
}

