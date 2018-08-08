// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef  IOWRAPPER_H
#define  IOWRAPPER_H

#include <cstdio>
#include <cstdarg>
#include <stdint.h>
#include "filewrapper.h"

/// Input with automatic binary/text mode and byte-swapping for cross-platform compatibility
/**
 @todo:
 Introduce a set of classes:
 Inputter (text, base class)
 InputterText
 InputterBinary
 InputterBinarySwap
 
 Instead of setBinarySwap), you would use the appropriate class.
 
 Outputter
 OutputterText
 
 
 */
class InputWrapper : public FileWrapper
{
private:
        
    /// The format ID of the input: this allow backward compatibility with older formats
    int       inFormat;
    
    /// The dimensionality of vectors stored in the file
    unsigned  inDIM;
    
    /** if the state is stored in a binary format, inBinary
        is set to 1 or 2. with 2, byte order is swapped automatically
        this occurs for example when reading a simulation calculated 
        on PC from mac, or vice et versa.
        */
    int       inBinary;
    
    /// reverse order of bytes in c[2]
    /**
     Can use the Intel SIMD function _bswap() and _bswap64()
     */
    inline void swap2(char* c)
    {
        char s = c[0];
        c[0] = c[1];
        c[1] = s;
    }
 
    /// reverse order of bytes in c[4]
    inline void swap4(char* c)
    {
        char s = c[0];
        char t = c[1];
        c[0] = c[3];
        c[1] = c[2];
        c[3] = s;
        c[2] = t;
    }

    /// reverse order of bytes in c[8]
    inline void swap8(char* c)
    {
        char s = c[0];
        char t = c[1];
        char u = c[2];
        char v = c[3];
        c[0] = c[7];
        c[1] = c[6];
        c[2] = c[5];
        c[3] = c[4];
        c[7] = s;
        c[6] = t;
        c[5] = u;
        c[4] = v;
    }

public:
    
    /// set defaults (not-binary)
    void      iwConstructor();
    
    /// Constructor
    InputWrapper() : FileWrapper(0)  { iwConstructor(); }
    
    /// Constructor
    InputWrapper(FILE * f, const char * path = 0) : FileWrapper(f, path)  { iwConstructor(); }
    
    /// constructor which opens a file
    InputWrapper(const char* name, bool bin) : FileWrapper(name, bin?"rb":"r") { iwConstructor(); }
    
    /// Destructor which closes the input and output files
    virtual   ~InputWrapper() {}

    /// Sets dimentionnality of vectors
    void      inputDIM(const int d)    { inDIM = d; }
    
    /// returns the type of input
    int       formatID()         const { return inFormat; }

    /// returns the type of input
    void      formatID(const int f)    { inFormat = f; }

    /// Returns 1 for native binary format, 2 for non-native binary format, and 0 if not binary
    int       binary()           const { return inBinary; }
    
    /// Automatically find in which way bytes are stored in the binary format
    void      setBinarySwap(const char[2]);
    
    /// Read integer on 1 byte
    int8_t    readInt8();
    /// Read integer on 2 bytes
    int16_t   readInt16();
    /// Read integer on 4 bytes
    int32_t   readInt32();

    /// Read unsigned integer on 1 byte
    uint8_t   readUInt8();
    /// Read unsigned integer on 2 bytes
    uint16_t  readUInt16();
    /// Read unsigned integer on 4 bytes
    uint32_t  readUInt32();
    
    /// Reads a float on 4 bytes
    float     readFloat();
    /// Reads a float on 8 bytes
    double    readDouble();
    /// Reads a vector, and store in the array of size D
    void      readFloatVector(float*, unsigned D);
    /// Reads a vector, and store in the array of size D
    void      readFloatVector(double*, unsigned D);
    /// Reads a vector, and store in the array of size D
    void      readFloatVector(float*, unsigned n, unsigned D);
    /// Reads a vector, and store in the array of size D
    void      readFloatVector(double*, unsigned n, unsigned D);

};


#pragma mark -


///Output with automatic binary/text mode and byte-swapping for cross-platform compatibility
class OutputWrapper : public FileWrapper
{
    
private:
        
    /// Flag for binary output
    bool    mBinary;

public:

    /// constructor
    OutputWrapper();

    /// constructor which opens a file
    OutputWrapper(const char* name, bool a, bool b=false);
    
    /// Open a file. mode[1] can be 'b' to specify binary mode
    int     open(const char* name, bool a, bool b=false);
    
    /// Destructor which closes the input and output files
    virtual ~OutputWrapper() {}
    
    /// Sets to write in binary format
    void    binary(bool b) { mBinary = b; }
    
    /// Return the current binary format
    bool    binary() const { return mBinary; }
    
    /// Puts a tag to specify a binary file, and the byte order 
    void    writeBinarySignature(const char[]);
        
    /// Inserts a return, only in text output mode, for nicer output
    void    writeSoftNewLine();
    
    /// Inserts N space(s), only in text output mode, for nicer output
    void    writeSoftSpace(int N = 1);
    
    /// Write integer on 1 byte 
    void    writeInt8(int, char before=' ');
    /// Write integer on 2 bytes
    void    writeInt16(int, char before=' ');
    /// Write integer on 4 bytes
    void    writeInt32(int, char before=' ');
    
    /// Write unsigned integer on 1 byte  
    void    writeUInt8(unsigned int, char before=' ');
    /// Write unsigned integer on 2 bytes
    void    writeUInt16(unsigned int, char before=' ');
    /// Write unsigned integer on 4 bytes
    void    writeUInt32(unsigned int, char before=' ');
    
    /// Write a float (4 bytes)
    void    writeFloat(float);
    /// Write n floats (4 bytes each)
    void    writeFloatVector(const float*, unsigned n, char before=0);
    /// Write n floats (4 bytes each)
    void    writeFloatVector(const double*, unsigned n, char before=0);

    /// Write a double (8 bytes)
    void    writeDouble(double);
    /// Write n double (8 bytes each)
    void    writeDoubleVector(const double*, unsigned n, char before=0);

};

#endif
