// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "iowrapper.h"
#include "exceptions.h"


///check the size of the type, as we rely on them to write byte-by-byte
int nonStandardTypeSizes()
{
    if ( 2 != sizeof(uint16_t) ) return 1;
    if ( 4 != sizeof(uint32_t) ) return 2;
    if ( 8 != sizeof(uint64_t) ) return 3;
    if ( 4 != sizeof(float) )    return 4;
    if ( 8 != sizeof(double) )   return 5;
    return 0;
}


//============================================================================
//=========                     INPUT                              ===========
//============================================================================


void InputWrapper::iwConstructor()
{
    inFormat  = 0;
    inBinary  = 0;
    inDIM     = 3;
    
    if ( nonStandardTypeSizes() )
    {
        fprintf(stderr, "Error: non-standard types in InputWrapper\n");
        exit(EXIT_FAILURE);
    }
}


/**
 Reads a short and compares with the native storage, to set
 inBinary=1, for same-endian or inBinary = 2, for opposite endian
*/
void InputWrapper::setBinarySwap(const char import[2])
{
    char native[3] = { 0 };
    *((uint16_t*)native) = 12592U;
    //inBinary = 1 for same-endianess, 2 for opposite-endianess:
    inBinary = 1 + ( import[0] != native[0] );
}


int8_t InputWrapper::readInt8()
{
    int8_t v;
    
    if ( inBinary )
    {
        v = getUL();
    }
    else
    {
        int u;
        if ( 1 != fscanf(mFile, " %i", &u) )
            throw InvalidIO("readUInt8() failed");
        v = u;
        if ( v != u )
            throw InvalidIO("invalid int8_t");
    }
    return v;
}


int16_t InputWrapper::readInt16()
{
    int16_t v;
    
    if ( inBinary )
    {
        if ( 1 != fread(&v, 2, 1, mFile) )
            throw InvalidIO("fread failed");
        if ( inBinary == 2 )
            swap2(reinterpret_cast<char*>(&v));
    }
    else
    {
        int u;
        if ( 1 != fscanf(mFile, " %i", &u) )
            throw InvalidIO("readInt16() failed");
        v = u;
        if ( v != u )
            throw InvalidIO("invalid int16_t");
    }
    return v;
}


int32_t InputWrapper::readInt32()
{
    int32_t v;
    
    if ( inBinary )
    {
        if ( 1 != fread(&v, 4, 1, mFile) )
            throw InvalidIO("fread failed");
        if ( inBinary == 2 )
            swap4(reinterpret_cast<char*>(&v));
    }
    else
    {
        int u;
        if ( 1 != fscanf(mFile, " %i", &u) )
            throw InvalidIO("readInt32() failed");
        v = u;
        if ( v != u )
            throw InvalidIO("invalid int32_t");
    }
    return v;
}


uint8_t InputWrapper::readUInt8()
{
    uint8_t v;

    if ( inBinary )
    {
        v = getUL();
    }
    else
    {
        unsigned u;
        if ( 1 != fscanf(mFile, " %u", &u) )
            throw InvalidIO("readUInt8() failed");
        v = u;
        if ( v != u )
            throw InvalidIO("invalid uint8_t");
    }
    return v;
}


uint16_t InputWrapper::readUInt16()
{
    uint16_t v;
    
    if ( inBinary )
    {
        if ( 1 != fread(&v, 2, 1, mFile) )
            throw InvalidIO("fread failed");
        if ( inBinary == 2 )
            swap2(reinterpret_cast<char*>(&v));
    }
    else
    {
        unsigned u;
        if ( 1 != fscanf(mFile, " %u", &u) )
            throw InvalidIO("readUInt16() failed");
        v = u;
        if ( v != u )
            throw InvalidIO("invalid uint16_t");
    }
    return v;
}


uint32_t InputWrapper::readUInt32()
{
    uint32_t v;
    
    if ( inBinary )
    {
        if ( 1 != fread(&v, 4, 1, mFile) )
            throw InvalidIO("fread failed");
        if ( inBinary == 2 )
            swap4(reinterpret_cast<char*>(&v));
    }
    else
    {
        unsigned u;
        if ( 1 != fscanf(mFile, " %u", &u) )
            throw InvalidIO("readUInt32() failed");
        v = u;
        if ( v != u )
            throw InvalidIO("invalid uint32_t");
    }
    return v;
}



float InputWrapper::readFloat()
{
    float v;
    
    if ( inBinary )
    {
        if ( 1 != fread(&v, 4, 1, mFile) )
            throw InvalidIO("fread failed");
        if ( inBinary == 2 )
            swap4(reinterpret_cast<char*>(&v));
    }
    else
    {
        if ( 1 != fscanf(mFile, " %f", &v) )
            throw InvalidIO("readFloat() failed");
    }
    return v;
}


