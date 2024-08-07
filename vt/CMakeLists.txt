add_executable(xadevt
	theme.cc
	window.cc
	main.cc
)
target_include_directories(xadevt PRIVATE ${SKIA})
target_link_libraries(xadevt
	xade
	OpenGL::GL
	Fontconfig::Fontconfig
	Freetype::Freetype
	JPEG::JPEG
	PNG::PNG
	WebP::webp WebP::webpdemux
	-L${SKIA}/out/Static -lskia
)
add_custom_command(TARGET xadevt POST_BUILD
	COMMAND ln -sf ${PROJECT_SOURCE_DIR}/symbols/Material_Symbols_Outlined/MaterialSymbolsOutlined-VariableFont_FILL,GRAD,opsz,wght.ttf xadevt.symbols
	VERBATIM
)
