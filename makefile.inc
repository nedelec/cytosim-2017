# // Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#---------------- MACHINE = {mac, linux, cygwin, auto}
MACHINE := auto

#---------------- COMPILER = {gcc, icc, clang}
COMPILER := gcc

#---------------- MODE = {F, P, D, C}
# F = Fast
# P = Profiling
# D = Debug 
# C = Coverage
MODE := F

#---------------- INTEL Math Kernel Library
# If the Intel Math Kernel library is available on your system,
# you can set:
#     HAS_MKL = 1 for dynamic linking
#     HAS_MKL = 2 for static linking
# otherwise, use HAS_MKL = 0;

HAS_MKL := 0

#---------------- PNG image export support
# `libpng` needs to be installed to save PNG images:
#     Mac OSX:
#        www.macports.org :  port install libpng 
#        Homebrew  (http://brew.sh)
#             brew install libpng
#             brew tap homebrew/dupes
#             brew install zlib
#     CentOS:  yum install libpng-devel
#     Ubuntu:  apt-get or synaptic libpng-dev
# HAS_PNG can be:
#     0 : no PNG support
#     1 : macport installation
#     2 : homebrew installation

HAS_PNG := 0

#-------------------------------------------------------------------------------
#---------------------------  Platform Detection  ------------------------------
#-------------------------------------------------------------------------------

ifeq ($(MACHINE), auto)

    ifeq ($(shell uname), Darwin)
       MACHINE := mac
    else ifeq ($(shell uname), Linux)
       MACHINE := linux
    else ifeq ($(shell uname -o), Cygwin)
       MACHINE := cygwin
    else
       $(error Unknown platform)
    endif

endif

#------------------------------------------------------------------------------------
#---------------------------  Compiler Specifications  ------------------------------
#------------------------------------------------------------------------------------

DONE     = @printf "\x1B[35m >>>>>> made %s\x1B[0m\n" $@

ALIGN   := -malign-double -falign-loops -falign-jumps -falign-functions
ALIGN   :=
WARN    := -Wall -Wno-unknown-pragmas # -Wno-unused


ifeq ($(MACHINE),mac)
    LIBTOOL := libtool -static -o
else
    LIBTOOL := ar rcs
endif


ifeq ($(COMPILER),gcc)

    CXX      := g++ -ffinite-math-only -ftrapping-math -Wfatal-errors
    FAST     :=-O3 -fno-tree-vectorize -ffast-math -funroll-loops -march=native -Wno-deprecated
    DEBUG    :=-g3 -ggdb -Wno-deprecated
    COVERAGE :=-fprofile-arcs -ftest-coverage
    
else ifeq ($(COMPILER),icc)

    MAKELIB  := xiar -rcs
     
    #CXX     := icpc -fp-trap=common -vec-report0
    #CXX     := icpc -bind_at_load -fvisibility=hidden -no-parallel -mkl=sequential -wd1224,161
    CXX      := icpc -vec-report0 -wd1224,161
    CXX      := icpc -no-parallel -mkl=sequential -wd1224,161
    # Intel advanced instruction sets:
    # '-xSSE4.1' for core duo (ca. 2009)
    # '-mavx' for Intel core i7 (ca. 2012)
    FAST     := -O3 -xSSE4.1
    FAST     := -O3 -mdynamic-no-pic -funroll-loops -fno-math-errno -mavx
    DEBUG    := -g -Wall -no-pie -wd279,383,810,869,981,1418,1419,1572,2259
    COVERAGE := -ftest-coverage

else ifeq ($(COMPILER),clang)

    CXX      := c++ -ffinite-math-only -ftrapping-math -Wfatal-errors #-mmacosx-version-min=10.10
    FAST     := -O3 -ffast-math -funroll-loops #-floop-optimize2 -msse3
    FAST     := -O3 -ffast-math -funroll-loops
    DEBUG    := -g3 -ggdb
    COVERAGE := -fprofile-arcs -ftest-coverage

endif

#------------------------------------------------------------------------------------
#------------------------------------- Linux ----------------------------------------
#------------------------------------------------------------------------------------