double InputWrapper::readDouble()
{
    double v;
    
    if ( inBinary )
    {
        if ( 1 != fread(&v, 8, 1, mFile) )
            throw InvalidIO("fread failed");
        if ( inBinary == 2 )
            swap8(reinterpret_cast<char*>(&v));
    }
    else
    {
        if ( 1 != fscanf(mFile, " %lf", &v) )
            throw InvalidIO("readDouble() failed");
    }
    return v;
}


/*
 This will read inDIM floats, and store the first D ones in a[].
 */
void InputWrapper::readFloatVector(float a[], const unsigned D)
{
    try {
        unsigned d;
        if ( inDIM <= D )
        {
            for ( d = 0; d < inDIM; ++d )
                a[d] = readFloat();
            for (; d < D; ++d )
                a[d] = 0;
        }
        else 
        {
            for ( d = 0; d < D; ++d )
                a[d] = readFloat();
            for (; d < inDIM; ++d )
                readFloat();
        }
    }
    catch( Exception& ) {
        throw InvalidIO("readFloatVector() failed");
    }
}


/*
 This will read inDIM floats, and store the first D ones in a[].
 */
void InputWrapper::readFloatVector(double a[], const unsigned D)
{
    try {
        unsigned d;
        if ( inDIM <= D )
        {
            for ( d = 0; d < inDIM; ++d )
                a[d] = readFloat();
            for (; d < D; ++d )
                a[d] = 0;
        }
        else
        {
            for ( d = 0; d < D; ++d )
                a[d] = readFloat();
            for (; d < inDIM; ++d )
                readFloat();
        }
    }
    catch( Exception& ) {
        throw InvalidIO("readFloatVector() failed");
    }
}


/*
 This will read n * inDIM floats, and store the n * D ones in a[].
 */
void InputWrapper::readFloatVector(double a[], const unsigned n, const unsigned D)
{
    const size_t nd = n * inDIM;
    float * v = new float[nd];
    
    if ( inBinary )
    {
        if ( nd != fread(v, 4, nd, mFile) )
        {
            delete[] v;
            throw InvalidIO("fread failed");
        }
        if ( inBinary == 2 )
            for ( unsigned u = 0; u < nd; ++u )
                swap4(reinterpret_cast<char*>(v+u));
    }
    else
    {
        for ( unsigned u = 0; u < nd; ++u )
            if ( 1 != fscanf(mFile, " %f", v+u) )
            {
                delete[] v;
                throw InvalidIO("readFloat() failed");
            }
    }

    const unsigned m = ( inDIM < D ? inDIM : D );
    
    for ( unsigned u = 0; u < n; ++u )
    {
        unsigned d;
        for ( d = 0; d < m; ++d )
            a[D*u+d] = v[inDIM*u+d];
        for (; d < D; ++d )
            a[D*u+d] = 0;
    }
    
    delete[] v;
}

//============================================================================
//=========                    OUTPUT                              ===========
//============================================================================
#pragma mark -

OutputWrapper::OutputWrapper()
: FileWrapper(stdout) 
{
    mBinary = false;
    
    if ( nonStandardTypeSizes() )
    {
        fprintf(stderr, "Error: non-standard types in InputWrapper\n");
        exit(EXIT_FAILURE);
    }
}


OutputWrapper::OutputWrapper(const char* name, const bool a, const bool b)
{
    open(name, a, b);
    
    if ( nonStandardTypeSizes() )
    {
        fprintf(stderr, "Error: non-standard types in InputWrapper\n");
        exit(EXIT_FAILURE);
    }
}


int OutputWrapper::open(const char* name, const bool a, const bool b)
{
    mBinary = b;
    
    //create a 'mode' string appropriate for Windows OS
    char m[3] = { 0 };
    
    if ( a )
        m[0] = 'a';
    else
        m[0] = 'w';
    
    if ( b )
        m[1] = 'b';
        
    return FileWrapper::open(name, m);
}


void OutputWrapper::writeBinarySignature(const char str[])
{
    fputs(str, mFile);
    
    //the value corresponds to the ASCII code of "01" 
    uint16_t x = 12592U;
    if ( 2 != fwrite(&x, 1, 2, mFile) )
        throw InvalidIO("writeBinarySignature() failed");
    
    putc('\n', mFile);
}


void OutputWrapper::writeInt8(const int n, char before)
{
    int8_t v = n;
    
    if ( n != v )
        ABORT_NOW("value out of range");

    if ( mBinary )
    {
        if ( 1 != fwrite(&v, 1, 1, mFile) )
            throw InvalidIO("writeInt8()-binary failed");
    }
    else
    {
        if ( 2 > fprintf(mFile, "%c%i", before, n) )
            throw InvalidIO("writeInt8() failed");
    }
}


