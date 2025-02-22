add_wayland_client(${CMAKE_CURRENT_SOURCE_DIR} input-method-unstable-v2 private)
add_executable(xadeim
	engine.cc
	basic_dictionary.cc
	input-method-unstable-v2.h
	input-method-unstable-v2.c
	main.cc
)
target_include_directories(xadeim PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(xadeim
	xade
	xadeskia
)
