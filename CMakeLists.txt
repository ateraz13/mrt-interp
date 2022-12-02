cmake_minimum_required(VERSION 3.1)

project(interp)

find_package(Boost)

# NOTE: We could link interpreter into an library.

add_executable(interp main.cxx parse.cxx syntax.cxx interpreter.cxx)
set_property(TARGET interp PROPERTY CXX_STANDARD 20)
target_link_libraries(interp PUBLIC ${Boost_LIBRARIES})
target_include_directories(interp PUBLIC ${Boost_INCLUDE_DIRS})

add_executable(test_instructions test_instructions.cxx parse.cxx syntax.cxx interpreter.cxx)
