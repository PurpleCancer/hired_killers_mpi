MPICC?=mpic++

all: main

main: main.cpp
	${MPICC} main.cpp -o main

