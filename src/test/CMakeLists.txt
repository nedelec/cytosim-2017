set(TEST_INCLUDES
	"${SFMT_SRC_DIR}"
	"${BASE_SRC_DIR}"
	"${MATH_SRC_DIR}"
	"${SIM_SRC_DIR}"
	"${GL_SRC_DIR}"
	"${PLAY_SRC_DIR}"
)

set(TEST_LIBS
	"${LAPACK_LIB}"
	"${BLAS_LIB}"
	Threads::Threads
	"${MATH_LIB_TARGET}"
	"${BASE_LIB_TARGET}"
	"${OPENGL_LIBS}"
)

set(TEST_OBJS
	"${PROJECT_BINARY_DIR}/src/math/CMakeFiles/${MATH_LIB_TARGET}.dir/random.cc.o"
	"${PROJECT_BINARY_DIR}/src/SFMT/CMakeFiles/${SFMT_LIB_TARGET}.dir/SFMT.c.o"
	"${PROJECT_BINARY_DIR}/src/base/CMakeFiles/${BASE_LIB_TARGET}.dir/exceptions.cc.o"
	"${PROJECT_BINARY_DIR}/src/base/CMakeFiles/${BASE_LIB_TARGET}.dir/filewrapper.cc.o"
	"${PROJECT_BINARY_DIR}/src/base/CMakeFiles/${BASE_LIB_TARGET}.dir/messages.cc.o"
	"${PROJECT_BINARY_DIR}/src/math/CMakeFiles/${MATH_LIB_TARGET}.dir/smath.cc.o"
	"${PROJECT_BINARY_DIR}/src/base/CMakeFiles/${BASE_LIB_TARGET}.dir/vecprint.cc.o"
	"${PROJECT_BINARY_DIR}/src/base/CMakeFiles/${BASE_LIB_TARGET}.dir/backtrace.cc.o"
	"${PROJECT_BINARY_DIR}/src/base/CMakeFiles/${BASE_LIB_TARGET}.dir/tictoc.cc.o"
	"${PROJECT_BINARY_DIR}/src/base/CMakeFiles/${BASE_LIB_TARGET}.dir/filepath.cc.o"
)

set(TEST_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src/test/")

set(TEST_LIST
	#"test" # Test is a reserved word in CMake 3.10.x
	"test_blas"
	"test_param"
	"test_random"
	"test_quaternion"
	"test_simd"
	"test_sizeof"
	"test_cxx"
	"test_math"
	"test_thread"
	"test_string"
)

foreach(TEST_NAME ${TEST_LIST})
	add_executable("${TEST_NAME}" ${TEST_OBJS} "${PROJECT_SOURCE_DIR}/src/test/${TEST_NAME}.cc")
	set_source_files_properties(
		${TEST_OBJS}
		PROPERTIES
		EXTERNAL_OBJECT true
		GENERATED true
	)
	target_include_directories(${TEST_NAME} PUBLIC "${TEST_INCLUDES}")
	target_link_libraries(${TEST_NAME} PUBLIC "${TEST_LIBS}")
endforeach()

set(TEST_GL_LIBS
	rasterizerGL
	"${GL_LIB_TARGET}"
	"${SPACES_LIB_TARGET}"
	"${SIM_LIB_TARGET}"
	"${MATH_LIB_TARGET}"
	"${BASE_LIB_TARGET}"
	"${LAPACK_LIB}"
	"${BLAS_LIB}"
	Threads::Threads
	"${OPENGL_LIBS}"
)

set(GL_TEST_LIST
	#"test_opengl" #Currently experiencing an issue with glext.h on Linux
	"test_glut"
	"test_glapp"
	"test_sphere"
	"test_rasterizer"
	"test_space"
	"test_grid"
	"test_solve"
)

foreach(TEST_NAME ${GL_TEST_LIST})
	add_executable(${TEST_NAME} "${PROJECT_SOURCE_DIR}/src/test/${TEST_NAME}.cc")
	target_include_directories(${TEST_NAME} PUBLIC "${TEST_INCLUDES}")
	target_link_libraries(${TEST_NAME} PUBLIC "${TEST_GL_LIBS}")
endforeach()
