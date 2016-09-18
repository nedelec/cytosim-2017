# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


OBJ_PLAY := fiber_disp.o line_disp.o point_disp.o display_prop.o\
            display.o display1.o display2.o display3.o\
            sim_thread.o frame_reader.o play_prop.o

FILES_PLAY := play.cc play.h play_disp.cc play_keys.cc play_sim.cc play_mouse.cc play_menus.cc


OBJ_PLAYND := $(addprefix $(BLDDIR)/, $(OBJ_PLAY))
OBJ_PLAY1D := $(addprefix $(BLDDIR)/, $(OBJ_PLAY:.o=1D.o))
OBJ_PLAY2D := $(addprefix $(BLDDIR)/, $(OBJ_PLAY:.o=2D.o))
OBJ_PLAY3D := $(addprefix $(BLDDIR)/, $(OBJ_PLAY:.o=3D.o))

#----------------------------------------------------------------------

PLAY_DEP  := $(BLDDIR)/libcytogl.a $(BLDDIR)/libcytomath.a $(BLDDIR)/libcytobase.a
PLAY_LINK := $(IMAGE_LIB) $(LINKGL) $(LINK)
PLAY_INC  := $(addprefix -I$(SRCDIR)/, SFMT math base sim gl play sim/organizers sim/singles)



$(BLDDIR)/play.o: $(FILES_PLAY) | $(BLDDIR)
	$(CXXFLAGS) -c $(PLAY_INC) $< -o $@

$(OBJ_PLAYND): $(BLDDIR)/%.o: %.cc %.h
	$(CXXFLAGS) -c $(PLAY_INC) $< -o $@

play: $(BLDDIR)/play.o $(OBJ_PLAYND) $(BLDDIR)/libcytosim.a $(BLDDIR)/libcytospaceGL.a $(PLAY_DEP)
	$(CXXFLAGS) $^ $(PLAY_LINK) -o $(BINDIR)/$@
	$(DONE)
vpath play $(BINDIR)



$(BLDDIR)/play1D.o: $(FILES_PLAY) | $(BLDDIR)
	$(CXXFLAGS) -c -DDIM=1 $(PLAY_INC) $< -o $@

$(OBJ_PLAY1D): $(BLDDIR)/%1D.o: %.cc %.h
	$(CXXFLAGS) -c -DDIM=1 $(PLAY_INC) $< -o $@

play1: $(BLDDIR)/play1D.o $(OBJ_PLAY1D) $(BLDDIR)/libcytosim1D.a $(BLDDIR)/libcytospace1DGL.a $(PLAY_DEP)
	$(CXXFLAGS) $^ $(PLAY_LINK) -o $(BINDIR)/$@
	$(DONE)
vpath play1 $(BINDIR)



$(BLDDIR)/play2D.o: $(FILES_PLAY) | $(BLDDIR)
	$(CXXFLAGS) -c -DDIM=2 $(PLAY_INC) $< -o $@

$(OBJ_PLAY2D): $(BLDDIR)/%2D.o: %.cc %.h
	$(CXXFLAGS) -c -DDIM=2 $(PLAY_INC) $< -o $@

play2: $(BLDDIR)/play2D.o $(OBJ_PLAY2D) $(BLDDIR)/libcytosim2D.a $(BLDDIR)/libcytospace2DGL.a $(PLAY_DEP)
	$(CXXFLAGS) $^ $(PLAY_LINK) -o $(BINDIR)/$@
	$(DONE)
vpath play2 $(BINDIR)



$(BLDDIR)/play3D.o: $(FILES_PLAY) | $(BLDDIR)
	$(CXXFLAGS) -c -DDIM=3 $(PLAY_INC) $< -o $@

$(OBJ_PLAY3D): $(BLDDIR)/%3D.o: %.cc %.h
	$(CXXFLAGS) -c -DDIM=3 $(PLAY_INC) $< -o $@

play3: $(BLDDIR)/play3D.o $(OBJ_PLAY3D) $(BLDDIR)/libcytosim3D.a $(BLDDIR)/libcytospace3DGL.a $(PLAY_DEP)
	$(CXXFLAGS) $^ $(PLAY_LINK) -o $(BINDIR)/$@
	$(DONE)
vpath play3 $(BINDIR)


#------------------------------------------------------------------------

$(DEPDIR)/part5.dep:
	$(MAKEDEP)  $(wildcard $(SRCDIR)/play/*.cc)  >  $@
