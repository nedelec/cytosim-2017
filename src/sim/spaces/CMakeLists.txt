set(SPACES_SOURCES
	"${CMAKE_SOURCE_DIR}/src/sim/modulo.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/space.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/space_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/spaces/space_square.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/spaces/space_sphere.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/spaces/space_dice.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/spaces/space_torus.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/spaces/space_polygon.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/spaces/space_banana.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/spaces/space_ellipse.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/spaces/space_cylinder.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/spaces/space_cylinderZ.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/spaces/space_capsule.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/spaces/space_strip.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/spaces/space_periodic.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/spaces/space_cylinderP.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/spaces/space_dynamic_ellipse.cc"
)

add_library(${SPACES_LIB_TARGET} STATIC ${SPACES_SOURCES})
target_include_directories(${SPACES_LIB_TARGET} PUBLIC
	"${SFMT_SRC_DIR}"
	"${MATH_SRC_DIR}"
	"${BASE_SRC_DIR}"
	"${SIM_SRC_DIR}"
	"${GL_SRC_DIR}"
)

add_library(${SPACES_GL_LIB_TARGET} STATIC ${SPACES_SOURCES})
target_compile_definitions(${SPACES_GL_LIB_TARGET} PRIVATE -DDISPLAY)
target_include_directories(${SPACES_GL_LIB_TARGET} PUBLIC
	"${SFMT_SRC_DIR}"
	"${MATH_SRC_DIR}"
	"${BASE_SRC_DIR}"
	"${SIM_SRC_DIR}"
	"${GL_SRC_DIR}"
)
