# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


OBJ_BASE:=messages.o filewrapper.o filepath.o iowrapper.o exceptions.o\
     tictoc.o node.o node_list.o inventoried.o inventory.o stream_func.o\
     tokenizer.o glossary.o property.o property_list.o vecprint.o backtrace.o

#----------------------------rules----------------------------------------------

INC_BASE:=$(addprefix -Isrc/, SFMT base math)

$(OBJ_BASE): %.o: %.cc %.h
	$(COMPILE) -c $(INC_BASE) $< -o build/$@

#----------------------------targets--------------------------------------------

libcytobase.a: $(OBJ_BASE) SFMT.o
	$(MAKELIB)
	$(DONE)


