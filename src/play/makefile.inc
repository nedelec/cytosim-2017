# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


OBJ_PLAY := fiber_disp.o line_disp.o point_disp.o display_prop.o\
            display.o display1.o display2.o display3.o\
            sim_thread.o frame_reader.o play_prop.o

FILES_PLAY := play.cc play.h play_disp.cc play_keys.cc play_sim.cc play_mouse.cc play_menus.cc


OBJ_PLAYND := $(addprefix build/, $(OBJ_PLAY))
OBJ_PLAY1D := $(addprefix build/, $(OBJ_PLAY:.o=1D.o))
OBJ_PLAY2D := $(addprefix build/, $(OBJ_PLAY:.o=2D.o))
OBJ_PLAY3D := $(addprefix build/, $(OBJ_PLAY:.o=3D.o))

#----------------------------------------------------------------------

PLAY_DEP  := build/libcytogl.a build/libcytomath.a build/libcytobase.a
PLAY_LINK := $(IMAGE_LIB) $(LINKGL) $(LINK)
PLAY_INC  := $(addprefix -Isrc/, SFMT math base sim gl play sim/organizers sim/singles)



build/play.o: $(FILES_PLAY) | build
	$(CXXFLAGS) -c $(PLAY_INC) $< -o $@

$(OBJ_PLAYND): build/%.o: %.cc %.h
	$(CXXFLAGS) -c $(PLAY_INC) $< -o $@

play: build/play.o $(OBJ_PLAYND) build/libcytosim.a build/libcytospaceGL.a $(PLAY_DEP) | bin
	$(CXXFLAGS) $^ $(PLAY_LINK) -o bin/$@
	$(DONE)
vpath play bin



build/play1D.o: $(FILES_PLAY) | build
	$(CXXFLAGS) -c -DDIM=1 $(PLAY_INC) $< -o $@

$(OBJ_PLAY1D): build/%1D.o: %.cc %.h
	$(CXXFLAGS) -c -DDIM=1 $(PLAY_INC) $< -o $@

play1: build/play1D.o $(OBJ_PLAY1D) build/libcytosim1D.a build/libcytospace1DGL.a $(PLAY_DEP)
	$(CXXFLAGS) $^ $(PLAY_LINK) -o bin/$@
	$(DONE)
vpath play1 bin



build/play2D.o: $(FILES_PLAY) | build
	$(CXXFLAGS) -c -DDIM=2 $(PLAY_INC) $< -o $@

$(OBJ_PLAY2D): build/%2D.o: %.cc %.h
	$(CXXFLAGS) -c -DDIM=2 $(PLAY_INC) $< -o $@

play2: build/play2D.o $(OBJ_PLAY2D) build/libcytosim2D.a build/libcytospace2DGL.a $(PLAY_DEP)
	$(CXXFLAGS) $^ $(PLAY_LINK) -o bin/$@
	$(DONE)
vpath play2 bin



build/play3D.o: $(FILES_PLAY) | build
	$(CXXFLAGS) -c -DDIM=3 $(PLAY_INC) $< -o $@

$(OBJ_PLAY3D): build/%3D.o: %.cc %.h
	$(CXXFLAGS) -c -DDIM=3 $(PLAY_INC) $< -o $@

play3: build/play3D.o $(OBJ_PLAY3D) build/libcytosim3D.a build/libcytospace3DGL.a $(PLAY_DEP)
	$(CXXFLAGS) $^ $(PLAY_LINK) -o bin/$@
	$(DONE)
vpath play3 bin


