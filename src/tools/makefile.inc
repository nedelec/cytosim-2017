# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.



TOOLS:=frametool sieve reader report reportF analyse1 analyse2 analyse3


.PHONY: tools
tools: $(TOOLS)


vpath %.cc $(SRCDIR)/tools


#--------------------macros----------------------------------------------------

TOOL_DEP := $(addprefix $(BLDDIR)/,libcytosim.a libcytospace.a libcytomath.a libcytobase.a)

TOOL_OBJ  = $(filter %.cc, $^) $(addprefix $(BLDDIR)/, $(notdir $(filter-out %.cc, $^)))

TOOL_INC  = $(addprefix -I$(SRCDIR)/, SFMT math base sim sim/spaces sim/fibers sim/singles sim/organizers gl play)

TOOL_MAKE = $(CXXFLAGS) $(TOOL_INC) $(TOOL_OBJ) $(LINK) -o $(BINDIR)/$@


#-------------------targets----------------------------------------------------
 
 
frametool: frametool.cc
	$(TOOL_MAKE)
	$(DONE)
vpath frametool $(BINDIR)


sieve: sieve.cc frame_reader.o $(TOOL_DEP) 
	$(TOOL_MAKE)
	$(DONE)
vpath sieve $(BINDIR)


$(BLDDIR)/frame_readerVB.o: frame_reader.cc frame_reader.h
	$(CXXFLAGS) -c $(PLAY_INC) -DVERBOSE_READER $< -o $@

reader: reader.cc frame_readerVB.o $(TOOL_DEP)
	$(TOOL_MAKE)
	$(DONE)
vpath reader $(BINDIR)



report: report.cc frame_reader.o $(TOOL_DEP)
	$(TOOL_MAKE)
	$(DONE)
vpath report $(BINDIR)

reportF: reportF.cc frame_reader.o $(TOOL_DEP)
	$(TOOL_MAKE)
	$(DONE)
vpath reportF $(BINDIR)

analyse1: analyse1.cc frame_reader.o $(TOOL_DEP)
	$(TOOL_MAKE)
	$(DONE)
vpath analyse1 $(BINDIR)

analyse2: analyse2.cc frame_reader.o $(TOOL_DEP)
	$(TOOL_MAKE)
	$(DONE)
vpath analyse2 $(BINDIR)

analyse3: analyse3.cc frame_reader.o $(TOOL_DEP)
	$(TOOL_MAKE)
	$(DONE)
vpath analyse3 $(BINDIR)


#----------------------------makedep--------------------------------------------

$(DEPDIR)/part7.dep:
	$(MAKEDEP) $(wildcard $(SRCDIR)/tools/*.cc) > $@

