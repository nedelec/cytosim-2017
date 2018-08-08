# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
#
# File src/gl/makefile.inc

OBJ_GL=gle.o gle_color_int.o gle_color_float.o gle_color.o gle_color_list.o\
       glapp_prop.o view.o view_prop.o glapp.o 


#------------------------Flag for lib-png if requested--------------------------

IMAGE_LIB=
IMAGE_DEF=

ifneq ($(HAS_PNG), 0)

    IMAGE_DEF += -DHAS_PNG $(INC_PNG)
    IMAGE_LIB += $(LIB_PNG)

endif

ifneq ($(HAS_GIF), 0)

    IMAGE_DEF += -DHAS_GIF $(INC_GIF)
	IMAGE_LIB += $(LIB_GIF)

endif


#----------------------------targets--------------------------------------------


build/libcytogl.a: $(OBJ_GL) build/offscreen.o build/saveimage.o
	$(MAKELIB) $@ $(addprefix build/, $(notdir $^))
	$(DONE)


build/saveimage.o: saveimage.cc saveimage.h
	$(CXXFLAGS) -c $(IMAGE_DEF) $< -o $@


build/offscreen.o: offscreen.cc offscreen.h
	$(CXXFLAGS) -c $< -o $@


$(OBJ_GL): %.o: %.cc %.h
	$(CXXFLAGS) -c $(addprefix -Isrc/, SFMT math base) $< -o build/$@


