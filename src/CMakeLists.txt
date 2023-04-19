add_executable(interp main.cxx parse/parse.cxx parse/syntax.cxx
                      instructions.cxx interpreter.cxx)
set_property(TARGET interp PROPERTY CXX_STANDARD 20)
target_link_libraries(interp PUBLIC ${Boost_LIBRARIES})
target_include_directories(interp PUBLIC ${CMAKE_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})

add_executable(
  test_instructions test_instructions.cxx parse/parse.cxx parse/syntax.cxx
                    interpreter.cxx instructions.cxx)
target_include_directories(test_instructions PUBLIC ${CMAKE_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})

add_executable(assembler assembler/main.cxx assembler/assembler.cxx)
target_include_directories(assembler PUBLIC ${CMAKE_SOURCE_DIR}/include)
