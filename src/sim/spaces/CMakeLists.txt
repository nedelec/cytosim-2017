set(SPACES_SOURCES
	"${PROJECT_SOURCE_DIR}/src/sim/modulo.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/space.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/space_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/spaces/space_square.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/spaces/space_sphere.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/spaces/space_dice.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/spaces/space_torus.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/spaces/space_polygon.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/spaces/space_banana.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/spaces/space_ellipse.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/spaces/space_cylinder.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/spaces/space_cylinderZ.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/spaces/space_capsule.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/spaces/space_strip.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/spaces/space_periodic.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/spaces/space_cylinderP.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/spaces/space_dynamic_ellipse.cc"
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
