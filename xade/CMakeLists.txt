add_wayland_client(${WAYLAND_PROTOCOLS}/stable/xdg-shell xdg-shell public)
add_wayland_client(${WAYLAND_PROTOCOLS}/unstable/text-input text-input-unstable-v3 private)
add_wayland_client(${WLR_PROTOCOLS}/unstable wlr-layer-shell-unstable-v1 public)
add_library(xade SHARED
	xdg-shell.h
	xdg-shell.c
	text-input-unstable-v3.h
	text-input-unstable-v3.c
	client.cc
	wlr-layer-shell-unstable-v1.h
	wlr-layer-shell-unstable-v1.c
	layered.cc
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
	${CMAKE_CURRENT_BINARY_DIR}/wlr-layer-shell-unstable-v1.h
	DESTINATION include/xade
)
