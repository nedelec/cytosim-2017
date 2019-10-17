set(SOURCES_SIM
	"${PROJECT_SOURCE_DIR}/src/sim/movable.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/mecable.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/point_set.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/point_exact.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/point_interpolated.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/fiber_locus.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/object.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/object_set.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/common.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/fiber_binder.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/sphere_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/sphere.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/sphere_set.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/bead_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/bead.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/bead_set.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/solid_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/solid.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/solid_set.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/meca.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/simul_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/fiber_grid.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/point_grid.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/field.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/field_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/field_set.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/space_set.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/simul.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/interface.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/parser.cc"
)

set(SOURCES_HANDS
	"${PROJECT_SOURCE_DIR}/src/sim/hand.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hand_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hands/motor.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hands/motor_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hands/slider.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hands/slider_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hands/actor.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hands/actor_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hands/nucleator.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hands/nucleator_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hands/tracker.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hands/tracker_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hands/cutter.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hands/cutter_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hands/rescuer.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hands/rescuer_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hands/mighty.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/hands/mighty_prop.cc"
)

set(SOURCES_FIBERS
	"${PROJECT_SOURCE_DIR}/src/sim/fiber_naked.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/fiber.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/rigid_fiber.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/fiber_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/fiber_set.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/fibers/dynamic_fiber.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/fibers/dynamic_fiber_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/fibers/classic_fiber.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/fibers/classic_fiber_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/fibers/treadmilling_fiber.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/fibers/treadmilling_fiber_prop.cc"
)

set(SOURCES_SINGLES
	"${PROJECT_SOURCE_DIR}/src/sim/single.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/single_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/single_set.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/singles/picket.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/singles/picket_long.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/singles/wrist.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/singles/wrist_long.cc"
)

set(SOURCES_COUPLES
	"${PROJECT_SOURCE_DIR}/src/sim/couple.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/couple_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/couple_set.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/couples/couple_long.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/couples/crosslink.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/couples/crosslink_long.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/couples/crosslink_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/couples/bridge.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/couples/bridge_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/couples/shackle.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/couples/shackle_long.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/couples/shackle_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/couples/fork.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/couples/fork_prop.cc"
)

set(SOURCES_ORGANIZERS
	"${PROJECT_SOURCE_DIR}/src/sim/organizer.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/organizer_set.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/organizers/aster.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/organizers/aster_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/organizers/nucleus.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/organizers/nucleus_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/organizers/fake.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/organizers/fake_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/organizers/bundle.cc"
	"${PROJECT_SOURCE_DIR}/src/sim/organizers/bundle_prop.cc"
)

set(SOURCES_CYTOSIM
	"${SOURCES_SIM}"
	"${SOURCES_HANDS}"
	"${SOURCES_FIBERS}"
	"${SOURCES_SINGLES}"
	"${SOURCES_COUPLES}"
	"${SOURCES_ORGANIZERS}"
)

add_library(${SIM_LIB_TARGET} STATIC ${SOURCES_CYTOSIM})

target_include_directories(${SIM_LIB_TARGET} PUBLIC
	"${SFMT_SRC_DIR}"
	"${MATH_SRC_DIR}"
	"${BASE_SRC_DIR}"
	"${SIM_SRC_DIR}"
	"${GL_SRC_DIR}"
)

if(BUILD_SIM_EXECUTABLE)
	add_executable(${CYTOSIM_EXE_TARGET} "${PROJECT_SOURCE_DIR}/src/sim/sim.cc")

	target_link_libraries(${CYTOSIM_EXE_TARGET} PRIVATE
		"${SIM_LIB_TARGET}"
		"${SPACES_LIB_TARGET}"
		"${MATH_LIB_TARGET}"
		"${BASE_LIB_TARGET}"
		"${LAPACK_LIB}"
		"${BLAS_LIB}"
		Threads::Threads
	)

	target_include_directories(${CYTOSIM_EXE_TARGET} PUBLIC
		"${MATH_SRC_DIR}"
		"${BASE_SRC_DIR}"
		"${SIM_SRC_DIR}"
	)
endif(BUILD_SIM_EXECUTABLE)
