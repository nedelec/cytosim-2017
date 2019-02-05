set(MATH_SOURCES
	"${CMAKE_SOURCE_DIR}/src/math/smath.cc"
	"${CMAKE_SOURCE_DIR}/src/math/vector1.cc"
	"${CMAKE_SOURCE_DIR}/src/math/vector2.cc"
	"${CMAKE_SOURCE_DIR}/src/math/vector3.cc"
	"${CMAKE_SOURCE_DIR}/src/math/matrix1.cc"
	"${CMAKE_SOURCE_DIR}/src/math/matrix2.cc"
	"${CMAKE_SOURCE_DIR}/src/math/matrix3.cc"
	"${CMAKE_SOURCE_DIR}/src/math/rasterizer.cc"
	"${CMAKE_SOURCE_DIR}/src/math/grid.cc"
	"${CMAKE_SOURCE_DIR}/src/math/matrix.cc"
	"${CMAKE_SOURCE_DIR}/src/math/matsparse.cc"
	"${CMAKE_SOURCE_DIR}/src/math/matsparsesym.cc"
	"${CMAKE_SOURCE_DIR}/src/math/matsym.cc"
	"${CMAKE_SOURCE_DIR}/src/math/matsparsesym1.cc"
	"${CMAKE_SOURCE_DIR}/src/math/bicgstab.cc"
	"${CMAKE_SOURCE_DIR}/src/math/polygon.cc"
	"${CMAKE_SOURCE_DIR}/src/math/pointsonsphere.cc"
	"${CMAKE_SOURCE_DIR}/src/math/random.cc"
	"${CMAKE_SOURCE_DIR}/src/math/random_vector.cc"
	"${CMAKE_SOURCE_DIR}/src/math/project_ellipse.cc"
)

add_library(${MATH_LIB_TARGET} STATIC ${MATH_SOURCES})
target_include_directories(${MATH_LIB_TARGET} PUBLIC
	"${SFMT_SRC_DIR}"
	"${MATH_SRC_DIR}"
	"${BASE_SRC_DIR}"
)

add_library(rasterizerGL "${CMAKE_SOURCE_DIR}/src/math/rasterizer.cc")
target_compile_definitions(rasterizerGL PRIVATE -DDISPLAY)
target_include_directories(rasterizerGL PUBLIC
	"${SFMT_SRC_DIR}"
	"${MATH_SRC_DIR}"
	"${BASE_SRC_DIR}"
	"${GL_SRC_DIR}"
)
