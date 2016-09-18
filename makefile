# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

# THIS FILE SHOULD NOT BE EDITED: EDIT FILE makefile.inc 


SRCDIR := src
BINDIR := bin
DEPDIR := dep
BLDDIR := build

#include the compiler-specifications
include makefile.inc

#-----------------check the compilers and the compiler-flags--------------------

ifeq ($CXX,)
   $(error No compiler defined for $$(MACHINE)=$(MACHINE))
endif

ifeq ($(Flags$(MODE)),)
   $(error No compiler-options defined for $$(MACHINE)=$(MACHINE) in mode $(MODE))
endif

CXXFLAGS := $(CXX) $(Flags$(MODE))

#-----------------prepare some useful variables---------------------------------

ifndef LINK
  $(error No linkage-options defined for $$(MACHINE)=$(MACHINE)!)
endif


SRCDIR1   := $(addprefix $(SRCDIR)/, math base sim gl play doc)
SRCDIR2   := $(addprefix $(SRCDIR)/sim/, spaces hands fibers singles couples organizers)
ALLSRCDIR := $(SRCDIR1) $(SRCDIR2)


#command used to build the dependencies files automatically
MAKEDEP := gcc -MM $(addprefix -I, $(ALLSRCDIR) $(SRCDIR)/SFMT)

#----------------------------paths----------------------------------------------

vpath %.h   $(ALLSRCDIR)
vpath %.cc  $(ALLSRCDIR)
vpath %.o   $(BLDDIR)
vpath %.a   $(BLDDIR)
vpath %.dep $(DEPDIR)
vpath SFMT% $(SRCDIR)/SFMT

#----------------------------targets--------------------------------------------

# calling 'make' without arguments will make sim and play:
.PHONY: simplay
simplay: sim play


include $(SRCDIR)/sim/makefile.inc
include $(SRCDIR)/base/makefile.inc
include $(SRCDIR)/math/makefile.inc
include $(SRCDIR)/sim/spaces/makefile.inc
include $(SRCDIR)/gl/makefile.inc
include $(SRCDIR)/play/makefile.inc

-include $(SRCDIR)/tools/makefile.inc
-include $(SRCDIR)/test/makefile.inc


# Attention: Mersenne-Twister is coded in C-language,
# and we must provide this with '-x c' to the compiler:
SFMT.o: SFMT.c SFMT.h
	$(CXXFLAGS) -x c -DNDEBUG -DSFMT_MEXP=19937 -c $< -o $(BLDDIR)/$@


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
        --exclude $(BINDIR)/\* --exclude $(BLDDIR)/\*  *


cytosim.tar.bz2: cytosim.tar
	rm -f cytosim.tar.bz2;
	bzip2 cytosim.tar;


cytosim_src.tar:
	tar cf cytosim_src.tar --exclude "*.o" --exclude ".*" \
	--exclude ".svn" --exclude "*~" src makefile makefile.inc cym python


tar: cytosim_src.tar cytosim.tar.bz2


#---------------------------- maintenance ----------------------------------------
.PHONY: ready clean cleaner sterile ts


$(BLDDIR):
	mkdir $(BLDDIR)

$(BINDIR):
	mkdir $(BINDIR)


ready: | $(BINDIR) $(BLDDIR) dep
	rm -rf $(BLDDIR)/*
	rm -f $(BINDIR)/*


clean:
	rm -f $(BLDDIR)/*.o $(BLDDIR)/*.a


cleaner:
	rm -f *.cmo $(BLDDIR)/*.o $(BLDDIR)/*.a $(DEPDIR)/*.dep;


sterile:
	rm -rf $(BLDDIR)/*
	rm -f  $(DEPDIR)/*
	rm -rf $(BINDIR)/*.dSYM
	rm -f  $(BINDIR)/*
	rm -f *.cmo


ts: sterile tar

#---------------------------- dependencies ----------------------------------------

depdir:
	if ! test -d $(DEPDIR); then mkdir $(DEPDIR); fi

dep: depdir | $(addprefix $(DEPDIR)/, part0.dep part1.dep part2.dep part3.dep part4.dep part5.dep part6.dep part7.dep)


-include $(DEPDIR)/part?.dep



