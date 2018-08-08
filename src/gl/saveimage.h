// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#ifndef SAVEIMAGE_H
#define SAVEIMAGE_H

#include <cstdio>


#ifdef NO_OPENGL

typedef unsigned char GLubyte;

#else

#  ifdef __APPLE__
#    include <OpenGL/gl.h>
#  else
#    include <GL/glew.h>
#  endif

#endif


/// Can save pixel array to files in (PNG or PPM) format
/**
 - PPM files do not require any library, and thus writing is supported on any platform.
   They can be read by various software, in particular the 'pbmplus' toolkit
   However, they are big and usually not supported by most viewers.
   Known viewers on Mac OS X: ImageJ, GraphicConverter, ToyViewer.
 - PNG files are more modern all purpose image format.
 .
 
 PNG support requires 'libpng'.
 
 Both libraries are available on Mac OSX via macPorts (www.macports.org):
 - port install libpng
 .
 */
namespace SaveImage
{
    /// open a file for binary write
    FILE * openFile(const char * name);
    
    /// Netpbm pixel image format, 3 bytes per pixels (R, G, B). Returns error-code
    int saveColorPPM(FILE* file, const GLubyte pixels[], int width, int height);
    
    /// write PNG image
    int savePNG(FILE* file, void* pixels, int bit_depth, int nb_colors, int width, int height);

    /// Portable Network Graphic format, 3 bytes per pixels (R, G, B)
    int saveAlphaPNG(FILE* file, void* pixels, int width, int height);

    /// Portable Network Graphic format, 3 bytes per pixels (R, G, B)
    int saveColorPNG(FILE* file, void* pixels, int width, int height);

    /// gray-level PNG format with 2 bytes per pixels
    int saveGrayPNG(FILE* file, void* pixels, int width, int height);
   
    /// save pixels[] and return error-code
    int savePixels(const char format[], FILE *, GLubyte* pixels, int width, int height);
    
    /// save pixels[] and return error-code
    int savePixels(const char format[], const char* name, GLubyte* pixels, int width, int height);

    //-------------------- use the functions below: ---------------------
    
    /// true if 'format' is the 3-letter file-entension of a supported image format
    /**
     Currently 'png' and 'ppm' are supported. 
     The extension can equally be lowercase or uppercase.
     */
    bool supported(const char format[]);
    
    /// save a region of the current buffer. The appropriate file-extension is added to 'name'. Returns error-code
    int saveImage(const char format[], const char* name, int x, int y, int width, int height);
    
    /// calls saveImage, with a file image????, where ???? is the 4-digit number 'nb'
    int saveImage(const char format[], unsigned int nb, int x, int y, int width, int height);

    /// save an image with higher resolution (better version)
    int saveMagnifiedImage(const char format[], const char* name, int mag, int width, int height, void (*display)(int, void *), void * arg);

    /// save an image with higher resolution
    int saveCompositeImage(const char format[], const char* name, int mag, int width, int height, double pixel_size, void (*display)(int, void *), void * arg);

    /// save mag^2 images, which can be tiled to get a high-resolution picture
    int saveTiledImages(const char format[], int mag, int width, int height, void (*display)(int, void *), void * arg);
    
    /// save mag^2 images, which can be tiled to get a high-resolution picture
    int saveTiledImagesT(const char format[], int mag, int width, int height, double pixel_size, void (*display)(int, void *), void * arg);
    
    /// assemble tiles using PPM tools
    void assemblePPMTiles(const char format[], int mag);
}


#endif
