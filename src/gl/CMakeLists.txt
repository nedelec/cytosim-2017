set(SOURCES_GL
	"${PROJECT_SOURCE_DIR}/src/gl/gle.cc"
	"${PROJECT_SOURCE_DIR}/src/gl/gle_color_int.cc"
	"${PROJECT_SOURCE_DIR}/src/gl/gle_color_float.cc"
	"${PROJECT_SOURCE_DIR}/src/gl/gle_color.cc"
	"${PROJECT_SOURCE_DIR}/src/gl/gle_color_list.cc"
	"${PROJECT_SOURCE_DIR}/src/gl/glapp_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/gl/view.cc"
	"${PROJECT_SOURCE_DIR}/src/gl/view_prop.cc"
	"${PROJECT_SOURCE_DIR}/src/gl/glapp.cc"
	"${PROJECT_SOURCE_DIR}/src/gl/saveimage.cc"
	"${PROJECT_SOURCE_DIR}/src/gl/offscreen.cc"
)

add_library(${GL_LIB_TARGET} STATIC ${SOURCES_GL})

target_include_directories(${GL_LIB_TARGET} PUBLIC
	"${SFMT_SRC_DIR}"
	"${MATH_SRC_DIR}"
	"${BASE_SRC_DIR}"
)
