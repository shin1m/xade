function(add_protocol protocol)
	cmake_path(GET protocol STEM LAST_ONLY name)
	set(in ${WAYLAND_PROTOCOLS}/${protocol})
	add_custom_command(OUTPUT ${name}-client.h
		COMMAND ${WAYLAND_SCANNER} client-header ${in} ${name}-client.h
		DEPENDS ${in}
		VERBATIM
	)
	add_custom_command(OUTPUT ${name}.c
		COMMAND ${WAYLAND_SCANNER} public-code ${in} ${name}.c
		DEPENDS ${in}
		VERBATIM
	)
endfunction()
add_protocol(stable/xdg-shell/xdg-shell.xml)
add_protocol(unstable/text-input/text-input-unstable-v3.xml)
add_library(xade SHARED
	xdg-shell-client.h
	xdg-shell.c
	text-input-unstable-v3-client.h
	text-input-unstable-v3.c
	client.cc
)
target_compile_features(xade PUBLIC cxx_std_20)
target_include_directories(xade PUBLIC
	${WAYLAND_INCLUDE_DIRS}
	"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
	"$<INSTALL_INTERFACE:include>"
)
target_link_libraries(xade
	OpenGL::EGL
	${WAYLAND_LIBRARIES}
	suisha
)
add_library(xadeskia SHARED
	skia.cc
)
target_compile_features(xadeskia PUBLIC cxx_std_20)
target_include_directories(xadeskia PUBLIC
	"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
	"$<INSTALL_INTERFACE:include>"
)
target_link_libraries(xadeskia
	OpenGL::GL
	skia
)
install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client.h
	${CMAKE_CURRENT_BINARY_DIR}/text-input-unstable-v3-client.h
	DESTINATION include/xade
)
