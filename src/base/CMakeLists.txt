set(BASE_SOURCES
	"${PROJECT_SOURCE_DIR}/src/base/messages.cc"
	"${PROJECT_SOURCE_DIR}/src/base/filewrapper.cc"
	"${PROJECT_SOURCE_DIR}/src/base/filepath.cc"
	"${PROJECT_SOURCE_DIR}/src/base/iowrapper.cc"
	"${PROJECT_SOURCE_DIR}/src/base/exceptions.cc"
	"${PROJECT_SOURCE_DIR}/src/base/tictoc.cc"
	"${PROJECT_SOURCE_DIR}/src/base/node.cc"
	"${PROJECT_SOURCE_DIR}/src/base/node_list.cc"
	"${PROJECT_SOURCE_DIR}/src/base/inventoried.cc"
	"${PROJECT_SOURCE_DIR}/src/base/inventory.cc"
	"${PROJECT_SOURCE_DIR}/src/base/stream_func.cc"
	"${PROJECT_SOURCE_DIR}/src/base/tokenizer.cc"
	"${PROJECT_SOURCE_DIR}/src/base/glossary.cc"
	"${PROJECT_SOURCE_DIR}/src/base/property.cc"
	"${PROJECT_SOURCE_DIR}/src/base/property_list.cc"
	"${PROJECT_SOURCE_DIR}/src/base/vecprint.cc"
	"${PROJECT_SOURCE_DIR}/src/base/backtrace.cc"
)

set(BASE_OBJS
	"${PROJECT_BINARY_DIR}/src/SFMT/CMakeFiles/${SFMT_LIB_TARGET}.dir/SFMT.c.o"
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
