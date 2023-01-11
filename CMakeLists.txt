cmake_minimum_required(VERSION 3.1)

project(interp)

find_package(Boost)

set(CMAKE_CXX_STANDARD 20)

# NOTE: We could link interpreter into an library.

set(CMAKE_SOURCE_DIR ./src)

add_subdirectory(./src)
