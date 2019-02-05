set(SOURCES_GL
	"${CMAKE_SOURCE_DIR}/src/gl/gle.cc"
	"${CMAKE_SOURCE_DIR}/src/gl/gle_color_int.cc"
	"${CMAKE_SOURCE_DIR}/src/gl/gle_color_float.cc"
	"${CMAKE_SOURCE_DIR}/src/gl/gle_color.cc"
	"${CMAKE_SOURCE_DIR}/src/gl/gle_color_list.cc"
	"${CMAKE_SOURCE_DIR}/src/gl/glapp_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/gl/view.cc"
	"${CMAKE_SOURCE_DIR}/src/gl/view_prop.cc"
	"${CMAKE_SOURCE_DIR}/src/gl/glapp.cc"
	"${CMAKE_SOURCE_DIR}/src/gl/saveimage.cc"
	"${CMAKE_SOURCE_DIR}/src/gl/offscreen.cc"
)

add_library(${GL_LIB_TARGET} STATIC ${SOURCES_GL})

target_include_directories(${GL_LIB_TARGET} PUBLIC
	"${SFMT_SRC_DIR}"
	"${MATH_SRC_DIR}"
	"${BASE_SRC_DIR}"
)
