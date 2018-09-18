# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.



OBJ_PLAYER := fiber_disp.o line_disp.o point_disp.o display_prop.o\
              display.o display1.o display2.o display3.o\
              sim_thread.o frame_reader.o play_prop.o play.o

FILES_PLAYER := play.h play.cc play_disp.cc play_keys.cc play_mouse.cc play_menus.cc


OBJ_PLAYER1D := $(OBJ_PLAYER:.o=1D.o)
OBJ_PLAYER2D := $(OBJ_PLAYER:.o=2D.o)
OBJ_PLAYER3D := $(OBJ_PLAYER:.o=3D.o)

#----------------------------------------------------------------------

PLAY_LINK := $(IMAGE_LIB) $(LINKGL) $(LINK)
PLAY_INC  := $(addprefix -Isrc/, SFMT math base sim gl play sim/organizers sim/singles)


# need to fix the paths of libraries:
PLAY_OBJS=$(filter %.cc, $^) $(addprefix build/, $(notdir $(filter-out %.cc, $^)))

$(OBJ_PLAYER): %.o: %.cc %.h | build
	$(COMPILE) -c $(PLAY_INC) $< -o build/$@

play: $(OBJ_PLAYER) libcytosim.a libcytospaceGL.a libcytogl.a libcytomath.a libcytobase.a | bin
	$(COMPILE) $(PLAY_OBJS) $(PLAY_LINK) -o bin/play
	$(DONE)
vpath play bin



$(OBJ_PLAYER1D): %1D.o: %.cc %.h | build
	$(COMPILE) -c -DDIM=1 $(PLAY_INC) $< -o build/$@

bin1/play: $(OBJ_PLAYER1D) libcytosim1D.a libcytospace1DGL.a libcytogl.a libcytomath1D.a libcytobase.a | bin
	$(COMPILE) $(PLAY_OBJS) $(PLAY_LINK) -o bin1/play
	$(DONE)
vpath bin1/play bin1



$(OBJ_PLAYER2D): %2D.o: %.cc %.h | build
	$(COMPILE) -c -DDIM=2 $(PLAY_INC) $< -o build/$@

bin2/play: $(OBJ_PLAYER2D) libcytosim2D.a libcytospace2DGL.a libcytogl.a libcytomath2D.a libcytobase.a | bin
	$(COMPILE) $(PLAY_OBJS) $(PLAY_LINK) -o bin2/play
	$(DONE)
vpath bin2/play bin2



$(OBJ_PLAYER3D): %3D.o: %.cc %.h | build
	$(COMPILE) -c -DDIM=3 $(PLAY_INC) $< -o build/$@

bin3/play: $(OBJ_PLAYER3D) libcytosim3D.a libcytospace3DGL.a libcytogl.a libcytomath3D.a libcytobase.a | bin
	$(COMPILE) $(PLAY_OBJS) $(PLAY_LINK) -o bin3/play
	$(DONE)
vpath bin3/play bin3

