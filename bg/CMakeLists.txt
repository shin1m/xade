add_wayland_client(${WLR_PROTOCOLS}/unstable wlr-layer-shell-unstable-v1 private)
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
