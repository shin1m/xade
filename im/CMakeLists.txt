set(name input-method-unstable-v2)
set(in ${CMAKE_CURRENT_SOURCE_DIR}/${name}.xml)
add_custom_command(OUTPUT ${name}.h
	COMMAND ${WAYLAND_SCANNER} client-header ${in} ${name}.h
	DEPENDS ${in}
	VERBATIM
)
add_custom_command(OUTPUT ${name}.c
	COMMAND ${WAYLAND_SCANNER} private-code ${in} ${name}.c
	DEPENDS ${in}
	VERBATIM
)
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
