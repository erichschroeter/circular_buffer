set(SRCS
circular_buffer.c
)
if(WIN32)
	set(SRCS ${SRCS} ${PROJECT_BINARY_DIR}/version.rc)
endif(WIN32)
add_library(cb SHARED ${SRCS})
add_library(cb_static STATIC ${SRCS})
install(TARGETS cb DESTINATION lib)
