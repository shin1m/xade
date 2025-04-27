add_library(xade-xemmai MODULE
	client.cc
	layered.cc
	xade.cc
)
target_link_libraries(xade-xemmai PRIVATE xade xemmai)
set_target_properties(xade-xemmai PROPERTIES OUTPUT_NAME xade PREFIX "")
