add_executable(xadevt
	theme.cc
	window.cc
	main.cc
)
target_link_libraries(xadevt
	xade
	xadeskia
)
add_custom_command(TARGET xadevt POST_BUILD
	COMMAND ln -sf ${PROJECT_SOURCE_DIR}/symbols/Material_Symbols_Outlined/MaterialSymbolsOutlined-VariableFont_FILL,GRAD,opsz,wght.ttf xadevt.symbols
	VERBATIM
)
