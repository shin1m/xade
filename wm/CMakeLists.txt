function(add_protocol protocol)
	cmake_path(GET protocol STEM LAST_ONLY name)
	set(in ${WAYLAND_PROTOCOLS}/${protocol})
	add_custom_command(OUTPUT ${name}-protocol.h
		COMMAND ${WAYLAND_SCANNER} server-header ${in} ${name}-protocol.h
		DEPENDS ${in}
		VERBATIM
	)
endfunction()
add_protocol(stable/xdg-shell/xdg-shell.xml)
add_executable(xadewm
	xdg-shell-protocol.h
	main.cc
)
target_compile_features(xadewm PRIVATE cxx_std_20)
target_include_directories(xadewm PRIVATE
	${WLROOTS_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
)
target_compile_definitions(xadewm PRIVATE WLR_USE_UNSTABLE)
target_link_libraries(xadewm
	${WLROOTS_LIBRARIES}
)
