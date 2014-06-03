set(SRCS
circular_buffer.c
)
if(WIN32)
	set(SRCS ${SRCS} ${PROJECT_BINARY_DIR}/version.rc)
endif(WIN32)
add_library(cb SHARED ${SRCS})
add_library(cb_static STATIC ${SRCS})
if(WIN32)
	target_link_libraries(cb)
	target_link_libraries(cb_static)
else(WIN32)
	target_link_libraries(cb pthread)
	target_link_libraries(cb_static pthread)
endif(WIN32)
install(TARGETS cb DESTINATION lib)

install(FILES
circular_buffer.h
DESTINATION include
COMPONENT headers)