ifeq ($(MACHINE),linux)

    ### Linux options
    FlagsD := -O0 $(DEBUG) $(WARN)
    FlagsP := -O0 -fno-inline -pg $(ALIGN) $(FAST)
    FlagsC := -O0 -fprofile-arcs -ftest-coverage
    FlagsF := -O3 -finline-functions $(ALIGN) $(FAST)

    ## select 64 or 32 bit libraries:
    ifeq ($(shell uname -m),x86_64)
         USRLIB := /usr/lib64
    else
         USRLIB := /usr/lib
    endif

    ### For 64bit Centos 5.8 March 2012:
    # Centos  blas: yum install blas-devel
    # Centos  lapack: yum install lapack-devel
    ### 64bit Linux distribution Ubuntu 12.04 LTS, 2012
    # Ubuntu   blas : apt-get install libblas.dev
    # Ubuntu lapack : apt-get install liblapack.dev

    #### Choose dynamic linking:
    LINK:= -L$(USRLIB) -llapack -lblas -lpthread
    #### ...  or static linking:
    LINK:= -L$(USRLIB) $(USRLIB)/libblas.so $(USRLIB)/liblapack.so -lpthread

    ### Install libraries needed for graphical executables:
    # Centos freglut: yum install freeglut-devel
    # Centos libXi:   yum install libXi-devel
    # Centos libXmu:  yum install libXmu-devel

    # Ubuntu freglut: apt-get install freeglut3-dev
    # Ubuntu libXi:   apt-get install libXi-dev
    # Ubuntu libXmu:  apt-get install libXmu-dev

    ### linkage options for programs using graphics:
    #   check for correct library libname.so otherwise give full path
    LINKGL:= -L$(USRLIB) -lglut -lGL -lGLU -lXt -lX11 -lGLEW

    ifneq ($(HAS_PNG), 0)

        LIB_PNG := $(USRLIB)/libpng.a $(USRLIB)/libz.a
        ING_PNG :=

    endif

endif

#------------------------------------------------------------------------------------
#-------------------------------- Mac OSX -------------------------------------------
#------------------------------------------------------------------------------------

ifeq ($(MACHINE),mac)

    MKL_PATH := /opt/intel/mkl/lib

    FlagsC := $(COVERAGE)
    FlagsD := $(DEBUG) $(WARN)
    FlagsP := $(DEBUG) $(FAST) $(ALIGN)
    FlagsF := $(FAST) $(ALIGN)
    FlagsN := -O2 
  
    LINK   := -framework Accelerate
    LINKGL := -framework GLUT -framework OpenGL -framework AGL
    #LINKGL = -L/opt/local/lib -lglut -lgl
    
    ifeq ($(HAS_PNG), 1)
        
        # macport library:
        LIB_PNG := /opt/local/lib/libpng.a  /opt/local/lib/libz.a
        INC_PNG :=

    endif
    ifeq ($(HAS_PNG), 2)
       
        # homebrew library:
        LIB_PNG := /usr/local/lib/libpng.a /usr/local/opt/zlib/lib/libz.a
        INC_PNG :=

    endif

endif

#------------------------------------------------------------------------------------
#---------------------------------- Cygwin ------------------------------------------
#------------------------------------------------------------------------------------

ifeq ($(MACHINE),cygwin)

    ### Cygwin October 2016
    FlagsD := -O0 $(DEBUG) $(WARN)
    FlagsP := -O0 -fno-inline -pg $(ALIGN) $(FAST)
    FlagsC := -O0 -fprofile-arcs -ftest-coverage
    FlagsF := -O3 -finline-functions $(ALIGN) $(FAST)

    ### Needs
    # blas:   yum install blas-devel
    # lapack: yum install lapack-devel
    LINK = -L/usr/lib -L/bin -llapack -lblas -lpthread -lgfortran
    
    ### linkage options for programs using graphics:
    ### Needs
    ### freglut: yum install freeglut-devel
    ### libXi:   yum install libXi-devel
    ### libXmu:  yum install libXmu-devel
    ###  check for correct library yourlibname.so otherwise give full path 	
    LINKGL := -L/usr/lib -lglut -lGL -lGLU -lXt -lX11

    ifneq ($(HAS_PNG), 0)
    
        LIB_PNG := /lib/libpng.dll.a /lib/libz.a
        LIB_PNG := /usr/lib/libpng.dll.a /usr/lib/libz.a
        ING_PNG :=
    
    endif

    BINEXT :=.exe

endif



#------------------------------------------------------------------------------------
#--------------------- linking with Intel Math Kernel Library -----------------------
#------------------------------------------------------------------------------------

MKL_EXT := _lp64


ifeq ($(HAS_MKL),1)

    # sequential dynamic linking:
    MKL_LIBS := -lmkl_intel$(MKL_EXT) -lmkl_sequential -lmkl_core
    
    # threaded dynamic linking:
    #MKL_LIBS := -lmkl_intel$(MKL_EXT) -lmkl_intel_thread -lmkl_core -liomp5

    # modify the linking command:
    LINK := -L$(MKL_PATH) $(MKL_LIBS) -lpthread    

endif


ifeq ($(HAS_MKL),2)

    # sequential static linking:
    MKL_LIBS := $(MKL_PATH)/libmkl_intel$(MKL_EXT).a $(MKL_PATH)/libmkl_sequential.a $(MKL_PATH)/libmkl_core.a
    
    # threaded static linking:
    #MKL_LIBS := $(MKL_PATH)/libmkl_intel$(MKL_EXT).a $(MKL_PATH)/libmkl_thread.a $(MKL_PATH)/libmkl_core.a /opt/intel/lib/libiomp5.a

    # modify the linking command:
    LINK := $(MKL_LIBS) -lpthread    

endif

