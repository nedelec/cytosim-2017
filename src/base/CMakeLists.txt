set(BASE_SOURCES
	"${CMAKE_SOURCE_DIR}/src/base/messages.cc"
	"${CMAKE_SOURCE_DIR}/src/base/filewrapper.cc"
	"${CMAKE_SOURCE_DIR}/src/base/filepath.cc"
	"${CMAKE_SOURCE_DIR}/src/base/iowrapper.cc"
	"${CMAKE_SOURCE_DIR}/src/base/exceptions.cc"
	"${CMAKE_SOURCE_DIR}/src/base/tictoc.cc"
	"${CMAKE_SOURCE_DIR}/src/base/node.cc"
	"${CMAKE_SOURCE_DIR}/src/base/node_list.cc"
	"${CMAKE_SOURCE_DIR}/src/base/inventoried.cc"
	"${CMAKE_SOURCE_DIR}/src/base/inventory.cc"
	"${CMAKE_SOURCE_DIR}/src/base/stream_func.cc"
	"${CMAKE_SOURCE_DIR}/src/base/tokenizer.cc"
	"${CMAKE_SOURCE_DIR}/src/base/glossary.cc"
	"${CMAKE_SOURCE_DIR}/src/base/property.cc"
	"${CMAKE_SOURCE_DIR}/src/base/property_list.cc"
	"${CMAKE_SOURCE_DIR}/src/base/vecprint.cc"
	"${CMAKE_SOURCE_DIR}/src/base/backtrace.cc"
)

set(BASE_OBJS
	"${CMAKE_BINARY_DIR}/src/SFMT/CMakeFiles/${SFMT_LIB_TARGET}.dir/SFMT.c.o"
)

add_library(${BASE_LIB_TARGET} STATIC ${BASE_OBJS} ${BASE_SOURCES})
add_dependencies(${BASE_LIB_TARGET} ${SFMT_LIB_TARGET})

set_source_files_properties(
	${BASE_OBJS}
	PROPERTIES
	EXTERNAL_OBJECT true
	GENERATED true
)

target_include_directories(${BASE_LIB_TARGET} PUBLIC
	"${SFMT_SRC_DIR}"
	"${BASE_SRC_DIR}"
	"${MATH_SRC_DIR}"
)
