link_libraries(xade xemmai)
function(add_module name)
	add_library(${name}-xemmai MODULE ${ARGN})
	set_target_properties(${name}-xemmai PROPERTIES OUTPUT_NAME ${name} PREFIX "")
endfunction()
add_module(xade
	client.cc
	layered.cc
	xade.cc
)
#add_module(xadeskia xadeskia.cc)
#target_link_libraries(xadeskia-xemmai xadeskia)
