set(MATH_SOURCES
	"${PROJECT_SOURCE_DIR}/src/math/smath.cc"
	"${PROJECT_SOURCE_DIR}/src/math/vector1.cc"
	"${PROJECT_SOURCE_DIR}/src/math/vector2.cc"
	"${PROJECT_SOURCE_DIR}/src/math/vector3.cc"
	"${PROJECT_SOURCE_DIR}/src/math/matrix1.cc"
	"${PROJECT_SOURCE_DIR}/src/math/matrix2.cc"
	"${PROJECT_SOURCE_DIR}/src/math/matrix3.cc"
	"${PROJECT_SOURCE_DIR}/src/math/rasterizer.cc"
	"${PROJECT_SOURCE_DIR}/src/math/grid.cc"
	"${PROJECT_SOURCE_DIR}/src/math/matrix.cc"
	"${PROJECT_SOURCE_DIR}/src/math/matsparse.cc"
	"${PROJECT_SOURCE_DIR}/src/math/matsparsesym.cc"
	"${PROJECT_SOURCE_DIR}/src/math/matsym.cc"
	"${PROJECT_SOURCE_DIR}/src/math/matsparsesym1.cc"
	"${PROJECT_SOURCE_DIR}/src/math/bicgstab.cc"
	"${PROJECT_SOURCE_DIR}/src/math/polygon.cc"
	"${PROJECT_SOURCE_DIR}/src/math/pointsonsphere.cc"
	"${PROJECT_SOURCE_DIR}/src/math/random.cc"
	"${PROJECT_SOURCE_DIR}/src/math/random_vector.cc"
	"${PROJECT_SOURCE_DIR}/src/math/project_ellipse.cc"
)

add_library(${MATH_LIB_TARGET} STATIC ${MATH_SOURCES})
target_include_directories(${MATH_LIB_TARGET} PUBLIC
	"${SFMT_SRC_DIR}"
	"${MATH_SRC_DIR}"
	"${BASE_SRC_DIR}"
)

add_library(rasterizerGL "${PROJECT_SOURCE_DIR}/src/math/rasterizer.cc")
target_compile_definitions(rasterizerGL PRIVATE -DDISPLAY)
target_include_directories(rasterizerGL PUBLIC
	"${SFMT_SRC_DIR}"
	"${MATH_SRC_DIR}"
	"${BASE_SRC_DIR}"
	"${GL_SRC_DIR}"
)
