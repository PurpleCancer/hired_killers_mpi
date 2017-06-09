MPICC?=mpic++

all: main

main: main.cpp request company
	${MPICC} main.cpp -o main CompanyRequest.o Company.o

request: CompanyRequest.cpp CompanyRequest.hpp
	g++ -c CompanyRequest.cpp

company: Company.cpp Company.hpp
	g++ -c Company.cpp

clean:
	rm CompanyRequest.o Company.o