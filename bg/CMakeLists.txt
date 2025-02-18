set(name wlr-layer-shell-unstable-v1)
set(in ${WLR_PROTOCOLS}/unstable/${name}.xml)
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
add_executable(xadebg
	wlr-layer-shell-unstable-v1.h
	wlr-layer-shell-unstable-v1.c
	main.cc
)
target_include_directories(xadebg PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(xadebg
	xade
	xadeskia
)
