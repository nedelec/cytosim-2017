set(TOOL_INCLUDES
	"${SFMT_SRC_DIR}"
	"${MATH_SRC_DIR}"
	"${BASE_SRC_DIR}"
	"${SIM_SRC_DIR}"
	"${GL_SRC_DIR}"
	"${PLAY_SRC_DIR}"
)

set(TOOL_LIBS
	"${SIM_LIB_TARGET}"
	"${SPACES_LIB_TARGET}"
	"${MATH_LIB_TARGET}"
	"${BASE_LIB_TARGET}"
	"${LAPACK_LIB}"
	"${BLAS_LIB}"
	Threads::Threads
)

set(TOOL_OBJS
	"${PROJECT_BINARY_DIR}/src/play/CMakeFiles/play.dir/frame_reader.cc.o"
)

set(TOOL_LIST
	"frametool"
	"sieve"
	"report"
	"reportF"
	"analyse1"
	"analyse2"
	"analyse3"
)

# Build the Tools
foreach(TOOL_NAME ${TOOL_LIST})
	add_executable("${TOOL_NAME}" ${TOOL_OBJS} "${PROJECT_SOURCE_DIR}/src/tools/${TOOL_NAME}.cc")
	set_source_files_properties(
		${TOOL_OBJS}
		PROPERTIES
		EXTERNAL_OBJECT true
		GENERATED true
	)
	target_link_libraries(${TOOL_NAME} PUBLIC "${TOOL_LIBS}")
	target_include_directories(${TOOL_NAME} PUBLIC ${TOOL_INCLUDES})
endforeach()

set(READER_OBJS
	"${PROJECT_BINARY_DIR}/src/play/CMakeFiles/play.dir/frame_reader.cc.o"
)

set(SOURCES_frame_readerVB
	"${PROJECT_SOURCE_DIR}/src/play/frame_reader.h"
	"${PROJECT_SOURCE_DIR}/src/play/frame_reader.cc"
)
add_library(frame_readerVB ${SOURCES_frame_readerVB})
target_include_directories(frame_readerVB PUBLIC "${TOOL_INCLUDES}")
target_compile_definitions(frame_readerVB PUBLIC -DVERBOSE_READER)

add_executable("reader" "${PROJECT_SOURCE_DIR}/src/tools/reader.cc")
target_link_libraries(reader PUBLIC
	"${TOOL_LIBS}"
	"frame_readerVB"
)
target_include_directories(reader PUBLIC ${TOOL_INCLUDES})
