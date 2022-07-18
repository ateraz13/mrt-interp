##
# interp
#
# @file
# @version 0.1

CXX=g++
CXX_FLAGS=-std=c++20

interp: main.cxx parse.cxx
	${CXX} -o $@ $^ ${CXX_FLAGS}

# end
