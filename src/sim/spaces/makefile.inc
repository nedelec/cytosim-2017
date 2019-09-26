# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

OBJ_SPACE=modulo.o space.o space_prop.o space_square.o space_sphere.o \
          space_dice.o space_torus.o space_polygon.o space_banana.o\
          space_ellipse.o space_cylinder.o space_cylinderZ.o space_capsule.o \
          space_strip.o space_periodic.o space_cylinderP.o space_dynamic_ellipse.o

OBJ_SPACE1D=$(OBJ_SPACE:.o=1D.o)
OBJ_SPACE2D=$(OBJ_SPACE:.o=2D.o)
OBJ_SPACE3D=$(OBJ_SPACE:.o=3D.o)

OBJ_SPACEGL=$(OBJ_SPACE:.o=GL.o)
OBJ_SPACE1DGL=$(OBJ_SPACE1D:.o=GL.o)
OBJ_SPACE2DGL=$(OBJ_SPACE2D:.o=GL.o)
OBJ_SPACE3DGL=$(OBJ_SPACE3D:.o=GL.o)

#----------------------------targets--------------------------------------------

libcytospace.a: $(OBJ_SPACE)
	$(MAKELIB)
	$(DONE)


libcytospace1D.a: $(OBJ_SPACE1D)
	$(MAKELIB)
	$(DONE)


libcytospace2D.a: $(OBJ_SPACE2D)
	$(MAKELIB)
	$(DONE)


libcytospace3D.a: $(OBJ_SPACE3D)
	$(MAKELIB)
	$(DONE)



#library with display
libcytospaceGL.a: $(OBJ_SPACEGL)
	$(MAKELIB)
	$(DONE)


#1D library with display
libcytospace1DGL.a: $(OBJ_SPACE1DGL)
	$(MAKELIB)
	$(DONE)


#2D library with display
libcytospace2DGL.a: $(OBJ_SPACE2DGL)
	$(MAKELIB)
	$(DONE)


#3D library with display
libcytospace3DGL.a: $(OBJ_SPACE3DGL)
	$(MAKELIB)
	$(DONE)


#----------------------------rules----------------------------------------------
INC_SPACE=$(addprefix -Isrc/, SFMT math base sim sim/spaces gl)

$(OBJ_SPACE): %.o: %.cc %.h
	$(COMPILE) -c $(INC_SPACE) $< -o build/$@

$(OBJ_SPACE1D): %1D.o: %.cc %.h
	$(COMPILE) -c -DDIM=1 $(INC_SPACE) $< -o build/$@

$(OBJ_SPACE2D): %2D.o: %.cc %.h 
	$(COMPILE) -c -DDIM=2 $(INC_SPACE) $< -o build/$@

$(OBJ_SPACE3D): %3D.o: %.cc %.h
	$(COMPILE) -c -DDIM=3 $(INC_SPACE) $< -o build/$@


$(OBJ_SPACEGL): %GL.o: %.cc %.h
	$(COMPILE) -c -DDISPLAY $(INC_SPACE) $< -o build/$@

$(OBJ_SPACE1DGL): %1DGL.o: %.cc %.h
	$(COMPILE) -c -DDIM=1 -DDISPLAY $(INC_SPACE) $< -o build/$@

$(OBJ_SPACE2DGL): %2DGL.o: %.cc %.h
	$(COMPILE) -c -DDIM=2 -DDISPLAY $(INC_SPACE) $< -o build/$@

$(OBJ_SPACE3DGL): %3DGL.o: %.cc %.h
	$(COMPILE) -c -DDIM=3 -DDISPLAY $(INC_SPACE) $< -o build/$@

