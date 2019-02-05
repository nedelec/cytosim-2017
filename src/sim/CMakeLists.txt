set(SOURCES_SIM
	"${CMAKE_SOURCE_DIR}/src/sim/movable.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/mecable.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/point_set.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/point_exact.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/point_interpolated.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/fiber_locus.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/object.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/object_set.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/common.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/fiber_binder.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/sphere_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/sphere.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/sphere_set.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/bead_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/bead.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/bead_set.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/solid_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/solid.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/solid_set.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/meca.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/simul_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/fiber_grid.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/point_grid.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/field.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/field_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/field_set.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/space_set.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/simul.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/interface.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/parser.cc"
)

set(SOURCES_HANDS
	"${CMAKE_SOURCE_DIR}/src/sim/hand.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hand_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hands/motor.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hands/motor_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hands/slider.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hands/slider_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hands/actor.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hands/actor_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hands/nucleator.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hands/nucleator_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hands/tracker.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hands/tracker_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hands/cutter.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hands/cutter_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hands/rescuer.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hands/rescuer_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hands/mighty.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/hands/mighty_prop.cc"
)

set(SOURCES_FIBERS
	"${CMAKE_SOURCE_DIR}/src/sim/fiber_naked.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/fiber.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/rigid_fiber.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/fiber_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/fiber_set.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/fibers/dynamic_fiber.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/fibers/dynamic_fiber_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/fibers/classic_fiber.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/fibers/classic_fiber_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/fibers/treadmilling_fiber.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/fibers/treadmilling_fiber_prop.cc"
)

set(SOURCES_SINGLES
	"${CMAKE_SOURCE_DIR}/src/sim/single.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/single_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/single_set.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/singles/picket.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/singles/picket_long.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/singles/wrist.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/singles/wrist_long.cc"
)

set(SOURCES_COUPLES
	"${CMAKE_SOURCE_DIR}/src/sim/couple.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/couple_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/couple_set.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/couples/couple_long.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/couples/crosslink.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/couples/crosslink_long.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/couples/crosslink_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/couples/bridge.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/couples/bridge_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/couples/shackle.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/couples/shackle_long.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/couples/shackle_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/couples/fork.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/couples/fork_prop.cc"
)

set(SOURCES_ORGANIZERS
	"${CMAKE_SOURCE_DIR}/src/sim/organizer.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/organizer_set.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/organizers/aster.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/organizers/aster_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/organizers/nucleus.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/organizers/nucleus_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/organizers/fake.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/organizers/fake_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/organizers/bundle.cc"
	"${CMAKE_SOURCE_DIR}/src/sim/organizers/bundle_prop.cc"
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
	add_executable(${CYTOSIM_EXE_TARGET} "${CMAKE_SOURCE_DIR}/src/sim/sim.cc")

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