void OutputWrapper::writeInt16(const int n, char before)
{
    int16_t v = n;
    
    if ( n != v )
        ABORT_NOW("value out of range");

    if ( mBinary )
    {
        if ( 2 != fwrite(&v, 1, 2, mFile) )
            throw InvalidIO("writeInt16()-binary failed");
    }
    else
    {
        if (2 > fprintf(mFile, "%c%i", before, n))
            throw InvalidIO("writeInt16() failed");
    }
}


void OutputWrapper::writeInt32(const int n, char before)
{
    int32_t v = n;
    
    if ( n != v )
        ABORT_NOW("value out of range");
    
    if ( mBinary )
    {
        if ( 4 != fwrite(&v, 1, 4, mFile) )
            throw InvalidIO("writeInt32()-binary failed");
    }
    else
    {
        if ( 2 > fprintf(mFile, "%c%d", before, n) )
            throw InvalidIO("writeInt32() failed");
    }
}



void OutputWrapper::writeUInt8(const unsigned int n, char before)
{
    uint8_t v = n;
    
    if ( n != v )
        ABORT_NOW("value out of range");
    
    if ( mBinary )
    {
        if ( 1 != fwrite(&v, 1, 1, mFile) )
            throw InvalidIO("writeUInt8()-binary failed");
    }
    else
    {
        if ( before )
        {
            if ( 2 > fprintf(mFile, "%c%u", before, n) )
                throw InvalidIO("writeUInt8() failed");
        }
        else {
            if ( 1 > fprintf(mFile, "%u", n) )
                throw InvalidIO("writeUInt8() failed");
        }
    }
}


void OutputWrapper::writeUInt16(const unsigned int n, char before)
{
    uint16_t v = n;
    
    if ( n != v )
        ABORT_NOW("value out of range");

    if ( mBinary )
    {
        if ( 2 != fwrite(&v, 1, 2, mFile) )
            throw InvalidIO("writeUInt16()-binary failed");
    }
    else
    {
        if ( before )
        {
            if ( 2 > fprintf(mFile, "%c%u", before, n) )
                throw InvalidIO("writeUInt16() failed");
        }
        else {
            if ( 1 > fprintf(mFile, "%u", n) )
                throw InvalidIO("writeUInt16() failed");
        }
    }
}


void OutputWrapper::writeUInt32(const unsigned int n, char before)
{
    uint32_t v = n;
    
    if ( n != v )
        ABORT_NOW("value out of range");
    
    if ( mBinary )
    {
        if ( 4 != fwrite(&v, 1, 4, mFile) )
            throw InvalidIO("writeUInt32()-binary failed");
    }
    else
    {
        if ( before )
        {
            if ( 2 > fprintf(mFile, "%c%u", before, n) )
                throw InvalidIO("writeUInt32() failed");
        }
        else
        {
            if ( 1 > fprintf(mFile, "%u", n) )
                throw InvalidIO("writeUInt32() failed");
        }
    }
}



void OutputWrapper::writeFloat(const float x)
{
    if ( mBinary )
    {
        if ( 4 != fwrite(&x, 1, 4, mFile) )
            throw InvalidIO("writeFloat()-binary failed");
    }
    else
    {
        if ( 6 > fprintf(mFile, " %.4f", x) )
            throw InvalidIO("writeFloat() failed");
    }
}


void OutputWrapper::writeFloatVector(const float* a, const unsigned n, char before)
{
    if ( before && !mBinary )
        putc(before, mFile);
    
    for ( unsigned d = 0; d < n; ++d )
        writeFloat(a[d]);
}


void OutputWrapper::writeFloatVector(const double* a, const unsigned n, char before)
{
    if ( before && !mBinary )
        putc(before, mFile);
    
    for ( unsigned d = 0; d < n; ++d )
        writeFloat(a[d]);
}


void OutputWrapper::writeDouble(const double x)
{
    if ( mBinary )
    {
        if ( 8 != fwrite(&x, 1, 8, mFile) )
            throw InvalidIO("writeDouble()-binary failed");
    }
    else
    {
        if ( 10 > fprintf(mFile, " %.8lf", x) )
            throw InvalidIO("writeDouble() failed");
    }
}


void OutputWrapper::writeDoubleVector(const double* a, const unsigned n, char before)
{
    if ( before && !mBinary )
        putc(before, mFile);
    
    for ( unsigned d = 0; d < n; ++d )
        writeDouble(a[d]);
}


void OutputWrapper::writeSoftNewLine()
{
    if ( !mBinary )
        putc('\n', mFile);
    fflush(mFile);
}


void OutputWrapper::writeSoftSpace(int N)
{
    if ( !mBinary )
    {
        while ( N > 0 ) {
            fprintf(mFile, " ");
            N--;
        }
    }
}

