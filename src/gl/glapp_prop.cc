// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "glapp_prop.h"
#include "glossary.h"


//------------------------------------------------------------------------------
void glAppProp::clear()
{
    full_screen    = 0;
    show_fps       = 0;

    depth_test     = 1;
    buffered       = 1;
    stencil_buffer = 0;
    multisample    = 0;
    depth_clamp    = 0;
    
    label          = "Cytosim ";
    show_message   = 0;
    message        = "Please, visit www.cytosim.org";
    message_left   = "";
    message_right  = "";
}


void glAppProp::read(Glossary& glos)
{
    glos.set(full_screen,     "full_screen");
    glos.set(label,           "label");
    glos.set(show_message,    "show_message");
    glos.set(show_fps,        "show_fps");
    glos.set(buffered,        "buffered");
    glos.set(depth_test,      "depth_test");
    glos.set(stencil_buffer,  "stencil_buffer");
    glos.set(multisample,     "multisample");
#ifdef BACKWARD_COMPATIBILITY
    glos.set(multisample,     "gl_multisample");  //synonym
#endif
    glos.set(depth_clamp,     "depth_clamp");
}
        
//------------------------------------------------------------------------------

void glAppProp::write_data(std::ostream & os) const
{
    write_param(os, "full_screen",    full_screen);
    write_param(os, "buffered",       buffered);
    write_param(os, "depth_test",     depth_test);
    write_param(os, "stencil_buffer", stencil_buffer);
    write_param(os, "multisample",    multisample);
    write_param(os, "depth_clamp",    depth_clamp);
    write_param(os, "show_fps",       show_fps);
    write_param(os, "label",          "("+label+")");
}


