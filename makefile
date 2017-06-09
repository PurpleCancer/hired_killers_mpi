MPICC?=mpic++

all: main

main: main.cpp request
	${MPICC} main.cpp -o main CompanyRequest.o

request: CompanyRequest.cpp CompanyRequest.hpp
	g++ -c CompanyRequest.cpp

clean:
	rm CompanyRequest.o