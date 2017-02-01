# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

OBJ_HANDS := hand.o hand_prop.o\
             motor.o motor_prop.o\
             slider.o slider_prop.o\
             actor.o actor_prop.o\
             nucleator.o nucleator_prop.o\
             tracker.o tracker_prop.o\
             cutter.o cutter_prop.o\
             rescuer.o rescuer_prop.o\
             mighty.o mighty_prop.o\
         
OBJ_FIBERS := fiber_naked.o fiber.o rigid_fiber.o fiber_prop.o fiber_set.o\
              dynamic_fiber.o dynamic_fiber_prop.o\
              classic_fiber.o classic_fiber_prop.o\
              treadmilling_fiber.o treadmilling_fiber_prop.o\
              tubule.o tubule_prop.o

OBJ_SINGLES := single.o single_prop.o single_set.o\
               picket.o picket_long.o wrist.o wrist_long.o

OBJ_COUPLES := couple.o couple_prop.o couple_set.o couple_long.o\
               crosslink.o crosslink_long.o crosslink_prop.o\
               bridge.o bridge_prop.o\
               shackle.o shackle_long.o shackle_prop.o\
               fork.o fork_prop.o

OBJ_ORGANIZERS := organizer.o organizer_set.o \
                  aster.o aster_prop.o nucleus.o nucleus_prop.o\
                  fake.o fake_prop.o bundle.o bundle_prop.o \


OBJ_SIM := movable.o mecable.o point_set.o\
           point_exact.o point_interpolated.o fiber_locus.o \
           object.o object_set.o common.o fiber_binder.o\
           sphere_prop.o sphere.o sphere_set.o \
           bead_prop.o bead.o bead_set.o\
           solid_prop.o solid.o solid_set.o\
           meca.o simul_prop.o fiber_grid.o point_grid.o\
           field.o field_prop.o field_set.o space_set.o\
           simul.o interface.o parser.o\
        

OBJ_CYTOSIM:=$(OBJ_SIM) $(OBJ_HANDS) $(OBJ_FIBERS) $(OBJ_SINGLES) $(OBJ_COUPLES) $(OBJ_ORGANIZERS)

OBJ_CYTOSIM_1D:=$(OBJ_CYTOSIM:.o=1D.o)
OBJ_CYTOSIM_2D:=$(OBJ_CYTOSIM:.o=2D.o)
OBJ_CYTOSIM_3D:=$(OBJ_CYTOSIM:.o=3D.o)

#----------------------------targets--------------------------------------------

#--- normal build:

sim: sim.cc $(addprefix build/, libcytosim.a libcytospace.a libcytomath.a libcytobase.a) | bin
	$(CXXFLAGS) $(INC_SIM) $^ $(LINK) -o bin/$@
	$(DONE)
vpath sim  bin


#--- build with debuging flags:

simd: sim.cc $(addprefix build/, libcytosim.a libcytospace.a libcytomath.a libcytobase.a) | bin
	$(CXX) $(FlagsD) $(INC_SIM) $^ $(LINK) -o bin/$@
	$(DONE)
vpath simd  bin


#--- builds with specific dimension:

sim1: sim.cc $(addprefix build/, libcytosim1D.a libcytospace1D.a libcytomath.a libcytobase.a)
	$(CXXFLAGS) $(INC_SIM) -DDIM=1 $^ $(LINK) -o bin/$@
	$(DONE)
vpath sim1  bin

sim2: sim.cc $(addprefix build/, libcytosim2D.a libcytospace2D.a libcytomath.a libcytobase.a)
	$(CXXFLAGS) $(INC_SIM) -DDIM=2 $^ $(LINK) -o bin/$@
	$(DONE)
vpath sim2  bin

sim3: sim.cc $(addprefix build/, libcytosim3D.a libcytospace3D.a libcytomath.a libcytobase.a)
	$(CXXFLAGS) $(INC_SIM) -DDIM=3 $^ $(LINK) -o bin/$@
	$(DONE)
vpath sim3  bin



build/libcytosim.a: $(OBJ_CYTOSIM)
	$(MAKELIB) $@ $(addprefix build/, $(notdir $^))
	$(DONE)


build/libcytosim1D.a: $(OBJ_CYTOSIM_1D)
	$(MAKELIB) $@ $(addprefix build/, $(notdir $^))
	$(DONE)


build/libcytosim2D.a: $(OBJ_CYTOSIM_2D)
	$(MAKELIB) $@ $(addprefix build/, $(notdir $^))
	$(DONE)


build/libcytosim3D.a: $(OBJ_CYTOSIM_3D)
	$(MAKELIB) $@ $(addprefix build/, $(notdir $^))
	$(DONE)


#----------------------------rules----------------------------------------------

INC_SIM=$(addprefix -Isrc/, SFMT math base sim sim/fibers sim/hands sim/singles sim/couples sim/organizers sim/spaces)


$(OBJ_CYTOSIM): %.o: %.cc %.h | build
	$(CXXFLAGS) -c $(INC_SIM) $< -o build/$@

$(OBJ_CYTOSIM_1D): %1D.o: %.cc %.h | build
	$(CXXFLAGS) -c -DDIM=1 $(INC_SIM) $< -o build/$@

$(OBJ_CYTOSIM_2D): %2D.o: %.cc %.h | build
	$(CXXFLAGS) -c -DDIM=2 $(INC_SIM) $< -o build/$@

$(OBJ_CYTOSIM_3D): %3D.o: %.cc %.h | build
	$(CXXFLAGS) -c -DDIM=3 $(INC_SIM) $< -o build/$@

