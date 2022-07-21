##
# interp
#
# @file
# @version 0.1

CXX=g++
CXX_FLAGS=-std=c++20 -g

interp: main.cxx parse.cxx syntax.cxx
	${CXX} -o $@ $^ ${CXX_FLAGS}

clear:
	rm ./interp

# end
