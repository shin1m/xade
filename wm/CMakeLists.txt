function(add_wayland_protocol path name)
	set(in ${path}/${name}.xml)
	add_custom_command(OUTPUT ${name}-protocol.h
		COMMAND ${WAYLAND_SCANNER} server-header ${in} ${name}-protocol.h
		DEPENDS ${in}
		VERBATIM
	)
endfunction()
add_wayland_protocol(${WAYLAND_PROTOCOLS}/stable/xdg-shell xdg-shell)
add_wayland_protocol(${WLR_PROTOCOLS}/unstable wlr-layer-shell-unstable-v1)
add_executable(xadewm
	xdg-shell-protocol.h
	wlr-layer-shell-unstable-v1-protocol.h
	main.cc
)
target_compile_features(xadewm PRIVATE cxx_std_20)
target_include_directories(xadewm PRIVATE
	${WLROOTS_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}
)
target_compile_definitions(xadewm PRIVATE WLR_USE_UNSTABLE)
target_link_libraries(xadewm
	${WLROOTS_LIBRARIES}
)
