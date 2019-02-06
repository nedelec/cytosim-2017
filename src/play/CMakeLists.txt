set(SOURCES_PLAY
	"${PROJECT_SOURCE_DIR}/src/play/fiber_disp.cc"
	"${PROJECT_SOURCE_DIR}/src/play/line_disp.cc"
	"${PROJECT_SOURCE_DIR}/src/play/point_disp.cc"
	"${PROJECT_SOURCE_DIR}/src/play/display_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/play/display.cc"
	"${PROJECT_SOURCE_DIR}/src/play/display1.cc"
	"${PROJECT_SOURCE_DIR}/src/play/display2.cc"
	"${PROJECT_SOURCE_DIR}/src/play/display3.cc"
	"${PROJECT_SOURCE_DIR}/src/play/sim_thread.cc"
	"${PROJECT_SOURCE_DIR}/src/play/frame_reader.cc"
	"${PROJECT_SOURCE_DIR}/src/play/play_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/play/play.cc"
)

if(BUILD_PLAY_EXECUTABLE)
	add_executable(${PLAY_EXE_TARGET} ${SOURCES_PLAY})

	target_link_libraries(${PLAY_EXE_TARGET} PUBLIC
		"${SIM_LIB_TARGET}"
		"${SPACES_GL_LIB_TARGET}"
		"${GL_LIB_TARGET}"
		"${MATH_LIB_TARGET}"
		"${BASE_LIB_TARGET}"
		"${OPENGL_LIBS}"
		"${LAPACK_LIB}"
		"${BLAS_LIB}"
		Threads::Threads
	)

	target_include_directories(${PLAY_EXE_TARGET} PUBLIC
		"${SFMT_SRC_DIR}"
		"${MATH_SRC_DIR}"
		"${BASE_SRC_DIR}"
		"${SIM_SRC_DIR}"
		"${GL_SRC_DIR}"
		"${PLAY_SRC_DIR}"
	)
endif(BUILD_PLAY_EXECUTABLE)
