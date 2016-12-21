# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

OBJ_SPACE=modulo.o space.o space_prop.o space_square.o space_sphere.o \
          space_dice.o space_torus.o space_polygon.o space_banana.o\
          space_ellipse.o space_cylinder.o space_cylinderZ.o space_capsule.o \
          space_strip.o space_periodic.o space_cylinderP.o

OBJ_SPACE1D=$(OBJ_SPACE:.o=1D.o)
OBJ_SPACE2D=$(OBJ_SPACE:.o=2D.o)
OBJ_SPACE3D=$(OBJ_SPACE:.o=3D.o)

OBJ_SPACEGL=$(OBJ_SPACE:.o=GL.o)
OBJ_SPACE1DGL=$(OBJ_SPACE1D:.o=GL.o)
OBJ_SPACE2DGL=$(OBJ_SPACE2D:.o=GL.o)
OBJ_SPACE3DGL=$(OBJ_SPACE3D:.o=GL.o)

#----------------------------targets--------------------------------------------

build/libcytospace.a: $(OBJ_SPACE)
	$(MAKELIB) $@ $(addprefix build/, $(notdir $^))
	$(DONE)


build/libcytospace1D.a: $(OBJ_SPACE1D)
	$(MAKELIB) $@ $(addprefix build/, $(notdir $^))
	$(DONE)


build/libcytospace2D.a: $(OBJ_SPACE2D)
	$(MAKELIB) $@ $(addprefix build/, $(notdir $^))
	$(DONE)


build/libcytospace3D.a: $(OBJ_SPACE3D)
	$(MAKELIB) $@ $(addprefix build/, $(notdir $^))
	$(DONE)



#library with display
build/libcytospaceGL.a: $(OBJ_SPACEGL)
	$(MAKELIB) $@ $(addprefix build/, $(notdir $^))
	$(DONE)


#1D library with display
build/libcytospace1DGL.a: $(OBJ_SPACE1DGL)
	$(MAKELIB) $@ $(addprefix build/, $(notdir $^))
	$(DONE)


#2D library with display
build/libcytospace2DGL.a: $(OBJ_SPACE2DGL)
	$(MAKELIB) $@ $(addprefix build/, $(notdir $^))
	$(DONE)


#3D library with display
build/libcytospace3DGL.a: $(OBJ_SPACE3DGL)
	$(MAKELIB) $@ $(addprefix build/, $(notdir $^))
	$(DONE)


#----------------------------rules----------------------------------------------
INC_SPACE=$(addprefix -Isrc/, SFMT math base sim sim/spaces gl)

$(OBJ_SPACE): %.o: %.cc %.h
	$(CXXFLAGS) -c $(INC_SPACE) $< -o build/$@

$(OBJ_SPACE1D): %1D.o: %.cc %.h
	$(CXXFLAGS) -c -DDIM=1 $(INC_SPACE) $< -o build/$@

$(OBJ_SPACE2D): %2D.o: %.cc %.h 
	$(CXXFLAGS) -c -DDIM=2 $(INC_SPACE) $< -o build/$@

$(OBJ_SPACE3D): %3D.o: %.cc %.h
	$(CXXFLAGS) -c -DDIM=3 $(INC_SPACE) $< -o build/$@


$(OBJ_SPACEGL): %GL.o: %.cc %.h
	$(CXXFLAGS) -c -DDISPLAY $(INC_SPACE) $< -o build/$@

$(OBJ_SPACE1DGL): %1DGL.o: %.cc %.h
	$(CXXFLAGS) -c -DDIM=1 -DDISPLAY $(INC_SPACE) $< -o build/$@

$(OBJ_SPACE2DGL): %2DGL.o: %.cc %.h
	$(CXXFLAGS) -c -DDIM=2 -DDISPLAY $(INC_SPACE) $< -o build/$@

$(OBJ_SPACE3DGL): %3DGL.o: %.cc %.h
	$(CXXFLAGS) -c -DDIM=3 -DDISPLAY $(INC_SPACE) $< -o build/$@

