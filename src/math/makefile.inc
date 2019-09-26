# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


OBJ_MATH:=smath.o vector1.o vector2.o vector3.o matrix1.o matrix2.o matrix3.o \
	 rasterizer.o grid.o matrix.o matsparse.o matsparsesym.o \
	 matsym.o matsparsesym1.o bicgstab.o polygon.o\
	 pointsonsphere.o random.o random_vector.o project_ellipse.o \


#----------------------------rules----------------------------------------------

INC_MATH:=$(addprefix -Isrc/, SFMT math base)


$(OBJ_MATH): %.o: %.cc %.h
	$(COMPILE) -c $(INC_MATH) $< -o build/$@


# this is for graphical debugging of the rasterizer:
rasterizerGL.o: rasterizer.cc rasterizer.h
	$(COMPILE) -c $(INC_MATH) -Isrc/gl -DDISPLAY $< -o $@


libcytomath.a: $(OBJ_MATH)
	$(MAKELIB)
	$(DONE)

