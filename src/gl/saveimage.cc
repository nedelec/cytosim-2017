// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "saveimage.h"
#include <cstdlib>
#include <cstring>

#ifndef NO_OPENGL
#  ifdef __APPLE__
#    include <OpenGL/gl.h>
#  else
#    include <GL/glew.h>
#  endif
#endif

//------------------------------------------------------------------------------
bool SaveImage::supported(const char format[])
{
#ifdef HAS_PNG
    if ( 0 == strcasecmp(format, "png") )
        return true;
#endif
    if ( 0 == strcasecmp(format, "ppm") )
        return true;
    
    return false;
}



//------------------------------------------------------------------------------
/**
 saveImage(...) will read pixels from the current OpenGL read buffer,
 and save them in a file with the requested format
 */
int SaveImage::saveImage(const char format[],
                         const char * filename,
                         const int xpos, const int ypos,
                         const int width, const int height)
{
    int res = 1;

#ifndef NO_OPENGL
    //allocate memory to hold image:
    GLubyte* pixels = new GLubyte[width*height*3];

    //set the alignment to double-words
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    
    //read the pixel values, from top-left corner:
    glReadPixels(xpos, ypos, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    //glutReportErrors();
    
    res = savePixels(format, filename, pixels, width, height);
    delete[] pixels;
#else
    fprintf(stderr, "Error: SaveImage was built without OpenGL!\n");
#endif

    return res;
}


int SaveImage::saveImage(const char format[],
                         unsigned int indx,
                         const int xpos, const int ypos,
                         const int width, const int height)
{
    char name[32];
    snprintf(name, sizeof(name), "image%04i.%s", indx, format);
    
    return saveImage(format, name, xpos, ypos, width, height);
}


//------------------------------------------------------------------------------
#pragma mark Composite

int SaveImage::saveTiledImages(const char format[], 
                               const int mag,
                               const int width, const int height,
                               void (*display)(int, void *), void * arg)
{
    if ( ! supported(format) )
        return -1;
    int res = 1;
    
#ifndef NO_OPENGL

    char name[1024];
    
    for ( int iy = 0; iy < mag ; ++iy )
    {
        for ( int ix = 0; ix < mag; ++ix )
        {
            glViewport(-ix*width, -iy*height, mag*width, mag*height);
            display(mag, arg);
            
            snprintf(name, sizeof(name), "tile-%02ix%02i.%s", iy, ix, format);
            
            res = saveImage(format, name, 0, 0, width, height);
            
            if (res)
                goto cleanup;
            else
                printf("saved image %s\n", name);
        }
    }
    
    if ( 0 == strcasecmp(format, "ppm") )
        assemblePPMTiles(format, mag);
    
cleanup:
    //restore initial viewport:
    glViewport(0, 0, width, height);
#endif
    
    return res;
}
//------------------------------------------------------------------------------
/**
 This translate the ModelView matrix to produce an image with higher resolution
 */
int SaveImage::saveTiledImagesT(const char format[],
                                const int mag,
                                const int width, const int height,
                                const double pixel_size,
                                void (*display)(int, void *), void * arg)
{
    if ( ! supported(format) )
        return -1;
    int res = 1;
    
#ifndef NO_OPENGL
    char name[1024];
    glMatrixMode(GL_MODELVIEW);
    const double cc = ( mag - 1 ) * 0.5;

    for ( int iy = 0; iy < mag ; ++iy )
    {
        for ( int ix = 0; ix < mag; ++ix )
        {
            glPushMatrix();
            glTranslated((cc-ix)*width*pixel_size, (cc-iy)*height*pixel_size, 0);
            glScaled(mag, mag, mag);
            display(mag, arg);
            glPopMatrix();
            
            snprintf(name, sizeof(name), "tile-%02ix%02i.%s", iy, ix, format);
            
            res = saveImage(format, name, 0, 0, width, height);
            
            if ( res )
                return res;
        }
    }
#endif
    return res;
}

//------------------------------------------------------------------------------
void SaveImage::assemblePPMTiles(const char format[], const int mag)
{
    char cmd[4096], *msg=cmd, *end=cmd+sizeof(cmd);
    char name[1024];
    
    for ( int iy = 0; iy < mag ; ++iy )
    {
        msg = cmd + snprintf(cmd, sizeof(cmd), "pnmcat -lr ");
        for ( int ix = 0; ix < mag; ++ix )
        {
            snprintf(name, sizeof(name), "tile-%02ix%02i.%s", iy, ix, format);
            msg += snprintf(msg, end-msg, " %s", name);
        }
        msg += snprintf(msg, end-msg, " > line-%i.ppm\n", mag-1-iy);
        system(cmd);
        printf("%s", cmd);
    }

    msg = cmd + snprintf(cmd, sizeof(cmd), "pnmcat -tb");
    for ( int iy = 0; iy < mag ; ++iy )
        msg += snprintf(msg, end-msg, " line-%i.ppm", iy);
    msg += snprintf(msg, end-msg, " > tileRGB.ppm\n");
    system(cmd);
    printf("%s", cmd);
    system("ppmquant 256 tileRGB.ppm > tile.ppm\n");
    printf("assembled tile.ppm\n");
}

//------------------------------------------------------------------------------
/**
 After setting a higher resolution, this will translate the ModelView to produce several
 images that will be stiched together, into an image with higher resolution.
 This works even if the image is larger than the maximum OpenGL viewPort,
 but the result is not always identical as stitching it not perfect.
 */
int SaveImage::saveCompositeImage(const char format[],
                                  const char * filename,
                                  const int mag,
                                  const int width, const int height,
                                  const double pixel_size,
                                  void (*display)(int, void *), void * arg)
{
    if ( ! supported(format) )
        return -1;
    int res = 1;
    
#ifndef NO_OPENGL

    const int pix = 3;  //number of bytes for each pixel
    GLubyte* pixels = new GLubyte[width*mag*height*mag*pix];
    GLubyte* sub    = new GLubyte[width*height*pix];
    
    const double cc = ( mag - 1 ) * 0.5;
    const double dx = width*pixel_size / mag;
    const double dy = height*pixel_size / mag;
    
    glMatrixMode(GL_MODELVIEW);
    glScaled(mag, mag, mag);

    for ( int iy = 0; iy < mag ; ++iy )
    {
        for ( int ix = 0; ix < mag; ++ix )
        {
            glPushMatrix();
            glTranslated((cc-ix)*dx, (cc-iy)*dy, 0);
            display(mag, arg);
            glPopMatrix();
            
            glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, sub);

            GLubyte * dst = &pixels[width*pix*(ix+mag*height*iy)];
            for ( int ii=0; ii<height; ++ii )
                memcpy(&dst[ii*mag*width*pix], &sub[ii*width*pix], width*pix);            
        }
    }
    delete[] sub;

    res = savePixels(format, filename, pixels, mag*width, mag*height);
    
    delete[] pixels;
    
#endif
    return res;
}


