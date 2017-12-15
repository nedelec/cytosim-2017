// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "assert_macro.h"
#include "stream_func.h"
#include "movable.h"
#include "exceptions.h"
#include "quaternion.h"
#include "iowrapper.h"
#include "tokenizer.h"
#include "glossary.h"
#include "modulo.h"
#include "random.h"
#include "space.h"
#include "simul.h"
extern Random RNG;

/** The default implementation is invalid */
void Movable::translate(Vector const&)
{
    ABORT_NOW("undefined/invalid translate()");
}

/**
The default implementation:
@code
 translate( w - position() );
@endcode
can be redefined in derived class for efficiency.
*/
void Movable::setPosition(Vector const& vec)
{
    assert_true( translatable() );
    translate( vec - position() );
}

/*
 if mobile() is true,
 the Object is translated by `[ rot * Object::position() - Object::position() ]`
*/
void Movable::rotate(Rotation const& rot)
{
    if ( translatable() )
    {
        Vector pos = position();
        translate(rot*pos-pos);
    }
}

/**
The default implementation:
@code
 Vector G = position();
 translate( -G );
 rotate( T );
 translate(  G ); 
@endcode
can be redefined in derived class for efficiency.
*/
void Movable::rotateP(Rotation const& T)
{
    Vector G = position();
    translate( -G );
    rotate( T );
    translate(  G );
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 Geometrical Primitives:
 
 Keyword (A, B... are real ) |   Position (X, Y, Z) / Remark
 ----------------------------|----------------------------------------------------
 `A B C`                     | The specified vector (A,B,C)
 `inside`                    | A random position inside the current Space
 `edge E`                    | At distance E from the edge of the current Space
 `surface E`                 | On the surface of the current Space\n By projecting a point at distance E from the surface.
 `line L T`                  | Random with -L/2 < X < L/2; norm(Y,Z) < T
 `sphere R T`                | At distance R +/- T/2 from the origin\n `R-T/2 < norm(X,Y,Z) < R+T/2`
 `ball R`                    | At distance R at most from the origin\n `norm(X,Y,Z) < R`
 `disc R T`                  | in 2D, a disc in the XY-plane \n in 3D, a disc in the XY-plane of thickness T in Z
 `discXZ R T`                | a disc in the XZ-plane of radius R, thickness T
 `discYZ R T`                | a disc in the YZ-plane of radius R, thickness T
 `circle R T`                | A circle of radius R and thickness T \n At distance T from the circle of radius R
 `cylinder W R`              | Cylinder of axis X, W=thickness in X, R=radius in YZ
 `ellipse A B C`             | Inside the ellipse or ellipsoid of main axes 2A, 2B and 2C
 `arc L Theta`               | A piece of circle of length L and covering an angle Theta
 `stripe L R`                | Random Vector with L < X < R
 `square R`                  | Random Vector with -R < X < R; -R < Y < R; -R < Z < R;
 `rectangle A B C`           | Random Vector with -A < X < A; -B < Y < B; -C < Z < C;
 `gradient S E`              | Provides a linear density gradient, from 0 at X=S to 1 at X=E

 Each primitive describes a certain area in Space, and in most cases the returned position is
 chosen randomly inside this area following a uniform probability.
 */

Vector Movable::readPrimitive(std::istream& is, const Space* spc)
{
    char c = Tokenizer::eat_space(is, false);

    if ( c == EOF )
        return Vector(0,0,0);

    if ( isalpha(c) )
    {
        std::string tok = Tokenizer::get_token(is);
        //std::cerr << "readPosition    `"<<tok<<"'"<<std::endl;
        
        if ( tok == "inside" || tok == "random" )
        {
            if ( spc == 0 )
                throw InvalidParameter("A space must be created first!");
            return spc->randomPlace();
        }
        
        if ( tok == "sphere" )
        {
            real R = 0, T = 0;
            is >> R >> T;
            if ( R < 0 ) throw InvalidParameter("sphere:radius must be >= 0");
            if ( T < 0 ) throw InvalidParameter("sphere:thickness must be >= 0");
            return Vector::randUnit(R) + Vector::randUnit(T*0.5);
        }
        
        if ( tok == "equator" )
        {
            real R = 0, T = 0;
            is >> R >> T;
            if ( R < 0 )
                throw InvalidParameter("you must specify a radius R >= 0 in `equator R T`");
            if ( T < 0 )
                throw InvalidParameter("the thickness T must be >= 0 in `equator R T`");
            Vector2 vec2 = Vector2::randBall();
            return Vector(R*vec2.XX, R*vec2.YY, T*0.5*RNG.sreal());
        }

        if ( tok == "cylinder" || tok == "discYZ"  )
        {
            real L = 0, R = 0;
            is >> L >> R;
            if ( L < 0 ) throw InvalidParameter("cylinder:length must be >= 0");
            if ( R < 0 ) throw InvalidParameter("cylinder:radius must be >= 0");
            Vector2 YZ = Vector2::randBall(R);
            return Vector(L*0.5*RNG.sreal(), YZ.XX, YZ.YY);
        }
        
        if ( tok == "circle" )
        {
            real R = 1, T = 0;
            is >> R >> T;
            if ( R <= 0 ) throw InvalidParameter("circle:radius must be > 0");
            if ( T <  0 ) throw InvalidParameter("circle:thickness must be >= 0");
#if ( DIM == 3 )
            Vector2 XY = Vector2::randUnit(R);
            return Vector3(XY.XX, XY.YY, 0) + (0.5*T) * Vector3::randUnit();
#endif
            return Vector::randUnit(R) + Vector::randUnit(T*0.5);
        }
        
        if ( tok == "ball" )
        {
            real R = 0;
            is >> R;
            if ( R < 0 ) throw InvalidParameter("ball:radius must be >= 0");
            return Vector::randBall(R);
        }
        
        if ( tok == "disc" || tok == "discXY" )
        {
            real R = 0, T = 0;
            is >> R >> T;
            if ( R < 0 ) throw InvalidParameter("disc:radius must be >= 0");
#if ( DIM == 3 )
            //in 3D, a disc in the XY-plane of thickness T in Z-direction
            if ( T < 0 ) throw InvalidParameter("disc:thickness must be >= 0");
            Vector2 V = Vector2::randBall(R);
            return Vector(V.XX, V.YY, T*0.5*RNG.sreal());
#endif
            //in 2D, a disc in the XY-plane
            return Vector::randBall(R);
        }
        
        if ( tok == "discXZ"  )
        {
            real R = 0, T = 0;
            is >> R >> T;
            if ( T < 0 ) throw InvalidParameter("cylinder:thickness must be >= 0");
            if ( R < 0 ) throw InvalidParameter("cylinder:radius must be >= 0");
            Vector2 V = Vector2::randBall(R);
            return Vector(V.XX, T*0.5*RNG.sreal(), V.YY);
        }

        if ( tok == "outside_sphere" )
        {
            real R = 0;
            is >> R;
            if ( R < 0 ) throw InvalidParameter("outside_sphere:radius must be >= 0");
            Vector P;
            do
                P = spc->randomPlace();
            while ( P.norm() < R );
            return P;
        }
        
        if ( tok == "ellipse" )
        {
            real x = 1, y = 1, z = 0;
            is >> x >> y >> z;
            return Vector(x,y,z).e_mul(Vector::randBall());
        }
        
        if ( tok == "line" )
        {
            real L = 0, T = 0;
            is >> L >> T;
            if ( L < 0 ) throw InvalidParameter("line:length must be >= 0");
            if ( T < 0 ) throw InvalidParameter("line:thickness must be >= 0");
#if ( DIM == 3 )
            Vector2 V = Vector2::randBall(T);
            return Vector(L*0.5*RNG.sreal(), V.XX, V.YY);
#endif
            return Vector(L*0.5*RNG.sreal(), T*0.5*RNG.sreal(), 0);
        }
        
        if ( tok == "arc" )
        {
            real L = 1, Theta = 1.57;
            is >> L >> Theta;
            
            if ( L <= 0 ) throw InvalidParameter("arc:length must be > 0");
            real x = 0, y = 0;
            
            if (Theta == 0) {
                x = 0;
                y = L*RNG.preal() - L/2.0;
            }
            else {
                real R = L / Theta;
                real angle = RNG.real_range(-Theta/2.0, Theta/2.0);
                x = R * cos( angle ) - R;   // origin centered on arc
                y = R * sin( angle );
            }
            return Vector(x, y, 0);
        }
        
        if ( tok == "stripe" )
        {
            if ( spc == 0 )
                throw InvalidParameter("A space must be created first!");
            real s = -0.5, e = 0.5;
            is >> s >> e;
            Vector pos = spc->extension();
            pos = pos.e_mul(Vector::randBox());
            pos.XX = s + RNG.preal() * ( e - s );
            return pos;
        }
        
        if ( tok == "square" )
        {
            real x = 1;
            is >> x;
            return Vector::randBox(x);
        }
        
        if ( tok == "rectangle" )
        {
            real x = 0, y = 0, z = 0;
            is >> x >> y >> z;
            return Vector(x,y,z).e_mul(Vector::randBox());
        }
        
        if ( tok == "edge" )
        {
            if ( spc == 0 )
                throw InvalidParameter("A space must be created first!");
            real R = 1;
            is >> R;
            if ( R <= 0 ) throw InvalidParameter("edge:radius must be > 0");
            return spc->randomPlaceNearEdge(R);
        }
        
        if ( tok == "surface" )
        {
            if ( spc == 0 )
                throw InvalidParameter("A space must be created first!");
            real e = 1;
            is >> e;
            return spc->randomPlaceOnEdge(e);
        }
        
        if ( tok == "gradient" )
        {
            if ( spc == 0 )
                throw InvalidParameter("A space must be created first!");
            real b = spc->extension().norm();  //maximum radius of the volume
            real s = -10, e = 10;
            is >> s >> e;
            real x;
            do {
                x = RNG.preal();
            } while ( RNG.preal() > x );
            return Vector(s+(e-s)*x, b*RNG.sreal(), b*RNG.sreal());
        }
        
#if ( 1 )
        /// A contribution from Beat Rupp
        if ( tok == "segment" || tok == "newsegment" )
        {
            real bending = 0, length = 0, thickness = 0, rotation = 0;
            is >> bending >> length >> thickness >> rotation;
            real x=0, y=0;
            
            // straight
            if ( bending == 0 ) {
                x = thickness * 0.5*RNG.sreal();
                y = length * RNG.preal();
            } else {
                real radius = length / (bending * M_PI);
                real radiusInner = radius - thickness/2.0;
                real theta = fabs( length / radius );
                real angle = RNG.preal() * theta;
                x = (radiusInner + thickness * RNG.preal()) * cos( angle ) - radius;   // substract R to have the arc start from 0,0
                y = (radiusInner + thickness * RNG.preal()) * sin( angle );            
            }
            
            real c = cos(rotation);
            real s = sin(rotation);        
            
            // rotate
            return Vector(c*x + s*y , -s*x + c*y, 0 );        
        }
#endif
        if ( tok == "center" )
            return Vector(0,0,0);
            
        throw InvalidParameter("Unknown position `"+tok+"'");
    }
    
    // expect a vector to be specified:
    real x = 0, y = 0, z = 0;
    is >> x >> y >> z;
    return Vector(x,y,z);
}


//------------------------------------------------------------------------------
/**
 A position is defined with a SHAPE followed by a number of transformations.
 
 Operation                   |  Result
 ----------------------------|----------------------------------------------------
 `at X Y Z`                  | Translate by specified vector (X,Y,Z)
 `add SHAPE`                 | Translate by a vector chosen according to SHAPE
 `align VECTOR`              | Rotate to align parallel with specified vector
 `turn ROTATION`             | Apply specified rotation
 `blur REAL`                 | Add centered Gaussian noise of variance REAL
 `to X Y Z`                  | Interpolate with the previously specified position
 
 A vector is set according to SHAPE, and the transformations are applied one after 
 the other, in the order in which they were given.\n

 Examples:
 @code
   position = 1 0 0
   position = circle 3 at 1 0
   position = square 3 turn 1 1 0 at 1 1
 @endcode
 */ 
Vector Movable::readPosition(std::istream& is, const Space* spc)
{
    std::string tok;
    Vector pos(0,0,0);
    std::streampos isp = 0;
    
    try
    {
        is.clear();
        isp = is.tellg();
        pos = readPrimitive(is, spc);
        is.clear();
        
        while ( !is.eof() )
        {
            isp = is.tellg();
            tok = Tokenizer::get_token(is);
            
            if ( !is.good() )
                break;
            
            if ( tok.size() == 0 || !isalpha(tok[0]) )
                throw InvalidParameter("keyword expected: at, move, add, turn or blur");

            // Translation is specified with 'at' or 'move'
            if ( tok == "at"  ||  tok == "move" )
            {
                Vector vec(0,0,0);
                is >> vec;
                pos = pos + vec;
            }
            // Convolve with shape
            else if ( tok == "add" )
            {
                Vector vec = readPrimitive(is, spc);
                pos = pos + vec;
            }
            // Alignment with a vector is specified with 'align'
            else if ( tok == "align" )
            {
                Vector vec = readDirection(is, pos, spc);
                Rotation rot = Rotation::rotationToVector(vec, RNG);
                pos = rot * pos;
            }
            // Rotation is specified with 'turn'
            else if ( tok == "turn" )
            {
                Rotation rot = readRotation(is, pos, spc);
                pos = rot * pos;
            }
            // Gaussian noise specified with 'blur'
            else if ( tok == "blur" )
            {
                real blur = 0;
                is >> blur;
                pos += Vector::randGauss(blur);
            }
            else
            {
                /*
                 We need to work around a bug in the stream extration operator,
                 which eats extra characters ('a','n','e','E') if doubles are extracted
                 19.10.2015
                 */
                is.clear();
                is.seekg(isp);
                is.seekg(-1, std::ios_base::cur);
                char c = is.peek();
                if ( c=='a' || c=='b' )
                    continue;
                
                throw InvalidParameter("unknown transformation `"+tok+"'");
            }
        }
    }
    catch ( InvalidParameter& e ) {
        StreamFunc::show_line(std::cerr, is, isp);
        throw;
    }
    return pos;
}


//------------------------------------------------------------------------------
/**
 Reads a direction which is a normalized vector:
 
 Keyword                                       |  Resulting Vector
 ----------------------------------------------|------------------------------------------------------------
 `REAL REAL REAL`                              | the vector of norm 1 co-aligned with given vector
 `parallel REAL REAL REAL`                     | one of the two vectors of norm 1 parallel with given vector
 `orthogonal REAL REAL REAL`                   | a vector of norm 1 perpendicular to the given vector
 `horizontal` \n `parallel X`                  | (+1,0,0) or (-1,0,0), randomly chosen with equal chance
 `vertical`\n `parallel Y`                     | (0,+1,0) or (0,-1,0), randomly chosen with equal chance
 `parallel Z`                                  | (0,0,+1) or (0,0,-1), randomly chosen with equal chance
 `parallel XY`\n `parallel XZ`\n `parallel YZ` | A random vector in the specified plane
 `radial`                                      | directed from the origin to the current point
 `circular`                                    | perpendicular to axis joining the current point to the origin
 
 
 If a Space is defined, one may also use:
 
 Keyword         |   Resulting Vector
 ----------------|----------------------------------------------------
 `tangent`       | parallel to the surface of the Space
 `normal`        | perpendicular to the surface
 `centrifuge`    | normal to the surface, directed outward
 `centripete`    | normal to the surface, directed inward


 Note: when the rotation is not uniquely determined in 3D (eg. `horizontal`), 
 cytosim will pick uniformly among all the possible rotations that fulfill the requirements.
 */


Vector Movable::readDirection(std::istream& is, Vector const& pos, const Space* spc)
{
    char c = Tokenizer::eat_space(is, false);
    
    if ( c == EOF )
        return Vector(1,0,0);

    if ( isalpha(c) )
    {
        std::string tok = Tokenizer::get_token(is);
        
        if ( tok == "parallel" )
        {
            char c = Tokenizer::eat_space(is, false);
            
            // an axis or a plane can be specified:
            if ( c == 'X' || c == 'Y' || c == 'Z' )
            {
                std::string k = Tokenizer::get_token(is);
                
                if ( k == "X" )
                    return Vector(RNG.sflip(), 0, 0);
                if ( k == "Y" )
                    return Vector(0, RNG.sflip(), 0);
                if ( k == "Z" && DIM == 3 )
                    return Vector(0, 0, RNG.sflip());
                if ( k == "XY" )
                {
                    Vector2 h = Vector2::randUnit();
                    return Vector(h.XX, h.YY, 0);
                }
                if ( k == "XZ" && DIM == 3 )
                {
                    Vector2 h = Vector2::randUnit();
                    return Vector(h.XX, 0, h.YY);
                }
                if ( k == "YZ" && DIM == 3 )
                {
                    Vector2 h = Vector2::randUnit();
                    return Vector(0, h.XX, h.YY);
                }
                throw InvalidParameter("Unexpected keyword `"+k+"' after `parallel`");
            }
            
            real x = 1, y = 0, z = 0;
            is >> x >> y >> z;
            is.clear();
            return Vector(x,y,z).normalized();
        }
        
        if ( tok == "orthogonal" )
        {
            real x = 1, y = 0, z = 0;
            is >> x >> y >> z;
            is.clear();
            return Vector(x,y,z).randPerp(1);
        }
        
        if ( tok == "horizontal" )
            return Vector(RNG.sflip(), 0, 0);
        
        if ( tok == "vertical" )
            return Vector(0, RNG.sflip(), 0);
        
        if ( tok == "radial" )
            return pos.normalized();
        
        if ( tok == "circular" )
            return pos.randPerp(1);
        
#if ( DIM == 3 )
        if ( tok == "orthoradial" )
        {
            Vector yz(0, pos.YY, pos.ZZ);
            return vecProd(yz.normalized(), Vector(RNG.sflip(), 0, 0));
        }
#endif
        
        if ( tok == "circular" )
            return pos.randPerp(1);

        if ( spc )
        {
            if ( tok == "tangent" )
                return spc->normalToEdge(pos).randPerp(1);
            
#if ( DIM == 3 )
            Vector dir(0,0,1);
#elif ( DIM == 2 )
            real dir = 1;
#endif
            
#if ( DIM > 1 )
            if ( tok == "clockwise" )
                return vecProd(dir, spc->normalToEdge(pos));
            
            if ( tok == "anticlockwise" )
                return -vecProd(dir, spc->normalToEdge(pos));
#endif
            
            if ( tok == "normal" )
                return RNG.sflip() * spc->normalToEdge(pos);
            
            if ( tok == "centrifuge" )
                return -spc->normalToEdge(pos);
            
            if ( tok == "centripete" )
                return spc->normalToEdge(pos);
        }
        
        throw InvalidParameter("Unknown direction `"+tok+"'");
    }
    
    // expect a Vector:
    real x = 1, y = 0, z = 0;
    is >> x >> y >> z;
    is.clear();
    return Vector(x,y,z).normalized();
}


/**
 The initial orientation of objects is defined by a rotation, which can be
 specified as follows:
 
 Keyword                   |   Rotation / Result
 --------------------------|----------------------------------------------------
 `random`                  | A rotation selected uniformly among all possible rotations
 `identity`                | The object is not rotated
 `angle A B C`             | As specified by 3 Euler angles in radians (in 2D, only A is needed)
 `degree A B C`            | As specified by 3 Euler angles in degrees (in 2D, only A is needed)
 `quat q0 q1 q2 q3`        | As specified by the Quaternion (q0, q1, q2, q3)
 DIRECTION                 | see @ref Movable::readDirection
 
 In the last case, a rotation will be built that transforms (1, 0, 0) into the given vector. 
 In 3D, this does not define the rotation uniquely (eg. `horizontal`), and cytosim 
 will pick uniformly among all the possible rotations that fulfill the requirements.
 
*/

Rotation Movable::readRotation(std::istream& is, Vector const& pos, const Space* spc)
{
    char c = Tokenizer::eat_space(is, false);
    
    if ( c == EOF )
        return Rotation::randomRotation(RNG);

    if ( isalpha(c) )
    {
        std::streampos isp = is.tellg();
        std::string tok = Tokenizer::get_token(is);
        
        if ( tok == "random" )
            return Rotation::randomRotation(RNG);
        else if ( tok == "identity" || tok == "none" )
        {
            return Rotation::one();
        }
        else if ( tok == "angle" )
        {
            Torque a;
            is >> a;
#if ( 0 )
            std::string unit;
            is >> unit;
            if ( unit == "degree" )
                a *= M_PI/180.0;
#endif
            return Rotation::rotationFromEulerAngles(a);
        }
        else if ( tok == "degree" )
        {
            Torque a;
            is >> a;
            a *= M_PI/180.0;
            return Rotation::rotationFromEulerAngles(a);
        }
#if ( DIM == 3 )
        else if ( tok == "quat" )
        {
            Quaternion<real> quat;
            is >> quat;
            quat.normalize();
            Rotation rot;
            quat.setMatrix3(rot);
            return rot;
        }
#endif
        
        is.clear();
        is.seekg(isp, std::ios::beg);
    }

    // The last option is to specity a vector:
    Vector vec = readDirection(is, pos, spc);
    
    /*
     A single Vector does not uniquely define a rotation in 3D:
     return a random rotation that is picked uniformly among
     the all possible rotations transforming (1,0,0) in vec.
     */
    return Rotation::rotationToVector(vec, RNG);
}

