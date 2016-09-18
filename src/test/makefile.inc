# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


TESTS:=test test_solve test_random test_math test_param test_quaternion\
       test_thread test_sizeof test_blas test_simd


TESTS_GL:=test_glapp test_rasterizer test_space test_grid test_sphere


.PHONY: tests
tests: $(TESTS) $(TESTS_GL)


vpath %.cc $(SRCDIR)/test


#--------------------macros----------------------------------------------------


TEST_OBJ=$(filter %.cc, $^) $(addprefix $(BLDDIR)/, $(notdir $(filter-out %.cc, $^)))

TEST_INC=$(addprefix -I$(SRCDIR)/, SFMT math base sim sim/spaces sim/fibers sim/singles sim/organizers gl play)

TEST_MAKE=$(CXXFLAGS) $(TEST_INC) $(TEST_OBJ) $(LINK) -o $(BINDIR)/$@

TEST_MAKE_GL=$(CXXFLAGS) $(TEST_INC) $(TEST_OBJ) $(LINK) $(LINKGL) -o $(BINDIR)/$@


#----------------------------targets--------------------------------------------

test: test.cc random.o SFMT.o tictoc.o backtrace.o
	$(TEST_MAKE)
	$(DONE)
vpath test $(BINDIR)

test_blas: test_blas.cc random.o SFMT.o backtrace.o
	$(TEST_MAKE)
	$(DONE)
vpath test_blas $(BINDIR)

test_param: test_param.cc glossary.o tokenizer.o stream_func.o exceptions.o backtrace.o
	$(TEST_MAKE)
	$(DONE)
vpath test_param $(BINDIR)

test_quartic: test_quartic.cc quartic.o random.o SFMT.o backtrace.o
	$(TEST_MAKE)
	$(DONE)
vpath test_quartic $(BINDIR)

test_quaternion: test_quaternion.cc libcytomath.a SFMT.o libcytobase.a 
	$(TEST_MAKE)
	$(DONE)
vpath test_quaternion $(BINDIR)

test_random: test_random.cc random.o SFMT.o exceptions.o filewrapper.o messages.o smath.o vecprint.o backtrace.o tictoc.o
	$(TEST_MAKE)
	$(DONE)
vpath test_random $(BINDIR)

test_simd: test_simd.cc tictoc.o 
	$(CXXFLAGS) -I$(SRCDIR)/base $(SRCDIR)/base/tictoc.cc $< -o $(BINDIR)/test_simd
	$(DONE)
vpath test_simd $(BINDIR)

test_sizeof: test_sizeof.cc
	$(TEST_MAKE)
	$(DONE)
vpath test_sizeof $(BINDIR)

test_cxx: test_cxx.cc
	$(TEST_MAKE)
	$(DONE)
vpath test_cxx $(BINDIR)

test_math: test_math.cc
	$(TEST_MAKE)
	$(DONE)
vpath test_math $(BINDIR)

test_thread: test_thread.cc
	$(TEST_MAKE)
	$(DONE)
vpath test_thread $(BINDIR)

test_string: test_string.cc
	$(TEST_MAKE)
	$(DONE)
vpath test_string $(BINDIR)

#----------------------------graphics targets-----------------------------------

test_opengl: test_opengl.cc
	$(CXX) $^ $(LINKGL) -o $(BINDIR)/$@
vpath test_opengl $(BINDIR)

test_glut: test_glut.cc
	$(CXX) $^ $(LINKGL) -o $(BINDIR)/$@
vpath test_glut $(BINDIR)

test_glapp: test_glapp.cc libcytogl.a libcytobase.a libcytomath.a 
	$(TEST_MAKE_GL)
vpath test_glapp $(BINDIR)

test_sphere: test_sphere.cc libcytogl.a libcytomath.a libcytobase.a
	$(TEST_MAKE_GL)
vpath test_sphere $(BINDIR)

test_rasterizer: test_rasterizer.cc rasterizerGL.o libcytomath.a libcytogl.a libcytobase.a 
	$(TEST_MAKE_GL)
vpath test_rasterizer $(BINDIR)

test_space: test_space.cc libcytospace.a libcytosim.a libcytomath.a libcytogl.a libcytobase.a 
	$(TEST_MAKE_GL)
vpath test_space $(BINDIR)

test_grid: test_grid.cc tictoc.o libcytomath.a libcytobase.a libcytogl.a
	$(TEST_MAKE_GL)
vpath test_grid $(BINDIR)

test_solve: test_solve.cc libcytomath.a libcytobase.a
	$(TEST_MAKE_GL)
vpath test_solve $(BINDIR)

#----------------------------makedep--------------------------------------------

$(DEPDIR)/part6.dep:
	$(MAKEDEP)  $(wildcard $(SRCDIR)/test/*.cc)  >  $@