//------------------------------------------------------------------------------
/**
 This adjusts the Viewport to produce an image with higher resolution
 */
int SaveImage::saveMagnifiedImage(const char format[],
                                  const char * filename,
                                  const int mag,
                                  const int width, const int height,
                                  void (*display)(int, void *), void * arg)
{
    if ( ! supported(format) )
        return -1;
    int res = 1;
    
#ifndef NO_OPENGL
    GLint maxSize[2];
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxSize);
    if ( width*mag > maxSize[0]  ||  height*mag > maxSize[1] )
    {
        fprintf(stderr, "SaveImage:: exceeding maximum viewport dimensions (%ix%i)\n", (int)maxSize[0], (int)maxSize[1]);
        return 1;
    }
    
    const int pix = 3;  //number of bytes for each pixel
    //allocate memory to hold the full image:
    GLubyte* pixels = new GLubyte[width*mag*height*mag*pix];
    GLubyte* sub    = new GLubyte[width*height*pix];

    //set the alignment ?
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    
    for ( int iy = 0; iy < mag ; ++iy )
    {
        for ( int ix = 0; ix < mag; ++ix )
        {
            glViewport(-ix*width, -iy*height, mag*width, mag*height);
            display(mag, arg);
            glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, sub);
            
            GLubyte * dst = &pixels[width*pix*(ix+mag*height*iy)];
            for ( int ii=0; ii<height; ++ii )
                memcpy(&dst[ii*mag*width*pix], &sub[ii*width*pix], width*pix);            
        }
    }
    delete[] sub;
    sub = 0;
    
    res = savePixels(format, filename, pixels, mag*width, mag*height);
    
    delete[] pixels;
    //restore initial viewport:
    glViewport(0, 0, width, height);
#endif
    
    return res;
}
//------------------------------------------------------------------------------
#pragma mark savePixels

FILE * SaveImage::openFile(const char * filename)
{
    if ( filename[0] == '\0' )
        return 0;
    FILE * file = fopen(filename, "wb");
    if ( file && ferror(file) )
    {
        fclose(file);
        file = 0;
    }
    return file;
}


int SaveImage::savePixels(const char format[],
                          FILE * file,
                          GLubyte* pixels,
                          int width, int height)
{
    if ( 0 == strcasecmp(format, "ppm") )
        return saveColorPPM(file, pixels, width, height);
    
    if ( 0 == strcasecmp(format, "png") )
        return saveColorPNG(file, pixels, width, height);

    return -1;
}


int SaveImage::savePixels(const char format[],
                          const char * name,
                          GLubyte* pixels,
                          int width, int height)
{
    FILE * file = openFile(name);
    
    if ( file )
    {
        int res = savePixels(format, file, pixels, width, height);
        
        fclose(file);
        
        // remove file if an error occured:
        if ( res )
            remove(name);
        
        return res;
    }
    
    return 3;
}

//------------------------------------------------------------------------------
//------------------------------- PPM FORMAT -----------------------------------
//------------------------------------------------------------------------------

