set(SRCS
circular_buffer.c
)
if(WIN32)
	set(SRCS ${SRCS} ${PROJECT_BINARY_DIR}/version.rc)
endif(WIN32)
add_library(cb SHARED ${SRCS})
add_library(cb_static STATIC ${SRCS})
target_link_libraries(cb pthread)
target_link_libraries(cb_static pthread)
install(TARGETS cb DESTINATION lib)
