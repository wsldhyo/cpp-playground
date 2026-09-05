add_executable(client main.cc) 
set_target_properties(client PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_compile_options(client PRIVATE -pthread -g)
target_link_libraries(client PRIVATE pthread)