/**
 Write the image in the Portable Pixmap format, also Netpbm format (man -k ppm).
 We use here the 'raw' binary format starting with P6 
 */
int SaveImage::saveColorPPM(FILE* file,
                             const GLubyte pixels[],
                             const int width, const int height)
{
    if ( file==0 || ferror(file) )
        return 1;
    
    fprintf(file, "P6\n");
    fprintf(file, "%i %i\n", width, height);
    fprintf(file, "255\n");
    
    //write the pixels binary, line by line:
    for ( int ii = height-1; ii >= 0; --ii )
        fwrite(&pixels[3*ii*width], 1, 3*width, file);
    return 0;
}



//------------------------------------------------------------------------------
//-------------------------------- PNG FORMAT ----------------------------------
//------------------------------------------------------------------------------

#ifndef HAS_PNG

int SaveImage::savePNG(FILE*, void* pixels,
                       const int, const int,
                       const int, const int)
{
    fprintf(stderr, "PNG format not supported (recompilation needed)\n");
    return -1;
}

#else

#include <png.h>

/*
int SaveImage::readPNG(FILE* fp, png_bytep *& row_pointers, int& bit_depth, int& color_mode, int& width, int& height)
{
    if (!fp)
        return 9;
    
    char header[8];    // 8 is the maximum size that can be checked
    fread(header, 1, 8, fp);
    
    if (png_sig_cmp(header, 0, 8))
        return 8;
    
    // initialize stuff 
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    
    if (!png_ptr)
        return 7;
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    
    if (!info_ptr)
        return 6;
    
    if (setjmp(png_jmpbuf(png_ptr)))
        return 5;
    
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    
    png_read_info(png_ptr, info_ptr);
    
    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    
    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    
    int number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);
    
    
    // read file
    if (setjmp(png_jmpbuf(png_ptr)))
        return 4;
    
    if ( row_pointers )
        free(row_pointers);
    
    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    
    for (y=0; y<height; y++)
        row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
    
    png_read_image(png_ptr, row_pointers);
}
*/


int savePNG(FILE* file,
            png_bytep row_pointers[],
            const int bit_depth, const int nb_colors,
            const int width, const int height)
{
    if ( bit_depth != 8 && bit_depth != 16 )
        return 9;
    
    int color_type = -1;
    
    if ( nb_colors == 1 )
        color_type = PNG_COLOR_TYPE_GRAY;
    
    if ( nb_colors == 3 )
        color_type = PNG_COLOR_TYPE_RGB;
    
    if ( nb_colors == 4 )
        color_type = PNG_COLOR_TYPE_RGBA;
    
    if ( color_type < 0 )
        return 8;
    
    /* initialize stuff */
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    
    if (!png_ptr)
        return 7;
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        return 6;
    
    if (setjmp(png_jmpbuf(png_ptr)))
        return 5;
    
    png_init_io(png_ptr, file);
    
    /* write header */
    if (setjmp(png_jmpbuf(png_ptr)))
        return 4;
    
    png_set_IHDR(png_ptr, info_ptr, width, height,
                 bit_depth, color_type,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    
    png_write_info(png_ptr, info_ptr);
    
    /* write bytes */
    if (setjmp(png_jmpbuf(png_ptr)))
        return 3;
    
    if ( nb_colors == 1 && bit_depth == 16 )
        png_set_swap(png_ptr);
    
    png_write_image(png_ptr, row_pointers);
    
    
    /* end write */
    if (setjmp(png_jmpbuf(png_ptr)))
        return 2;
    
    png_write_end(png_ptr, NULL);
 
    return 0;
}


int SaveImage::savePNG(FILE* file, void* pixels,
                       const int bit_depth, const int nb_colors,
                       const int width, const int height)
{
    png_bytep * rows = new png_bytep[height];
    
    int bytes_per_row = ( bit_depth / 8 ) * nb_colors * width;
    
    png_byte * start = (png_byte*)pixels;
    
    for ( int y = 0; y < height; ++y )
        rows[y] = start + bytes_per_row * ( height-y-1 );
    
    int res = savePNG(file, rows, bit_depth, nb_colors, width, height);
    
    delete[] rows;

    return res;
}

#endif


/**
 Save RGBA image, 4 x 8-bits per pixel
 */
int SaveImage::saveAlphaPNG(FILE* file,  void* pixels,
                           const int width, const int height)
{
    return savePNG(file, pixels, 8, 4, width, height);
}

/**
 Save RGB image, 3 x 8-bits per pixel
 */
int SaveImage::saveColorPNG(FILE* file,  void* pixels,
                            const int width, const int height)
{
    return savePNG(file, pixels, 8, 3, width, height);
}

/**
 Save 16-bits gray-level image
 */
int SaveImage::saveGrayPNG(FILE* file,  void* pixels,
                           const int width, const int height)
{    
    return savePNG(file, pixels, 16, 1, width, height);
}

