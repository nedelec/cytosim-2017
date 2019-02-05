# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


TESTS:=test test_solve test_random test_math test_param test_quaternion\
       test_thread test_sizeof test_blas test_simd


TESTS_GL:=test_glapp test_rasterizer test_space test_grid test_sphere


.PHONY: tests
tests: $(TESTS) $(TESTS_GL)


vpath %.cc src/test


#--------------------macros----------------------------------------------------


TEST_OBJ=$(filter %.cc, $^) $(addprefix build/, $(notdir $(filter-out %.cc, $^)))

TEST_INC=$(addprefix -Isrc/, SFMT math base sim sim/spaces sim/fibers sim/singles sim/organizers gl play)

TEST_MAKE=$(COMPILE) $(TEST_INC) $(TEST_OBJ) $(LINK) -o bin/$@

TEST_MAKE_GL=$(COMPILE) $(TEST_INC) $(TEST_OBJ) $(LINK) $(LINKGL) -o bin/$@


#----------------------------targets--------------------------------------------

test: test.cc random.o SFMT.o tictoc.o backtrace.o
	$(TEST_MAKE)
	$(DONE)
vpath test bin

test_blas: test_blas.cc random.o SFMT.o backtrace.o
	$(TEST_MAKE)
	$(DONE)
vpath test_blas bin

test_param: test_param.cc glossary.o tokenizer.o stream_func.o exceptions.o backtrace.o filepath.o
	$(TEST_MAKE)
	$(DONE)
vpath test_param bin

test_quartic: test_quartic.cc quartic.o random.o SFMT.o backtrace.o
	$(TEST_MAKE)
	$(DONE)
vpath test_quartic bin

test_quaternion: test_quaternion.cc libcytomath.a SFMT.o libcytobase.a
	$(TEST_MAKE)
	$(DONE)
vpath test_quaternion bin

test_random: test_random.cc random.o SFMT.o exceptions.o filewrapper.o messages.o smath.o vecprint.o backtrace.o tictoc.o
	$(TEST_MAKE)
	$(DONE)
vpath test_random bin

test_simd: test_simd.cc tictoc.o
	$(COMPILE) -Isrc/base src/base/tictoc.cc $< -o bin/test_simd
	$(DONE)
vpath test_simd bin

test_sizeof: test_sizeof.cc
	$(TEST_MAKE)
	$(DONE)
vpath test_sizeof bin

test_cxx: test_cxx.cc
	$(TEST_MAKE)
	$(DONE)
vpath test_cxx bin

test_math: test_math.cc
	$(TEST_MAKE)
	$(DONE)
vpath test_math bin

test_thread: test_thread.cc
	$(TEST_MAKE)
	$(DONE)
vpath test_thread bin

test_string: test_string.cc
	$(TEST_MAKE)
	$(DONE)
vpath test_string bin

#----------------------------graphics targets-----------------------------------

test_opengl: test_opengl.cc
	$(CXX) $^ $(LINKGL) -o bin/$@
vpath test_opengl bin

test_glut: test_glut.cc
	$(CXX) $^ $(LINKGL) -o bin/$@
vpath test_glut bin

test_glapp: test_glapp.cc libcytogl.a libcytobase.a libcytomath.a
	$(TEST_MAKE_GL)
vpath test_glapp bin

test_sphere: test_sphere.cc libcytogl.a libcytomath.a libcytobase.a
	$(TEST_MAKE_GL)
vpath test_sphere bin

test_rasterizer: test_rasterizer.cc rasterizerGL.o libcytomath.a libcytogl.a libcytobase.a
	$(TEST_MAKE_GL)
vpath test_rasterizer bin

test_space: test_space.cc libcytospace.a libcytosim.a libcytomath.a libcytogl.a libcytobase.a
	$(TEST_MAKE_GL)
vpath test_space bin

test_grid: test_grid.cc tictoc.o libcytomath.a libcytobase.a libcytogl.a
	$(TEST_MAKE_GL)
vpath test_grid bin

test_solve: test_solve.cc libcytomath.a libcytobase.a
	$(TEST_MAKE_GL)
vpath test_solve bin
