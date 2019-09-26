// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "play_prop.h"
#include "glossary.h"
#include "saveimage.h"

//------------------------------------------------------------------------------
void PlayProp::clear()
{
    frame        = 0;
    play         = 0;
    loop         = 0;
    period       = 1;
    delay        = 32;
    live         = 0;
    style        = 1;
    
    report       = "";
    report1      = "fiber:lengths";
    report2      = "fiber:dynamics";
    report3      = "single";
    report4      = "couple";
    report5      = "fiber:segments";
    
    for ( int k = 0; k < NB_MAGIC_KEYS; ++k )
    {
        magic_key[k]  = 0;
        magic_code[k] = "";
    }
    
    if ( SaveImage::supported("png") )
        image_format = "png";
    else
        image_format = "ppm";

    image_dir    = "";
    image_index  = 0;
    poster_index = 0;
}

//------------------------------------------------------------------------------
void PlayProp::read(Glossary& glos)
{
    glos.set(frame,        "frame");
    glos.set(play,         "play");
    glos.set(loop,         "loop");
    glos.set(period,       "period");
    glos.set(delay,        "delay");
    glos.set(style,        "style");
    glos.set(image_format, "image_format");
    glos.set(image_dir,    "image_dir");
    
    if ( ! SaveImage::supported(image_format.c_str()) )
        throw InvalidParameter("unsupported image format");
    
    for ( int k = 0; k < NB_MAGIC_KEYS; ++k )
    {
        char var[16];
        snprintf(var, sizeof(var), "magic_key%u", k);
        glos.set(magic_key[k],  var);
        glos.set(magic_code[k], var, 1);
    }
    
    glos.set(report,      "report");
    glos.set(report1,     "report1");
    glos.set(report2,     "report2");
    glos.set(report3,     "report3");
    glos.set(report4,     "report4");
    glos.set(report5,     "report5");
}


//------------------------------------------------------------------------------

void PlayProp::write_data(std::ostream & os) const
{
    write_param(os, "frame",  frame);
    write_param(os, "play",   play);
    write_param(os, "loop",   loop);
    write_param(os, "period", period);
    write_param(os, "delay",  delay);
    write_param(os, "style",  style);
    write_param(os, "report", report);
    write_param(os, "image_format", image_format);
    write_param(os, "image_dir", image_dir);

    for ( int k = 0; k < NB_MAGIC_KEYS; ++k )
    {
        char var[16];
        snprintf(var, sizeof(var), "magic_key%u", k);
        write_param(os, var, magic_key[k], "("+magic_code[k]+")");
    }
}

//------------------------------------------------------------------------------

void PlayProp::toggleReport(bool alt)
{
    if ( alt )
    {
        if ( report.empty() )
            report = report5;
        else
            report.clear();
    }
    else
    {
        if ( report.empty() )          report = report1;
        else if ( report == report1 )  report = report2;
        else if ( report == report2 )  report = report3;
        else if ( report == report3 )  report = report4;
        else report.clear();
    }
}

