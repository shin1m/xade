cmake_minimum_required(VERSION 3.30)
project(xade VERSION 202509)
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
	wlroots-0.19
)
pkg_get_variable(WAYLAND_PROTOCOLS wayland-protocols pkgdatadir)
pkg_get_variable(WAYLAND_SCANNER wayland-scanner wayland_scanner)
pkg_get_variable(WLR_PROTOCOLS wlr-protocols pkgdatadir)
find_package(suisha 202508 REQUIRED)
# skia 6903a4e65c3ffeb3e7aaa3a94bdc46f1bb1a75b7
# is_official_build=true is_component_build=true skia_use_egl=true
set(SKIA ${PROJECT_SOURCE_DIR}/skia)
add_library(skia SHARED IMPORTED)
target_include_directories(skia INTERFACE ${SKIA})
set_property(TARGET skia PROPERTY IMPORTED_LOCATION ${SKIA}/out/Shared/libskia.so)
find_package(xemmai 202508 REQUIRED)
function(add_wayland_client path name visibility)
	set(in ${path}/${name}.xml)
	add_custom_command(OUTPUT ${name}.h
		COMMAND ${WAYLAND_SCANNER} client-header ${in} ${name}.h
		DEPENDS ${in}
		VERBATIM
	)
	add_custom_command(OUTPUT ${name}.c
		COMMAND ${WAYLAND_SCANNER} ${visibility}-code ${in} ${name}.c
		DEPENDS ${in}
		VERBATIM
	)
endfunction()
add_subdirectory(xade)
add_subdirectory(vt)
add_subdirectory(wm)
add_subdirectory(bg)
add_subdirectory(im)
add_subdirectory(xemmai)
install(IMPORTED_RUNTIME_ARTIFACTS skia DESTINATION lib)
install(DIRECTORY include/xade DESTINATION include)
install(TARGETS xade xadeskia DESTINATION lib)
set_property(TARGET xadevt xadewm xadebg xadeim PROPERTY INSTALL_RPATH "\$ORIGIN/../lib")
install(TARGETS xadevt xadewm xadebg xadeim DESTINATION bin)
set_property(TARGET xade-xemmai PROPERTY INSTALL_RPATH "\$ORIGIN/..")
install(TARGETS xade-xemmai DESTINATION lib/xemmai-${xemmai_VERSION_MAJOR})
