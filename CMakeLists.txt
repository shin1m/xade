cmake_minimum_required(VERSION 3.30)
project(xade)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND REQUIRED
	wayland-client
	wayland-cursor
	wayland-egl
	xkbcommon
)
pkg_check_modules(WLROOTS REQUIRED
	wayland-server
	xkbcommon
	wlroots-0.18
)
pkg_get_variable(WAYLAND_PROTOCOLS wayland-protocols pkgdatadir)
pkg_get_variable(WAYLAND_SCANNER wayland-scanner wayland_scanner)
find_package(suisha 202408 REQUIRED)
set(SKIA ${PROJECT_SOURCE_DIR}/skia)
add_library(skia SHARED IMPORTED)
target_include_directories(skia INTERFACE ${SKIA})
set_property(TARGET skia PROPERTY IMPORTED_LOCATION ${SKIA}/out/Shared/libskia.so)
add_subdirectory(xade)
add_subdirectory(vt)
add_subdirectory(wm)
add_subdirectory(im)
install(IMPORTED_RUNTIME_ARTIFACTS skia DESTINATION lib)
install(DIRECTORY include/xade DESTINATION include)
install(TARGETS xadevt xadewm xadeim DESTINATION bin)
install(TARGETS xade xadeskia DESTINATION lib)
