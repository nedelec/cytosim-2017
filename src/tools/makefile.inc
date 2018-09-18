# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.



TOOLS:=frametool sieve reader report reportF analyse1 analyse2 analyse3


.PHONY: tools
tools: $(TOOLS)


vpath %.cc src/tools


#--------------------macros----------------------------------------------------

TOOL_DEP := libcytosim.a libcytospace.a libcytomath.a libcytobase.a

TOOL_OBJ  = $(filter %.cc, $^) $(addprefix build/, $(notdir $(filter-out %.cc, $^)))

TOOL_INC  = $(addprefix -Isrc/, SFMT math base sim sim/spaces sim/fibers sim/singles sim/organizers gl play)

TOOL_MAKE = $(COMPILE) $(TOOL_INC) $(TOOL_OBJ) $(LINK) -o bin/$@


#-------------------targets----------------------------------------------------
 
 
frametool: frametool.cc
	$(TOOL_MAKE)
	$(DONE)
vpath frametool bin


sieve: sieve.cc frame_reader.o $(TOOL_DEP) 
	$(TOOL_MAKE)
	$(DONE)
vpath sieve bin


frame_readerVB.o: frame_reader.cc frame_reader.h
	$(COMPILE) -c $(PLAY_INC) -DVERBOSE_READER $< -o build/$@
vpath frame_readerVB.o build


reader: reader.cc frame_readerVB.o $(TOOL_DEP)
	$(TOOL_MAKE)
	$(DONE)
vpath reader bin



report: report.cc frame_reader.o $(TOOL_DEP) | bin
	$(TOOL_MAKE)
	$(DONE)
vpath report bin

reportF: reportF.cc frame_reader.o $(TOOL_DEP)
	$(TOOL_MAKE)
	$(DONE)
vpath reportF bin

analyse1: analyse1.cc frame_reader.o $(TOOL_DEP)
	$(TOOL_MAKE)
	$(DONE)
vpath analyse1 bin

analyse2: analyse2.cc frame_reader.o $(TOOL_DEP)
	$(TOOL_MAKE)
	$(DONE)
vpath analyse2 bin

analyse3: analyse3.cc frame_reader.o $(TOOL_DEP)
	$(TOOL_MAKE)
	$(DONE)
vpath analyse3 bin


