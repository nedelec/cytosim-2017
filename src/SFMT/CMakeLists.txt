set(SFMT_SOURCES
	"${PROJECT_SOURCE_DIR}/src/SFMT/SFMT.c"
)

add_library(${SFMT_LIB_TARGET} STATIC ${SFMT_SOURCES})

target_compile_definitions(${SFMT_LIB_TARGET} PUBLIC SFMT_MEXP=19937)

target_include_directories(${SFMT_LIB_TARGET} PUBLIC
	"${SFMT_SRC_DIR}"
)
set_target_properties(${SFMT_LIB_TARGET}
	PROPERTIES
	LINKER_LANGUAGE C
)
