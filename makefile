# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

# THIS FILE SHOULD NOT BE EDITED: EDIT FILE makefile.inc 

#include the compiler-specifications
include makefile.inc

#-----------------check the compilers and the compiler-flags--------------------

ifeq ($CXX,)
   $(error No compiler defined for $$(MACHINE)=$(MACHINE))
endif

ifeq ($(Flags$(MODE)),)
   $(error No compiler-options defined for $$(MACHINE)=$(MACHINE) in mode $(MODE))
endif

#command to compile:
COMPILE := $(CXX) $(Flags$(MODE))

#command to make a library:
MAKELIB = $(LIBTOOL) build/$@ $(addprefix build/, $(notdir $^))

#-----------------prepare some useful variables---------------------------------

ifndef LINK
  $(error No linkage-options defined for $$(MACHINE)=$(MACHINE)!)
endif


SRCDIR1   := $(addprefix src/, math base sim gl play doc)
SRCDIR2   := $(addprefix src/sim/, spaces hands fibers singles couples organizers)
ALLSRCDIR := $(SRCDIR1) $(SRCDIR2)


#command used to build the dependencies files automatically
MAKEDEP := gcc -MM $(addprefix -I, $(ALLSRCDIR) src/SFMT)

#----------------------------paths----------------------------------------------

vpath %.h   $(ALLSRCDIR)
vpath %.cc  $(ALLSRCDIR)
vpath %.o   build
vpath %.a   build
vpath %.dep dep
vpath SFMT% src/SFMT

#----------------------------targets--------------------------------------------

# calling 'make' without arguments will make sim and play:
.PHONY: simplay
simplay: sim report play


include src/sim/makefile.inc
include src/base/makefile.inc
include src/math/makefile.inc
include src/sim/spaces/makefile.inc
include src/gl/makefile.inc
include src/play/makefile.inc

-include src/tools/makefile.inc
-include src/test/makefile.inc


# Attention: Mersenne-Twister is coded in C-language,
# and we must provide this with '-x c' to the compiler:
SFMT.o: SFMT.c SFMT.h
	$(COMPILE) -x c -DNDEBUG -DSFMT_MEXP=19937 -c $< -o build/$@


.PHONY: all
all: sim play tools tests


.PHONY: alldim
alldim: sim1 sim2 sim3 play1 play2 play3


doc:
	if test -d html; then rm -rf html; fi
	mkdir html;
	doxygen src/doc/doxygen.cfg > log.txt

#------------------------------- archive -----------------------------------------
.PHONY: cytosim.tar cytosim.tar.bz2 cytosim_src.tar tar


cytosim.tar:
	tar cf cytosim.tar --exclude \*.cmo --exclude \*tar\* \
        --exclude bin/\* --exclude build/\*  *


cytosim.tar.bz2: cytosim.tar
	rm -f cytosim.tar.bz2;
	bzip2 cytosim.tar;


cytosim_src.tar:
	tar cf cytosim_src.tar --exclude "*.o" --exclude ".*" \
	--exclude ".svn" --exclude "*~" src makefile makefile.inc cym 


tar: cytosim_src.tar cytosim.tar.bz2


#---------------------------- maintenance ----------------------------------------
.PHONY: bin build dep clean cleaner sterile


bin:
	if ! test -d bin; then mkdir bin; fi

build:
	if ! test -d build; then mkdir build; fi


clean:
	rm -f build/*.o build/*.a


cleaner:
	rm -f *.cmo build/*.o build/*.a dep/*.dep;


sterile:
	rm -rf build/*
	rm -f  dep/*.dep
	rm -rf bin/*.dSYM
	rm -f  bin/*
	rm -f *.cmo
	rm -f log.txt;

#---------------------------- dependencies ----------------------------------------

dep:
	if ! test -d dep; then mkdir dep; else rm -f dep/*; fi
	$(foreach file, $(wildcard src/base/*.cc),  $(MAKEDEP) $(file) >> dep/part0.dep; )
	$(foreach file, $(wildcard src/math/*.cc),  $(MAKEDEP) $(file) >> dep/part1.dep; )
	$(foreach file, $(wildcard src/sim/*.cc),   $(MAKEDEP) $(file) >> dep/part2.dep; )
	$(foreach file, $(wildcard src/sim/*/*.cc), $(MAKEDEP) $(file) >> dep/part3.dep; )
	$(foreach file, $(wildcard src/disp/*.cc),  $(MAKEDEP) $(file) >> dep/part4.dep; )
	$(foreach file, $(wildcard src/play/*.cc),  $(MAKEDEP) $(file) >> dep/part5.dep; )
	$(foreach file, $(wildcard src/tools/*.cc), $(MAKEDEP) $(file) >> dep/part6.dep; )
	$(foreach file, $(wildcard src/test/*.cc),  $(MAKEDEP) $(file) >> dep/part7.dep; )


-include dep/part?.dep

